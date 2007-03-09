#ifndef RUBYGOSU_COLOR_HPP
#define RUBYGOSU_COLOR_HPP

#include <RubyGosu/RubyGosu.hpp>
#include <Gosu/Color.hpp>

namespace RubyGosu
{
    void registerColor(VALUE module);
    VALUE Color_initialize(int argc, VALUE* argv, VALUE cls);
    VALUE Color_alpha(VALUE self);
    VALUE Color_alpha_set(VALUE self, VALUE redV);
    VALUE Color_red(VALUE self);
    VALUE Color_red_set(VALUE self, VALUE redV);
    VALUE Color_green(VALUE self);
    VALUE Color_green_set(VALUE self, VALUE redV);
    VALUE Color_blue(VALUE self);
    VALUE Color_blue_set(VALUE self, VALUE redV);
    // TODO: Color comparison

    Gosu::Color valueToColor(VALUE colorV);
}

#endif
