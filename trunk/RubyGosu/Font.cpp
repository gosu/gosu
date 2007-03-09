#include <RubyGosu/Font.hpp>
#include <RubyGosu/Color.hpp>
#include <RubyGosu/Window.hpp>
#include <RubyGosu/DataWrapper.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Font.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>
#include <stdexcept>

// TODO: check for nullness
typedef boost::scoped_ptr<Gosu::Font> FontPtr;

// IMPR: Hax0r
Gosu::AlphaMode valueToAlphaMode(VALUE alphaModeV);

void RubyGosu::registerFont(VALUE module)
{
    VALUE cFont = rb_define_class_under(module, "Font", rb_cObject);
    defineDefaultNew<FontPtr>(cFont);

    rb_define_method(cFont, "initialize", (RubyMethod)Font_initialize, 3);
    rb_define_method(cFont, "height", (RubyMethod)Font_height, 0);
    rb_define_method(cFont, "text_width", (RubyMethod)Font_text_width, -1);
    rb_define_method(cFont, "draw", (RubyMethod)Font_draw, -1);
    rb_define_method(cFont, "draw_rel", (RubyMethod)Font_draw_rel, -1);
}

VALUE RubyGosu::Font_initialize(VALUE self, VALUE windowV, VALUE fontNameV,
    VALUE heightV)
{
    TRY_CPP;

    get<FontPtr>(self).reset(new Gosu::Font(getWindow(windowV).graphics(),
        Gosu::widen(valueToString(fontNameV)), rb_num2ulong(heightV)));

    rb_iv_set(self, "@window", windowV);

    return self;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Font_height(VALUE self)
{
    return UINT2NUM(get<FontPtr>(self)->height());
}

VALUE RubyGosu::Font_text_width(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;

    std::wstring text;
    double factorX = 1;

    switch (argc)
    {
    case 2:
        factorX = rb_num2dbl(argv[1]);
    case 1:
        break;
    default:
        throw std::runtime_error("Invalid number of arguments, must be 1 to 2");
    }

    text = Gosu::widen(valueToString(argv[0]));

    return rb_float_new(get<FontPtr>(self)->textWidth(text, factorX));

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Font_draw(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;

    std::wstring text;
    double x, y;
    Gosu::ZPos z = 0;
    double factorX = 1, factorY = 1;
    Gosu::Color c = Gosu::Colors::white;
    Gosu::AlphaMode mode = Gosu::amDefault;

    switch (argc)
    {
    case 8:
        mode = valueToAlphaMode(argv[7]);
    case 7:
        c = valueToColor(argv[6]);
    case 6:
        factorY = rb_num2dbl(argv[5]);
    case 5:
        factorX = rb_num2dbl(argv[4]);
    case 4:
        z = rb_num2ulong(argv[3]);
    case 3:
        break;
    default:
        throw std::runtime_error("Invalid number of arguments, must be 3 to 8");
    }

    y = rb_num2dbl(argv[2]);
    x = rb_num2dbl(argv[1]);
    text = Gosu::widen(valueToString(argv[0]));

    get<FontPtr>(self)->draw(text, x, y, z, factorX, factorY, c, mode);
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Font_draw_rel(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;

    std::wstring text;
    double x, y;
    Gosu::ZPos z;
    double relX, relY;
    double factorX = 1, factorY = 1;
    Gosu::Color c = Gosu::Colors::white;
    Gosu::AlphaMode mode = Gosu::amDefault;

    switch (argc)
    {
    case 10:
        mode = valueToAlphaMode(argv[9]);
    case 9:
        c = valueToColor(argv[8]);
    case 8:
        factorY = rb_num2dbl(argv[7]);
    case 7:
        factorX = rb_num2dbl(argv[6]);
    case 6:
        break;
    default:
        throw std::runtime_error("Invalid number of arguments, must be 6 to 10");
    }

    relY = rb_num2dbl(argv[5]);
    relX = rb_num2dbl(argv[4]);
    z = rb_num2ulong(argv[3]);
    y = rb_num2dbl(argv[2]);
    x = rb_num2dbl(argv[1]);
    text = Gosu::widen(valueToString(argv[0]));

    get<FontPtr>(self)->drawRel(text, x, y, z, relX, relY, factorX, factorY, c, mode);
    return Qnil;

    TRANSLATE_TO_RB;
}
