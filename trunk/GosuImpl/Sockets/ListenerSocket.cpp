#include <Gosu/Sockets.hpp>
#include <GosuImpl/Sockets/Sockets.hpp>
#include <cassert>
#include <cstring>

struct Gosu::ListenerSocket::Impl
{
    Socket socket;
};

Gosu::ListenerSocket::ListenerSocket(SocketPort port)
: pimpl(new Impl)
{
    pimpl->socket.setHandle(socketCheck(::socket(AF_INET, SOCK_STREAM, 0)));
    pimpl->socket.setBlocking(false);

    int enable = 1;
    socketCheck(::setsockopt(pimpl->socket.handle(), SOL_SOCKET, SO_REUSEADDR,
        reinterpret_cast<char*>(&enable), sizeof enable));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof addr);
    addr.sin_family = AF_INET;
//    addr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    socketCheck(::bind(pimpl->socket.handle(),
        reinterpret_cast<sockaddr*>(&addr), sizeof addr));
    socketCheck(::listen(pimpl->socket.handle(), 10));
}

Gosu::ListenerSocket::~ListenerSocket()
{
}

Gosu::SocketAddress Gosu::ListenerSocket::address() const
{
    return pimpl->socket.address();
}

Gosu::SocketPort Gosu::ListenerSocket::port() const
{
    return pimpl->socket.port();
}

void Gosu::ListenerSocket::update()
{
    while (onConnection)
    {
        SocketHandle newHandle =
            socketCheck(::accept(pimpl->socket.handle(), 0, 0));

        if (newHandle == INVALID_SOCKET)
            break;

        Socket newSocket;
        newSocket.setHandle(newHandle);
        onConnection(newSocket);
    }
}
