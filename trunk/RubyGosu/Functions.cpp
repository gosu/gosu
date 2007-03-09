#include <RubyGosu/Functions.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Directories.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/Utility.hpp>

void RubyGosu::registerFunctions(VALUE module)
{
    rb_define_module_function(module, "offset_x", (RubyMethod)offset_x, 2);
    rb_define_module_function(module, "offset_y", (RubyMethod)offset_y, 2);
    rb_define_module_function(module, "angle", (RubyMethod)angle, 4);
    rb_define_module_function(module, "distance", (RubyMethod)distance, 4);
    rb_define_module_function(module, "milliseconds", (RubyMethod)milliseconds, 0);
    rb_define_module_function(module, "default_font_name", (RubyMethod)default_font_name, 0);
    //rb_define_module_function(module, "resource_prefix", (RubyMethod)resource_prefix, 0);
    //rb_define_module_function(module, "shared_resource_prefix", (RubyMethod)shared_resource_prefix, 0);
    rb_define_module_function(module, "user_settings_prefix", (RubyMethod)user_settings_prefix, 0);
    rb_define_module_function(module, "user_docs_prefix", (RubyMethod)user_docs_prefix, 0);
}

VALUE RubyGosu::offset_x(VALUE self, VALUE angleV, VALUE distV)
{
    TRY_CPP;

    return rb_float_new(Gosu::offsetX(rb_num2dbl(angleV), rb_num2dbl(distV)));

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::offset_y(VALUE self, VALUE angleV, VALUE distV)
{
    TRY_CPP;

    return rb_float_new(Gosu::offsetY(rb_num2dbl(angleV), rb_num2dbl(distV)));

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::angle(VALUE self, VALUE x1v, VALUE y1v, VALUE x2v, VALUE y2v)
{
    TRY_CPP;

    return rb_float_new(Gosu::angle(rb_num2dbl(x1v), rb_num2dbl(y1v),
        rb_num2dbl(x2v), rb_num2dbl(y2v)));

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::distance(VALUE self, VALUE x1v, VALUE y1v, VALUE x2v, VALUE y2v)
{
    TRY_CPP;

    return rb_float_new(Gosu::distance(rb_num2dbl(x1v), rb_num2dbl(y1v),
        rb_num2dbl(x2v), rb_num2dbl(y2v)));

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::milliseconds(VALUE self)
{
    TRY_CPP;
    
    return rb_uint2inum(Gosu::milliseconds());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::default_font_name(VALUE self)
{
    TRY_CPP;

    return rb_str_new2(Gosu::narrow(Gosu::defaultFontName()).c_str());

    TRANSLATE_TO_RB;
}

/*VALUE RubyGosu::resource_prefix(VALUE self)
{
    TRY_CPP;
    
    return rb_str_new2(Gosu::narrow(Gosu::resourcePrefix()).c_str());
    
    TRANSLATE_TO_RB;
}

VALUE RubyGosu::shared_resource_prefix(VALUE self)
{
    TRY_CPP;
    
    return rb_str_new2(Gosu::narrow(Gosu::sharedResourcePrefix()).c_str());
    
    TRANSLATE_TO_RB;
}*/

VALUE RubyGosu::user_settings_prefix(VALUE self)
{
    TRY_CPP;
    
    return rb_str_new2(Gosu::narrow(Gosu::userSettingsPrefix()).c_str());
    
    TRANSLATE_TO_RB;
}

VALUE RubyGosu::user_docs_prefix(VALUE self)
{
    TRY_CPP;
    
    return rb_str_new2(Gosu::narrow(Gosu::userDocsPrefix()).c_str());
    
    TRANSLATE_TO_RB;
}
