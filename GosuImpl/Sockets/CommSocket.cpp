#include <Gosu/Sockets.hpp>
#include <Gosu/TR1.hpp>
#include <GosuImpl/Sockets/Sockets.hpp>
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <vector>

struct Gosu::CommSocket::Impl
{
    Socket socket;
    CommMode mode;

    typedef std::vector<char> Buffer;
    Buffer inbox, outbox;

    void appendBuffer(const char* buffer, std::size_t size,
        std::tr1::function<void (const void*, std::size_t)>& event)
    {
        switch (mode)
        {
            case cmRaw:
            {
                // Raw = simple! Yay.
                if (event)
                    event(buffer, size);

                break;
            }

            case cmManaged:
            {
                // Append new data to inbox.
                inbox.insert(inbox.end(), buffer, buffer + size);

                for (;;) // IMPR.
                {
                    const size_t sizeSize = 4;

                    // Not even enough bytes there to determine the size of the
                    // incoming message.
                    if (inbox.size() < sizeSize)
                        break;

                    // Message size is already here, convert it.
                    std::tr1::uint32_t msgSize = *reinterpret_cast<std::tr1::uint32_t*>(&inbox[0]);
                    msgSize = ntohl(msgSize);

                    // Can't really handle zero-size messages. IMPR?!
                    if (msgSize == 0)
                        throw std::logic_error("Cannot handle empty messages");

                    // Has the current message arrived completely?
                    if (inbox.size() < sizeSize + msgSize)
                        break;

                    // Current message is here: Call event...
                    if (event)
                        event(&inbox[sizeSize], msgSize);

                    // ...and delete it from the inbox.
                    inbox.erase(inbox.begin(),
                        inbox.begin() + sizeSize + msgSize);
                }

                break;
            }
        }
    }
};

Gosu::CommSocket::CommSocket(CommMode mode, SocketAddress targetAddress,
    SocketPort targetPort)
: pimpl(new Impl)
{
    pimpl->mode = mode;

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(targetAddress);
    addr.sin_port = htons(targetPort);

    pimpl->socket.setHandle(socketCheck(::socket(AF_INET, SOCK_STREAM, 0)));
    pimpl->socket.setBlocking(true);
    socketCheck(::connect(pimpl->socket.handle(), reinterpret_cast<sockaddr*>(&addr),
        sizeof addr));
    pimpl->socket.setBlocking(false);
}

Gosu::CommSocket::CommSocket(CommMode mode, Socket& socket)
: pimpl(new Impl)
{
    pimpl->mode = mode;

    pimpl->socket.swap(socket);
    pimpl->socket.setBlocking(false);
}

Gosu::CommSocket::~CommSocket()
{
}

Gosu::SocketAddress Gosu::CommSocket::address() const
{
    return pimpl->socket.address();
}

Gosu::SocketPort Gosu::CommSocket::port() const
{
    return pimpl->socket.port();
}

Gosu::SocketAddress Gosu::CommSocket::remoteAddress() const
{
    assert(connected());

    sockaddr_in addr;
    int size = sizeof addr;
    socketCheck(::getpeername(pimpl->socket.handle(),
        reinterpret_cast<sockaddr*>(&addr),
        reinterpret_cast<socklen_t*>(&size)));

    return ntohl(addr.sin_addr.s_addr);
}

Gosu::SocketPort Gosu::CommSocket::remotePort() const
{
    assert(connected());

    sockaddr_in addr;
    int size = sizeof addr;
    socketCheck(::getpeername(pimpl->socket.handle(),
        reinterpret_cast<sockaddr*>(&addr),
        reinterpret_cast<socklen_t*>(&size)));
    return ntohs(addr.sin_port);
}

Gosu::CommMode Gosu::CommSocket::mode() const
{
    return pimpl->mode;
}

bool Gosu::CommSocket::connected() const
{
    return pimpl->socket.handle() != INVALID_SOCKET;
}

void Gosu::CommSocket::disconnect()
{
    pimpl->socket.setHandle(INVALID_SOCKET);
    // IMPR: Mmmmh. A full-blown sockets library should probably try to send
    // the remaining contents of the outbox. This is annoying to implement,
    // though...
    pimpl->outbox.clear();
    if (onDisconnection)
        onDisconnection();
}

