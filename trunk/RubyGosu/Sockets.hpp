#ifndef RUBYGOSU_SOCKETS_HPP
#define RUBYGOSU_SOCKETS_HPP

#include <RubyGosu/RubyGosu.hpp>

namespace RubyGosu
{
    void registerSockets(VALUE module);

    VALUE string_to_address(VALUE mod, VALUE stringV);
    VALUE address_to_string(VALUE mod, VALUE addressV);

    VALUE MessageSocket_initialize(VALUE self, VALUE portV);
    VALUE MessageSocket_address(VALUE self);
    VALUE MessageSocket_port(VALUE self);
    VALUE MessageSocket_max_message_size(VALUE self);
    VALUE MessageSocket_update(VALUE self);
    VALUE MessageSocket_send(VALUE self, VALUE addressV, VALUE portV, VALUE stringV);
    VALUE MessageSocket_on_receive(VALUE self, VALUE addressV, VALUE portV,
        VALUE stringV);

    VALUE CommSocket_initialize(VALUE self, VALUE targetAddressV,
        VALUE targetPortV);
    VALUE CommSocket_address(VALUE self);
    VALUE CommSocket_port(VALUE self);
    VALUE CommSocket_remote_address(VALUE self);
    VALUE CommSocket_remote_port(VALUE self);
    VALUE CommSocket_connected(VALUE self);
    VALUE CommSocket_disconnect(VALUE self);
    VALUE CommSocket_keep_alive(VALUE self);
    VALUE CommSocket_set_keep_alive(VALUE self, VALUE keepAliveV);
    VALUE CommSocket_update(VALUE self);
    VALUE CommSocket_send(VALUE self, VALUE stringV);
    VALUE CommSocket_send_pending_data(VALUE self);
    VALUE CommSocket_pending_bytes(VALUE self);
    VALUE CommSocket_on_receive(VALUE self, VALUE stringV);
    VALUE CommSocket_on_disconnection(VALUE self);

    VALUE ListenerSocket_initialize(VALUE self, VALUE portV);
    VALUE ListenerSocket_address(VALUE self);
    VALUE ListenerSocket_port(VALUE self);
    VALUE ListenerSocket_update(VALUE self);
    VALUE ListenerSocket_on_connection(VALUE self, VALUE commSocketV);
}

#endif
