#ifndef GOSUIMPL_SOCKETS_HPP
#define GOSUIMPL_SOCKETS_HPP

#include <Gosu/Platform.hpp>
#include <boost/utility.hpp>

#ifdef GOSU_IS_WIN
    #include "winsock2.h"
    #define GOSU_SOCK_ERR(code) WSA##code
    namespace Gosu { typedef SOCKET SocketHandle; }
    typedef int socklen_t;
#else
    #include <sys/errno.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/ioctl.h> 
    #define GOSU_SOCK_ERR(code) code
    // IMPR: Macros ok?
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
    #define ioctlsocket ioctl
    namespace Gosu { typedef int SocketHandle; }
#endif

namespace Gosu
{
    // Owns a socket and manages library initialization.
    class Socket : boost::noncopyable
    {
        SocketHandle handle_;

    public:
        Socket();
        ~Socket();

        SocketHandle handle() const;
        void setHandle(SocketHandle value);
        void setBlocking(bool value);

        SocketAddress address() const;
        SocketPort port() const;

        void swap(Socket& other);
    };

    int lastSocketError();
    
    GOSU_NORETURN void throwLastSocketError();
    
    template<typename T>
    T socketCheck(T retVal)
    {
        if (retVal == SOCKET_ERROR &&
            lastSocketError() != GOSU_SOCK_ERR(EWOULDBLOCK))
        {
            throwLastSocketError();
        }
        
        return retVal;
    }
}

#endif