bool Gosu::CommSocket::keepAlive() const
{
    int buf;
    int size = sizeof buf;
    socketCheck(::getsockopt(pimpl->socket.handle(), SOL_SOCKET, SO_KEEPALIVE,
        reinterpret_cast<char*>(&buf),
        reinterpret_cast<socklen_t*>(&size)));
    return buf != 0;
}

void Gosu::CommSocket::setKeepAlive(bool value)
{
    int buf = value;
    socketCheck(::setsockopt(pimpl->socket.handle(), SOL_SOCKET, SO_KEEPALIVE,
        reinterpret_cast<char*>(&buf), sizeof buf));
}

void Gosu::CommSocket::update()
{
    sendPendingData();

    if (!connected())
        return;

    for (;;)
    {
        char buffer[1024];
        int received = ::recv(pimpl->socket.handle(), buffer, sizeof buffer, 0);

        if (received > 0 && received <= static_cast<int>(sizeof buffer))
        {
            // Data arrived and fit into the buffer.
            pimpl->appendBuffer(buffer, received, onReceive);
        }
        else if (received == 0)
        {
            // The other side has gracefully closed the connection.
            disconnect();
            return;
        }
        else if (received == SOCKET_ERROR)
        {
            switch (lastSocketError())
            {
                // Arriving data didn't fit into the buffer.
                case GOSU_SOCK_ERR(EMSGSIZE):
                    pimpl->appendBuffer(buffer, sizeof buffer, onReceive);
                    break;

                // There simply was no data.
                case GOSU_SOCK_ERR(EWOULDBLOCK):
                    return;

                // Connection was reset or is invalid.
                case GOSU_SOCK_ERR(ENETDOWN):
                case GOSU_SOCK_ERR(ENOTCONN):
                case GOSU_SOCK_ERR(ENETRESET):
                case GOSU_SOCK_ERR(ECONNABORTED):
                case GOSU_SOCK_ERR(ETIMEDOUT):
                case GOSU_SOCK_ERR(ECONNRESET):
				#ifndef GOSU_IS_WIN
				// UNIX specific, rare error
                case GOSU_SOCK_ERR(EPIPE):
				#endif
                    disconnect();
                    return;

                // Everything else is unexpected.
                default:
                    throwLastSocketError();
            }
        }
        else assert(false);
    }
}

void Gosu::CommSocket::send(const void* buffer, std::size_t size)
{
    if (!connected())
        return;

    // In managed mode, also send the length of the buffer.
    if (mode() == cmManaged)
    {
        std::tr1::uint32_t netSize = htonl(size);
        const char* charBuf = reinterpret_cast<const char*>(&netSize);
        pimpl->outbox.insert(pimpl->outbox.end(), charBuf,
            charBuf + sizeof netSize);
    }

    const char* charBuf = reinterpret_cast<const char*>(buffer);
    pimpl->outbox.insert(pimpl->outbox.end(), charBuf, charBuf + size);
}

void Gosu::CommSocket::sendPendingData()
{
    if (pendingBytes() == 0 || !connected())
        return;

    int sent = ::send(pimpl->socket.handle(), &pimpl->outbox.front(),
        pendingBytes(), 0);

    if (sent >= 0)
    {
        // Remove sent data from the outbox.
        if (sent >= static_cast<int>(pendingBytes()))
            pimpl->outbox.clear();
        else
            pimpl->outbox.erase(pimpl->outbox.begin(),
                pimpl->outbox.begin() + sent);
    }
    else
    {
        switch (lastSocketError())
        {
            // These error codes basically mean "try again later".
            case GOSU_SOCK_ERR(ENOBUFS):
            case GOSU_SOCK_ERR(EWOULDBLOCK):
            case GOSU_SOCK_ERR(EHOSTUNREACH):
                break;

            // And these tell us we're disconnected.
            case GOSU_SOCK_ERR(ENETDOWN):
            case GOSU_SOCK_ERR(ENETRESET):
            case GOSU_SOCK_ERR(ENOTCONN):
            case GOSU_SOCK_ERR(ECONNABORTED):
            case GOSU_SOCK_ERR(ECONNRESET):
            case GOSU_SOCK_ERR(ETIMEDOUT):
			#ifndef GOSU_IS_WIN
			// UNIX-specific, rare error
            case GOSU_SOCK_ERR(EPIPE):
			#endif
                disconnect();
                break;

            // Everything else is unexpected.
            default:
                throwLastSocketError();
        }
    }
}

std::size_t Gosu::CommSocket::pendingBytes() const
{
    return pimpl->outbox.size();
}
