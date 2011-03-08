#include <Gosu/Sockets.hpp>
#include <GosuImpl/Sockets/Sockets.hpp>
#ifdef GOSU_IS_WIN
#include <Gosu/WinUtility.hpp>
#endif
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <stdexcept>

namespace Gosu
{
    namespace
    {
        void cleanup()
        {
#ifdef GOSU_IS_WIN
            ::WSACleanup();
#endif
        }

        void needsSockLib()
        {
#ifdef GOSU_IS_WIN
            static bool initialized = false;
            if (!initialized)
            {
                WSADATA data;
                if (::WSAStartup(0x0202, &data) != 0)
                    throw std::runtime_error("Could not initialize "
                        "Windows sockets");

                initialized = true;
                std::atexit(cleanup);
            }
#endif
        }
    }
}

int Gosu::lastSocketError()
{
#ifdef GOSU_IS_WIN
    needsSockLib();
    return ::WSAGetLastError();
#else
    return errno;
#endif
}

GOSU_NORETURN void Gosu::throwLastSocketError()
{
#ifdef GOSU_IS_WIN
    Win::throwLastError();
#else
    throw std::runtime_error(strerror(errno));
#endif
}

Gosu::SocketAddress Gosu::stringToAddress(const std::string& s)
{
    needsSockLib();

    SocketAddress address;

    // Try to interpret name as an IP number string.
    address = ntohl(::inet_addr(s.c_str()));
    if (address != INADDR_NONE)
        return address;

    // This didn't work: Resolve host name via DNS.
    hostent* host = ::gethostbyname(s.c_str());
    if (!host)
        return 0;
    in_addr addr = *reinterpret_cast<in_addr*>(host->h_addr_list[0]);
    return ntohl(addr.s_addr);
}

std::string Gosu::addressToString(SocketAddress address)
{
    needsSockLib();

    in_addr addr;
    addr.s_addr = htonl(address);
    return ::inet_ntoa(addr);
}

Gosu::Socket::Socket()
: handle_(INVALID_SOCKET)
{
    needsSockLib();
}

Gosu::Socket::~Socket()
{
    setHandle(INVALID_SOCKET);
}

Gosu::SocketHandle Gosu::Socket::handle() const
{
    return handle_;
}

void Gosu::Socket::setHandle(SocketHandle value)
{
    if (handle() != INVALID_SOCKET)
    {
        ::closesocket(handle());
    }

    handle_ = value;
}

void Gosu::Socket::setBlocking(bool value)
{
    unsigned long enable = !value;
    socketCheck(::ioctlsocket(handle(), FIONBIO, &enable));
}

Gosu::SocketAddress Gosu::Socket::address() const
{
    sockaddr_in addr;
    int size = sizeof addr;
    socketCheck(::getsockname(handle(),
        reinterpret_cast<sockaddr*>(&addr),
        reinterpret_cast<socklen_t*>(&size)));

    return ntohl(addr.sin_addr.s_addr);
}

Gosu::SocketPort Gosu::Socket::port() const
{
    sockaddr_in addr;
    int size = sizeof addr;
    socketCheck(::getsockname(handle(),
        reinterpret_cast<sockaddr*>(&addr),
        reinterpret_cast<socklen_t*>(&size)));

    return ntohs(addr.sin_port);
}

void Gosu::Socket::swap(Socket& other)
{
    std::swap(handle_, other.handle_);
}
