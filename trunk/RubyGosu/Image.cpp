#include <RubyGosu/Image.hpp>
#include <RubyGosu/Color.hpp>
#include <RubyGosu/Window.hpp>
#include <RubyGosu/DataWrapper.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>
#include <boost/shared_ptr.hpp>
#include <stdexcept>
#include <vector>

// TODO: check for nullness
typedef boost::shared_ptr<Gosu::Image> ImgPtr;

// Local helper functions:

Gosu::AlphaMode valueToAlphaMode(VALUE alphaModeV)
{
    std::string str = RubyGosu::valueToString(alphaModeV);
    if (str == "default")
        return Gosu::amDefault;
    else if (str == "additive")
        return Gosu::amAdditive;
    else
        throw std::invalid_argument("Invalid alpha mode: " + str);
}

Gosu::TextAlign valueToTextAlign(VALUE alignV)
{
    std::string str = RubyGosu::valueToString(alignV);
    if (str == "left")
        return Gosu::taLeft;
    else if (str == "right")
        return Gosu::taRight;
    else if (str == "center")
        return Gosu::taCenter;
    else if (str == "justify")
        return Gosu::taJustify;
    else
        throw std::invalid_argument("Invalid text align: " + str);
}

void loadBitmap(VALUE filenameV, Gosu::Bitmap& bitmap)
{
    Gosu::Buffer buf;
    Gosu::loadFile(buf, Gosu::widen(StringValueCStr(filenameV)));

    char formatTester[2];
    buf.frontReader().read(formatTester, sizeof formatTester);
    if (formatTester[0] == 'B' && formatTester[1] == 'M')
    {
        Gosu::loadFromBMP(bitmap, buf.frontReader());
        applyColorKey(bitmap, Gosu::Colors::fuchsia);
    }
    else
        Gosu::loadFromPNG(bitmap, buf.frontReader());
}

void RubyGosu::registerImage(VALUE module)
{
    VALUE cImage = rb_define_class_under(module, "Image", rb_cObject);
    defineDefaultNew<ImgPtr>(cImage);

    rb_define_method(cImage, "initialize", (RubyMethod)Image_initialize, -1);
    rb_define_method(cImage, "width", (RubyMethod)Image_width, 0);
    rb_define_method(cImage, "height", (RubyMethod)Image_height, 0);
    rb_define_method(cImage, "draw", (RubyMethod)Image_draw, -1);
    rb_define_method(cImage, "draw_rot", (RubyMethod)Image_draw_rot, -1);
    rb_define_singleton_method(cImage, "from_text", (RubyMethod)Image_from_text, 7);
    rb_define_singleton_method(cImage, "load_tiles", (RubyMethod)Image_load_tiles, 5);
}

VALUE RubyGosu::Image_initialize(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;

    if (argc != 3 && argc != 7)
        throw std::invalid_argument("wrong number of arguments; must be 3 or 7");

    Gosu::Bitmap bmp;

    loadBitmap(argv[1], bmp);

    unsigned srcX, srcY, srcWidth, srcHeight;
    if (argc == 3)
    {
        srcX = srcY = 0;
        srcWidth = bmp.width();
        srcHeight = bmp.height();
    }
    else
    {
        srcX = rb_num2ulong(argv[3]);
        srcY = rb_num2ulong(argv[4]);
        srcWidth = rb_num2ulong(argv[5]);
        srcHeight = rb_num2ulong(argv[6]);
    }

    get<ImgPtr>(self).reset(new Gosu::Image(getWindow(argv[0]).graphics(), bmp,
        srcX, srcY, srcWidth, srcHeight, RTEST(argv[2])));

    rb_iv_set(self, "@window", argv[0]);

    return self;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Image_width(VALUE self)
{
    TRY_CPP

    return INT2NUM(get<ImgPtr>(self)->width());

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Image_height(VALUE self)
{
    TRY_CPP

    return INT2NUM(get<ImgPtr>(self)->height());

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Image_draw(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;

    double x, y;
    Gosu::ZPos z = 0;
    double factorX = 1, factorY = 1;
    Gosu::Color c = Gosu::Colors::white;
    Gosu::AlphaMode mode = Gosu::amDefault;

    switch (argc)
    {
    case 7:
        mode = valueToAlphaMode(argv[6]);
    case 6:
        c = valueToColor(argv[5]);
    case 5:
        factorY = rb_num2dbl(argv[4]);
    case 4:
        factorX = rb_num2dbl(argv[3]);
    case 3:
        z = rb_num2ulong(argv[2]);
    case 2:
        break;
    default:
        throw std::runtime_error("Invalid number of arguments, must be 2 to 7");
    }
    y = rb_num2dbl(argv[1]);
    x = rb_num2dbl(argv[0]);

    get<ImgPtr>(self)->draw(x, y, z, factorX, factorY, c, mode);
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Image_draw_rot(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;

    double x, y;
    Gosu::ZPos z = 0;
    double angle = 0, centerX = 0.5, centerY = 0.5;
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
        centerY = rb_num2dbl(argv[5]);
    case 5:
        centerX = rb_num2dbl(argv[4]);
    case 4:
        angle = rb_num2dbl(argv[3]);
    case 3:
        z = rb_num2ulong(argv[2]);
    case 2:
        break;
    default:
        throw std::runtime_error("Invalid number of arguments, must be 2 to 10");
    }
    y = rb_num2dbl(argv[1]);
    x = rb_num2dbl(argv[0]);

    get<ImgPtr>(self)->drawRot(x, y, z, angle, centerX, centerY, factorX,
        factorY, c, mode);
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Image_from_text(VALUE cls, VALUE windowV, VALUE textV,
    VALUE fontNameV, VALUE fontHeightV, VALUE lineSpacingV, VALUE maxWidthV,
    VALUE alignV)
{
    TRY_CPP;

    Gosu::Bitmap bmp = Gosu::createText(Gosu::widen(StringValueCStr(textV)),
        Gosu::widen(StringValueCStr(fontNameV)), rb_num2ulong(fontHeightV),
        rb_num2ulong(lineSpacingV), rb_num2ulong(maxWidthV),
        valueToTextAlign(alignV));

    ImgPtr textBlock(new Gosu::Image(getWindow(windowV).graphics(), bmp));
    ImgPtr* imgPtrPtr = new ImgPtr(textBlock);
    return Data_Wrap_Struct(cls, 0, freeObj<ImgPtr>, imgPtrPtr);

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Image_load_tiles(VALUE cls, VALUE windowV, VALUE filenameV,
    VALUE tileWidthV, VALUE tileHeightV, VALUE hardBordersV)
{
    TRY_CPP;

    Gosu::Bitmap bmp;
    loadBitmap(filenameV, bmp);
    std::vector<ImgPtr> vec;
    Gosu::imagesFromTiledBitmap(getWindow(windowV).graphics(), bmp,
        rb_num2long(tileWidthV), rb_num2long(tileHeightV), RTEST(hardBordersV), vec);

    VALUE ary = rb_ary_new();
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        ImgPtr* imgPtrPtr = new ImgPtr(vec[i]);
        VALUE curImg = Data_Wrap_Struct(cls, 0, freeObj<ImgPtr>, imgPtrPtr);
        rb_ary_push(ary, curImg);
    }
    return ary;

    TRANSLATE_TO_RB;
}
