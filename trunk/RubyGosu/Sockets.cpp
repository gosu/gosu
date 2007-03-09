#include <RubyGosu/Sockets.hpp>
#include <RubyGosu/DataWrapper.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Sockets.hpp>
#include <boost/bind.hpp>
#include <stdexcept>

namespace
{
    typedef boost::scoped_ptr<Gosu::MessageSocket> MsgSockPtr;
    typedef boost::scoped_ptr<Gosu::CommSocket> CommSockPtr;
    typedef boost::scoped_ptr<Gosu::ListenerSocket> LnrSockPtr;
    
    void callOnMsgRecv(VALUE socket, Gosu::SocketAddress address,
        Gosu::SocketPort port, const void* buffer, std::size_t size)
    {
        VALUE stringV = rb_str_new(reinterpret_cast<const char*>(buffer), size);
        rb_funcall(socket, rb_intern("on_receive"), 3, UINT2NUM(address),
            UINT2NUM(port), stringV);
    }

    void callOnCommRecv(VALUE socket, const void* buffer, std::size_t size)
    {
        VALUE stringV = rb_str_new(reinterpret_cast<const char*>(buffer), size);
        rb_funcall(socket, rb_intern("on_receive"), 1, stringV);
    }

    void callOnDisconnection(VALUE socket)
    {
        rb_funcall(socket, rb_intern("on_disconnection"), 0);
    }

    VALUE cCommSocket;

    void callOnConnection(VALUE socket, Gosu::Socket& newSocket)
    {
        CommSockPtr holder(new Gosu::CommSocket(Gosu::cmManaged, newSocket));
        CommSockPtr* commSockPtrPtr = new CommSockPtr;
        commSockPtrPtr->swap(holder);
        VALUE newSocketV = Data_Wrap_Struct(cCommSocket, 0, RubyGosu::freeObj<CommSockPtr>, commSockPtrPtr);
        rb_funcall(socket, rb_intern("on_connection"), 1, newSocketV);
    }
}

void RubyGosu::registerSockets(VALUE module)
{
    rb_define_module_function(module, "string_to_address", (RubyMethod)string_to_address, 1);
    rb_define_module_function(module, "address_to_string", (RubyMethod)address_to_string, 1);

    VALUE cMessageSocket = rb_define_class_under(module, "MessageSocket", rb_cObject);
    defineDefaultNew<MsgSockPtr>(cMessageSocket);

    rb_define_method(cMessageSocket, "initialize", (RubyMethod)MessageSocket_initialize, 1);
    rb_define_method(cMessageSocket, "address", (RubyMethod)MessageSocket_address, 0);
    rb_define_method(cMessageSocket, "port", (RubyMethod)MessageSocket_port, 0);
    rb_define_method(cMessageSocket, "max_message_size", (RubyMethod)MessageSocket_max_message_size, 0);
    rb_define_method(cMessageSocket, "update", (RubyMethod)MessageSocket_update, 0);
    rb_define_method(cMessageSocket, "send", (RubyMethod)MessageSocket_send, 3);
    rb_define_method(cMessageSocket, "on_receive", (RubyMethod)MessageSocket_on_receive, 3);

    /*VALUE */cCommSocket = rb_define_class_under(module, "CommSocket", rb_cObject);
    defineDefaultNew<CommSockPtr>(cCommSocket);

    rb_define_method(cCommSocket, "initialize", (RubyMethod)CommSocket_initialize, 2);
    rb_define_method(cCommSocket, "address", (RubyMethod)CommSocket_address, 0);
    rb_define_method(cCommSocket, "port", (RubyMethod)CommSocket_port, 0);
    rb_define_method(cCommSocket, "remote_address", (RubyMethod)CommSocket_remote_address, 0);
    rb_define_method(cCommSocket, "remote_port", (RubyMethod)CommSocket_remote_port, 0);
    rb_define_method(cCommSocket, "connected?", (RubyMethod)CommSocket_connected, 0);
    rb_define_method(cCommSocket, "disconnect", (RubyMethod)CommSocket_disconnect, 0);
    rb_define_method(cCommSocket, "keep_alive", (RubyMethod)CommSocket_keep_alive, 0);
    rb_define_method(cCommSocket, "keep_alive=", (RubyMethod)CommSocket_set_keep_alive, 1);
    rb_define_method(cCommSocket, "update", (RubyMethod)CommSocket_update, 0);
    rb_define_method(cCommSocket, "send", (RubyMethod)CommSocket_send, 1);
    rb_define_method(cCommSocket, "send_pending_data", (RubyMethod)CommSocket_send_pending_data, 0);
    rb_define_method(cCommSocket, "pending_bytes", (RubyMethod)CommSocket_pending_bytes, 0);
    rb_define_method(cCommSocket, "on_receive", (RubyMethod)CommSocket_on_receive, 1);
    rb_define_method(cCommSocket, "on_disconnection", (RubyMethod)CommSocket_on_disconnection, 0);

    VALUE cListenerSocket = rb_define_class_under(module, "ListenerSocket", rb_cObject);
    defineDefaultNew<LnrSockPtr>(cListenerSocket);

    rb_define_method(cListenerSocket, "initialize", (RubyMethod)ListenerSocket_initialize, 1);
    rb_define_method(cListenerSocket, "address", (RubyMethod)ListenerSocket_address, 0);
    rb_define_method(cListenerSocket, "port", (RubyMethod)ListenerSocket_port, 0);
    rb_define_method(cListenerSocket, "update", (RubyMethod)ListenerSocket_update, 0);
    rb_define_method(cListenerSocket, "on_connection", (RubyMethod)ListenerSocket_on_connection, 1);
}

