#ifndef RUBYGOSU_FONT_HPP
#define RUBYGOSU_FONT_HPP

#include <RubyGosu/RubyGosu.hpp>

namespace RubyGosu
{
    void registerFont(VALUE module);
    VALUE Font_initialize(VALUE self, VALUE graphicsV, VALUE fontNameV,
        VALUE heightV);
    VALUE Font_height(VALUE self);
    VALUE Font_text_width(int argc, VALUE* argv, VALUE self);
    VALUE Font_draw(int argc, VALUE* argv, VALUE self);
    VALUE Font_draw_rel(int argc, VALUE* argv, VALUE self);
}

#endif
