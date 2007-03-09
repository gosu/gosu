#ifndef RUBYGOSU_WINDOW_HPP
#define RUBYGOSU_WINDOW_HPP

#include <RubyGosu/RubyGosu.hpp>
#include <Gosu/Fwd.hpp>
#include <boost/shared_ptr.hpp>

namespace RubyGosu
{
    void registerWindow(VALUE module);
    VALUE Window_initialize(VALUE self, VALUE widthV, VALUE heightV,
        VALUE fullscreenV, VALUE updateIntervalV);
    VALUE Window_caption(VALUE self);
    VALUE Window_caption_set(VALUE self, VALUE captionV);
    VALUE Window_show(VALUE self);
    VALUE Window_close(VALUE self);
    VALUE Window_empty_nullary_fun(VALUE self);
    VALUE Window_empty_unary_fun(VALUE self, VALUE);
    VALUE Window_draw_line(int argc, VALUE* argv, VALUE self);
    VALUE Window_draw_triangle(int argc, VALUE* argv, VALUE self);
    VALUE Window_draw_quad(int argc, VALUE* argv, VALUE self);
    VALUE Window_mouse_x(VALUE self);
    VALUE Window_mouse_y(VALUE self);
    VALUE Window_button_down_qm(VALUE self, VALUE buttonV);
    VALUE Window_button_id_to_char(VALUE self, VALUE buttonV);
    VALUE Window_char_to_button_id(VALUE self, VALUE charV);
    VALUE Window_width(VALUE self);
    VALUE Window_height(VALUE self);

    Gosu::Window& getWindow(VALUE rubyWindow);
    boost::shared_ptr<Gosu::Window> getWindowRef(VALUE rubyWindow);
}

#endif
