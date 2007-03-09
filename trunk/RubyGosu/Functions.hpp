#ifndef RUBYGOSU_FUNCTIONS_HPP
#define RUBYGOSU_FUNCTIONS_HPP

#include <RubyGosu/RubyGosu.hpp>

namespace RubyGosu
{
    void registerFunctions(VALUE module);
    VALUE offset_x(VALUE self, VALUE angleV, VALUE distV);
    VALUE offset_y(VALUE self, VALUE angleV, VALUE distV);
    VALUE angle(VALUE self, VALUE x1v, VALUE y1v, VALUE x2v, VALUE y2v);
    VALUE distance(VALUE self, VALUE x1v, VALUE y1v, VALUE x2v, VALUE y2v);
    VALUE milliseconds(VALUE self);
    VALUE default_font_name(VALUE self);
    //VALUE resource_prefix(VALUE self);
    //VALUE shared_resource_prefix(VALUE self);
    VALUE user_settings_prefix(VALUE self);
    VALUE user_docs_prefix(VALUE self);
}

#endif