VALUE RubyGosu::string_to_address(VALUE mod, VALUE stringV)
{
    TRY_CPP;

    return UINT2NUM(Gosu::stringToAddress(valueToString(stringV)));

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::address_to_string(VALUE mod, VALUE addressV)
{
    TRY_CPP;

    return rb_str_new2(Gosu::addressToString(rb_num2ulong(addressV)).c_str());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::MessageSocket_initialize(VALUE self, VALUE portV)
{
    TRY_CPP;

    get<MsgSockPtr>(self).reset(new Gosu::MessageSocket(rb_num2ulong(portV)));
    return self;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::MessageSocket_address(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<MsgSockPtr>(self)->address());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::MessageSocket_port(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<MsgSockPtr>(self)->port());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::MessageSocket_max_message_size(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<MsgSockPtr>(self)->maxMessageSize());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::MessageSocket_update(VALUE self)
{
    TRY_CPP;

    get<MsgSockPtr>(self)->onReceive = boost::bind(callOnMsgRecv, self,
        _1, _2, _3, _4);
    get<MsgSockPtr>(self)->update();
    get<MsgSockPtr>(self)->onReceive.clear();
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::MessageSocket_send(VALUE self, VALUE addressV, VALUE portV,
    VALUE stringV)
{
    TRY_CPP;

    get<MsgSockPtr>(self)->send(rb_num2ulong(addressV), rb_num2ulong(portV),
        RSTRING(stringV)->ptr, RSTRING(stringV)->len);
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::MessageSocket_on_receive(VALUE self, VALUE addressV, VALUE portV,
    VALUE stringV)
{
    return Qnil;
}

VALUE RubyGosu::CommSocket_initialize(VALUE self, VALUE targetAddressV,
    VALUE targetPortV)
{
    TRY_CPP;

    get<CommSockPtr>(self).reset(new Gosu::CommSocket(Gosu::cmManaged,
        rb_num2ulong(targetAddressV), rb_num2ulong(targetPortV)));
    return self;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_address(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<CommSockPtr>(self)->address());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_port(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<CommSockPtr>(self)->port());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_remote_address(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<CommSockPtr>(self)->remoteAddress());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_remote_port(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<CommSockPtr>(self)->remotePort());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_connected(VALUE self)
{
    TRY_CPP;

    return get<CommSockPtr>(self)->connected() ? Qtrue : Qfalse;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_disconnect(VALUE self)
{
    TRY_CPP;

    get<CommSockPtr>(self)->disconnect();
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_keep_alive(VALUE self)
{
    TRY_CPP;

    return get<CommSockPtr>(self)->keepAlive() ? Qtrue : Qfalse;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_set_keep_alive(VALUE self, VALUE keepAliveV)
{
    TRY_CPP;

    get<CommSockPtr>(self)->setKeepAlive(RTEST(keepAliveV));
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_update(VALUE self)
{
    TRY_CPP;

    get<CommSockPtr>(self)->onReceive = boost::bind(callOnCommRecv, self,
        _1, _2);
    get<CommSockPtr>(self)->onDisconnection = boost::bind(callOnDisconnection, self);
    get<CommSockPtr>(self)->update();
    get<CommSockPtr>(self)->onReceive.clear();
    get<CommSockPtr>(self)->onDisconnection.clear();
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_send(VALUE self, VALUE stringV)
{
    TRY_CPP;

    get<CommSockPtr>(self)->send(RSTRING(stringV)->ptr, RSTRING(stringV)->len);
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_send_pending_data(VALUE self)
{
    TRY_CPP;

    get<CommSockPtr>(self)->onDisconnection = boost::bind(callOnDisconnection, self);
    get<CommSockPtr>(self)->sendPendingData();
    get<CommSockPtr>(self)->onDisconnection.clear();
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_pending_bytes(VALUE self)
{
    TRY_CPP;

    return rb_uint2inum(get<CommSockPtr>(self)->pendingBytes());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::CommSocket_on_receive(VALUE self, VALUE stringV)
{
    return Qnil;
}

VALUE RubyGosu::CommSocket_on_disconnection(VALUE self)
{
    return Qnil;
}

VALUE RubyGosu::ListenerSocket_initialize(VALUE self, VALUE portV)
{
    TRY_CPP;

    get<LnrSockPtr>(self).reset(new Gosu::ListenerSocket(rb_num2ulong(portV)));
    return self;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::ListenerSocket_address(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<LnrSockPtr>(self)->address());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::ListenerSocket_port(VALUE self)
{
    TRY_CPP;

    return UINT2NUM(get<LnrSockPtr>(self)->port());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::ListenerSocket_update(VALUE self)
{
    TRY_CPP;

    get<LnrSockPtr>(self)->onConnection = boost::bind(callOnConnection, self,
        _1);
    get<LnrSockPtr>(self)->update();
    get<LnrSockPtr>(self)->onConnection.clear();
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::ListenerSocket_on_connection(VALUE self, VALUE socketV)
{
    return Qnil;
}
