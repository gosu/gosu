#include <RubyGosu/Color.hpp>
#include <RubyGosu/DataWrapper.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Color.hpp>
#include <stdexcept>

void RubyGosu::registerColor(VALUE module)
{
    VALUE cColor = rb_define_class_under(module, "Color", rb_cObject);
    defineDefaultNew<Gosu::Color>(cColor);

    rb_define_method(cColor, "initialize", (RubyMethod)Color_initialize, -1);
    rb_define_method(cColor, "alpha", (RubyMethod)Color_alpha, 0);
    rb_define_method(cColor, "alpha=", (RubyMethod)Color_alpha_set, 1);
    rb_define_method(cColor, "red", (RubyMethod)Color_red, 0);
    rb_define_method(cColor, "red=", (RubyMethod)Color_red_set, 1);
    rb_define_method(cColor, "green", (RubyMethod)Color_green, 0);
    rb_define_method(cColor, "green=", (RubyMethod)Color_green_set, 1);
    rb_define_method(cColor, "blue", (RubyMethod)Color_blue, 0);
    rb_define_method(cColor, "blue=", (RubyMethod)Color_blue_set, 1);
}                              

VALUE RubyGosu::Color_initialize(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;

    Gosu::Color c;
    switch (argc)
    {
    case 1:
        c = rb_num2ulong(argv[0]);
        break;
    case 4:
        c.setAlpha(rb_num2ulong(argv[0]));
        c.setRed  (rb_num2ulong(argv[1]));
        c.setGreen(rb_num2ulong(argv[2]));
        c.setBlue (rb_num2ulong(argv[3]));
        break;
    default:
        throw std::invalid_argument("Invalid number of arguments, must be 1 or 4");
    }

    get<Gosu::Color>(self) = c;
    return self;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_alpha(VALUE self)
{
    TRY_CPP;

    return INT2NUM(get<Gosu::Color>(self).alpha());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_alpha_set(VALUE self, VALUE alphaV)
{
    TRY_CPP;

    get<Gosu::Color>(self).setAlpha(NUM2INT(alphaV));
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_red(VALUE self)
{
    TRY_CPP;

    return INT2NUM(get<Gosu::Color>(self).red());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_red_set(VALUE self, VALUE redV)
{
    TRY_CPP;

    get<Gosu::Color>(self).setRed(NUM2INT(redV));
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_green(VALUE self)
{
    TRY_CPP;

    return INT2NUM(get<Gosu::Color>(self).green());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_green_set(VALUE self, VALUE greenV)
{
    TRY_CPP;

    get<Gosu::Color>(self).setGreen(NUM2INT(greenV));
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_blue(VALUE self)
{
    TRY_CPP;

    return INT2NUM(get<Gosu::Color>(self).blue());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Color_blue_set(VALUE self, VALUE blueV)
{
    TRY_CPP;

    get<Gosu::Color>(self).setBlue(NUM2INT(blueV));
    return Qnil;

    TRANSLATE_TO_RB;
}

Gosu::Color RubyGosu::valueToColor(VALUE colorV)
{
    VALUE cls = rb_class_of(colorV);
    if (cls == rb_cFixnum || cls == rb_cBignum)
        return rb_num2ulong(colorV);
    else
        return get<Gosu::Color>(colorV);
}
