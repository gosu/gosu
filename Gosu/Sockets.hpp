//! \file Sockets.hpp
//! Interface of the three socket classes, MessageSocket, CommSocket and ListenerSocket.

#ifndef GOSU_SOCKETS_HPP
#define GOSU_SOCKETS_HPP

#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <string>

namespace Gosu
{
    //! Addresses are returned from and given to Gosu functions in host byte order.
    typedef boost::uint32_t SocketAddress;

    //! Ports are returned from and given to Gosu functions in host byte order.
    typedef boost::uint16_t SocketPort;
    
    //! Constant that can be used as a placeholder for an arbitrary port, e.g. when
    //! starting to listen.
    const SocketPort anyPort = 0;
    
    //! Tries to convert a dotted IP4 string into an address suitable for
    //! socket functions. If the string supplied is not such a string, it
    //! tries to look up the host via DNS. If both methods fail, zero is
    //! returned.
    SocketAddress stringToAddress(const std::string& s);
    //! Converts an address into a dotted IP4 string.
    std::string addressToString(SocketAddress address);

    //! Wraps an UDP socket. Message sockets can send data to and receive
    //! data from arbitrary addresses. Also, message sockets send messages
    //! (packets) which are limited in size and can arrive in any order, or
    //! not at all.
    class MessageSocket
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        //! Opens a message socket for listening at the specified port.
        //! Gosu::anyPort may be passed to have the message socket use
        //! a random free port.
        explicit MessageSocket(SocketPort port);
        ~MessageSocket();

        //! Returns the local address of the socket.
        SocketAddress address() const;
        //! Returns the local port of the socket.
        SocketPort port() const;
        //! Returns the maximum size, in bytes, of a packet that can be sent
        //! from this socket.
        std::size_t maxMessageSize() const;

        //! Collects all the packets that were sent to this socket and
        //! calls onReceive for each of them.
        void update();

        //! Sends something to the given port of the computer identified
        //! by the address.
        void send(SocketAddress address, SocketPort port,
            const void* buffer, std::size_t size);
        /*void broadcast(SocketPort port, const void* buffer,
            std::size_t size);*/

        //! If assigned, will be called by update for every packet received.
        boost::function<void (SocketAddress, SocketPort, const void*,
            std::size_t)> onReceive;
    };
    
    //! Defines the way in which data is collected until the onReceive event
    //! is called for CommSockets.
    enum CommMode
    {
        cmRaw,
        //cmLines,
        cmManaged
    };
    
    class Socket;
    
    //! Wraps a TCP socket that is used for one part of bi-directional
    //! communication.
    class CommSocket
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        CommSocket(CommMode mode, SocketAddress targetAddress,
            SocketPort targetPort);
        CommSocket(CommMode mode, Socket& socket);
        ~CommSocket();

        SocketAddress address() const;
        SocketPort port() const;
        SocketAddress remoteAddress() const;
        SocketPort remotePort() const;
        CommMode mode() const;

        bool connected() const;
        void disconnect();
        bool keepAlive() const;
        void setKeepAlive(bool value);

        void update();
        void send(const void* buffer, std::size_t size);
        void sendPendingData();
        std::size_t pendingBytes() const;

        boost::function<void (const void*, std::size_t)> onReceive;
        boost::function<void ()> onDisconnection;
    };
    
    //! Wraps a TCP socket that waits on a specific port and can create
    //! CommSocket instances via its onConnection event.
    class ListenerSocket
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        ListenerSocket(SocketPort port);
        ~ListenerSocket();

        SocketAddress address() const;
        SocketPort port() const;

        void update();

        //! This signal is fired by update() whenever someone connects
        //! to the port which is currently listened on.
        boost::function<void (Socket&)> onConnection;
    };
}

#endif
