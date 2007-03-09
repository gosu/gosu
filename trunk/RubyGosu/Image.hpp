#ifndef RUBYGOSU_IMAGE_HPP
#define RUBYGOSU_IMAGE_HPP

#include <RubyGosu/RubyGosu.hpp>

namespace RubyGosu
{
    void registerImage(VALUE module);
    VALUE Image_initialize(int argc, VALUE* argv, VALUE self);
    VALUE Image_width(VALUE self);
    VALUE Image_height(VALUE self);
    VALUE Image_draw(int argc, VALUE* argv, VALUE self);
    VALUE Image_draw_rot(int argc, VALUE* argv, VALUE self);
    VALUE Image_from_text(VALUE cls, VALUE windowV, VALUE textV,
        VALUE fontNameV, VALUE fontHeightV, VALUE lineSpacingV,
        VALUE maxWidthV, VALUE alignV);
    VALUE Image_load_tiles(VALUE cls, VALUE windowV, VALUE filenameV,
        VALUE tileWidthV, VALUE tileHeightV, VALUE hardBordersV);
}

#endif
