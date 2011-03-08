#include <Gosu/Sockets.hpp>
#include <GosuImpl/Sockets/Sockets.hpp>
#include <cassert>
#include <cstring>
#include <vector>

struct Gosu::MessageSocket::Impl
{
    Socket socket;
    std::size_t maxMessageSize;
};

Gosu::MessageSocket::MessageSocket(SocketPort port)
: pimpl(new Impl)
{
    pimpl->socket.setHandle(socketCheck(::socket(AF_INET, SOCK_DGRAM, 0)));
    pimpl->socket.setBlocking(false);
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    socketCheck(::bind(pimpl->socket.handle(),
        reinterpret_cast<sockaddr*>(&addr), sizeof addr));

    unsigned maxMessageSize;
    socklen_t size = sizeof maxMessageSize;
    #ifdef GOSU_IS_WIN
    socketCheck(::getsockopt(pimpl->socket.handle(), SOL_SOCKET, SO_MAX_MSG_SIZE,
        reinterpret_cast<char*>(&maxMessageSize), &size));
    #else
    socketCheck(::getsockopt(pimpl->socket.handle(), SOL_SOCKET, SO_SNDBUF,
        reinterpret_cast<char*>(&maxMessageSize), &size));
    #endif
    pimpl->maxMessageSize = maxMessageSize;
}

Gosu::MessageSocket::~MessageSocket()
{
}

Gosu::SocketAddress Gosu::MessageSocket::address() const
{
    return pimpl->socket.address();
}

Gosu::SocketPort Gosu::MessageSocket::port() const
{
    return pimpl->socket.port();
}

std::size_t Gosu::MessageSocket::maxMessageSize() const
{
    return pimpl->maxMessageSize;
}

void Gosu::MessageSocket::update()
{
    std::vector<char> buffer(maxMessageSize());

    sockaddr_in addr;
    socklen_t size = sizeof addr;

    for (;;)
    {
        int received = ::recvfrom(pimpl->socket.handle(), &buffer.front(),
            buffer.size(), 0, reinterpret_cast<sockaddr*>(&addr),
            &size);

        if (received != SOCKET_ERROR && onReceive)
        {
            onReceive(ntohl(addr.sin_addr.s_addr),
                ntohs(addr.sin_port), &buffer.front(), received);
        }
        else switch (lastSocketError())
        {
            // Ignore some of the errors.
            case GOSU_SOCK_ERR(EWOULDBLOCK):
            case GOSU_SOCK_ERR(ENETDOWN):
            case GOSU_SOCK_ERR(ENETRESET):
            case GOSU_SOCK_ERR(ETIMEDOUT):
            case GOSU_SOCK_ERR(ECONNRESET):
                return;

            // Everything else is unexpected.
            default:
                throwLastSocketError();
        }
    }
}

void Gosu::MessageSocket::send(SocketAddress address, SocketPort port,
    const void* buffer, std::size_t size)
{
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(address);
    addr.sin_port = htons(port);

    int sent = ::sendto(pimpl->socket.handle(),
        reinterpret_cast<const char*>(buffer), size, 0,
        reinterpret_cast<sockaddr*>(&addr), sizeof addr);

    if (sent == static_cast<int>(size))
        return; // Yay, did it!

    assert(sent == SOCKET_ERROR); // Don't expect partial sends.

    switch (lastSocketError())
    {
        // Just ignore a lot of errors... this is UDP, right?
        case GOSU_SOCK_ERR(ENETDOWN):
        case GOSU_SOCK_ERR(ENETRESET):
        case GOSU_SOCK_ERR(ENOBUFS):
        case GOSU_SOCK_ERR(EWOULDBLOCK):
        case GOSU_SOCK_ERR(EHOSTUNREACH):
        case GOSU_SOCK_ERR(ECONNABORTED):
        case GOSU_SOCK_ERR(ECONNRESET):
        case GOSU_SOCK_ERR(ENETUNREACH):
        case GOSU_SOCK_ERR(ETIMEDOUT):
            break;

        // Everything else means more than just another lost packet, though.
        default:
            throwLastSocketError();
    }
}
