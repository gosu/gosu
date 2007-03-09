#include <RubyGosu/Window.hpp>
#include <RubyGosu/Color.hpp>
#include <RubyGosu/DataWrapper.hpp>
#include <RubyGosu/Exceptions.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>

Gosu::AlphaMode valueToAlphaMode(VALUE alphaModeV); // IMPR

VALUE callFun(VALUE args)
{
    VALUE fun = rb_ary_pop(args);
    VALUE obj = rb_ary_pop(args);
    return rb_apply(obj, rb_intern(RubyGosu::valueToString(fun).c_str()), args);
}

VALUE protectedCall(VALUE object, const char* fun, VALUE array)
{
    rb_ary_push(array, object);
    rb_ary_push(array, rb_str_new2(fun));
    int err;
    VALUE retVal = rb_protect(callFun, array, &err);
    if (!err)
        return retVal;
    throw RubyGosu::RubyError();
}

namespace RubyGosu
{
    class WindowImpl : public Gosu::Window
    {
        VALUE* self;

    public:
        WindowImpl(unsigned width, unsigned height, bool fullscreen,
            unsigned updateInterval)
        : Gosu::Window(width, height, fullscreen, updateInterval), self(0)
        {
        }

        void update()
        {
            if (self && BUILTIN_TYPE(*self) != 0)
                protectedCall(*self, "update", rb_ary_new());
        }

        void draw()
        {
            if (self && BUILTIN_TYPE(*self) != 0)
                protectedCall(*self, "draw", rb_ary_new());
        }

        void buttonDown(unsigned id)
        {
            if (self && BUILTIN_TYPE(*self) != 0)
                protectedCall(*self, "button_down", rb_ary_new3(1, INT2NUM(id)));
        }
        
        void buttonUp(unsigned id)
        {
            if (self && BUILTIN_TYPE(*self) != 0)
                protectedCall(*self, "button_up", rb_ary_new3(1, INT2NUM(id)));
        }

        void show(VALUE& rubySelf)
        {
            // Already shown!
            if (self)
                return;

            self = &rubySelf;
            try
            {
                Window::show();
            }
            catch (...)
            {
                self = 0;
                throw;
            }
            self = 0;
        }
    };

    typedef boost::shared_ptr<WindowImpl> WindowPtr;
}

void RubyGosu::registerWindow(VALUE module)
{
    VALUE cWindow = rb_define_class_under(module, "Window", rb_cObject);
    defineDefaultNew<WindowPtr>(cWindow);

    rb_define_method(cWindow, "initialize", (RubyMethod)Window_initialize, 4);
    rb_define_method(cWindow, "caption", (RubyMethod)Window_caption, 0);
    rb_define_method(cWindow, "caption=", (RubyMethod)Window_caption_set, 1);
    rb_define_method(cWindow, "show", (RubyMethod)Window_show, 0);
    rb_define_method(cWindow, "close", (RubyMethod)Window_close, 0);
    rb_define_method(cWindow, "update", (RubyMethod)Window_empty_nullary_fun, 0);
    rb_define_method(cWindow, "draw", (RubyMethod)Window_empty_nullary_fun, 0);
    rb_define_method(cWindow, "button_down", (RubyMethod)Window_empty_unary_fun, 1);
    rb_define_method(cWindow, "button_up", (RubyMethod)Window_empty_unary_fun, 1);
    rb_define_method(cWindow, "draw_line", (RubyMethod)Window_draw_line, -1);
    rb_define_method(cWindow, "draw_triangle", (RubyMethod)Window_draw_triangle, -1);
    rb_define_method(cWindow, "draw_quad", (RubyMethod)Window_draw_quad, -1);
    rb_define_method(cWindow, "mouse_x", (RubyMethod)Window_mouse_x, 0);
    rb_define_method(cWindow, "mouse_y", (RubyMethod)Window_mouse_y, 0);
    rb_define_method(cWindow, "button_down?", (RubyMethod)Window_button_down_qm, 1);
    rb_define_method(cWindow, "button_id_to_char", (RubyMethod)Window_button_id_to_char, 1);
    rb_define_method(cWindow, "char_to_button_id", (RubyMethod)Window_char_to_button_id, 1);
    rb_define_method(cWindow, "width", (RubyMethod)Window_width, 0);
    rb_define_method(cWindow, "height", (RubyMethod)Window_height, 0);
}                              

VALUE RubyGosu::Window_initialize(VALUE self, VALUE widthV, VALUE heightV,
    VALUE fullscreenV, VALUE updateIntervalV)
{
    TRY_CPP

    WindowPtr& ptr = get<WindowPtr>(self);
    ptr.reset(new WindowImpl(NUM2INT(widthV), NUM2INT(heightV),
        RTEST(fullscreenV), NUM2INT(updateIntervalV)));
    return self;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Window_caption(VALUE self)
{
    TRY_CPP;

    std::string caption = Gosu::narrow(getWindow(self).caption());
    return rb_str_new2(caption.c_str());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_caption_set(VALUE self, VALUE captionV)
{
    TRY_CPP;

    getWindow(self).setCaption(Gosu::widen(valueToString(captionV)));
    return Qnil;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_show(VALUE self)
{
    TRY_CPP

    WindowPtr ptr = get<WindowPtr>(self);
    if (!ptr)
        throw std::logic_error("Tried to show an uninitialized window");
    ptr->show(self);
    return Qnil;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Window_close(VALUE self)
{
    TRY_CPP

    WindowPtr ptr = get<WindowPtr>(self);
    if (ptr)
        ptr->close();
    return Qnil;

    TRANSLATE_TO_RB
}

VALUE RubyGosu::Window_empty_nullary_fun(VALUE self)
{
    return Qnil;
}

VALUE RubyGosu::Window_empty_unary_fun(VALUE self, VALUE parm)
{
    return Qnil;
}

VALUE RubyGosu::Window_draw_line(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;
    
    Gosu::ZPos z = 0;
    Gosu::AlphaMode mode = Gosu::amDefault;
    
    switch (argc)
    {
        case 8:
            mode = valueToAlphaMode(argv[7]);
        case 7:
            z = rb_num2ulong(argv[6]);
        case 6:
            break;
        default:
            throw std::runtime_error("Invalid number of arguments, must be 6 to 8");
    }
    
    getWindow(self).graphics().drawLine(
                                        rb_num2dbl(argv[0]), rb_num2dbl(argv[1]), valueToColor(argv[2]),
                                        rb_num2dbl(argv[3]), rb_num2dbl(argv[4]), valueToColor(argv[5]),
                                        z, mode
                                        );
    
    return Qnil;
    
    TRANSLATE_TO_RB;
}


VALUE RubyGosu::Window_draw_triangle(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;
    
    Gosu::ZPos z = 0;
    Gosu::AlphaMode mode = Gosu::amDefault;
    
    switch (argc)
    {
        case 11:
            mode = valueToAlphaMode(argv[10]);
        case 10:
            z = rb_num2ulong(argv[9]);
        case 9:
            break;
        default:
            throw std::runtime_error("Invalid number of arguments, must be 9 to 11");
    }
    
    getWindow(self).graphics().drawTriangle(
                                            rb_num2dbl(argv[0]), rb_num2dbl(argv[1]), valueToColor(argv[2]),
                                            rb_num2dbl(argv[3]), rb_num2dbl(argv[4]), valueToColor(argv[5]),
                                            rb_num2dbl(argv[6]), rb_num2dbl(argv[7]), valueToColor(argv[8]),
                                            z, mode
                                            );
    
    return Qnil;
    
    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_draw_quad(int argc, VALUE* argv, VALUE self)
{
    TRY_CPP;
    
    Gosu::ZPos z = 0;
    Gosu::AlphaMode mode = Gosu::amDefault;
    
    switch (argc)
    {
        case 14:
            mode = valueToAlphaMode(argv[13]);
        case 13:
            z = rb_num2ulong(argv[12]);
        case 12:
            break;
        default:
            throw std::runtime_error("Invalid number of arguments, must be 12 to 14");
    }
    
    getWindow(self).graphics().drawQuad(
                                        rb_num2dbl(argv[0]), rb_num2dbl(argv[1]), valueToColor(argv[2]),
                                        rb_num2dbl(argv[3]), rb_num2dbl(argv[4]), valueToColor(argv[5]),
                                        rb_num2dbl(argv[6]), rb_num2dbl(argv[7]), valueToColor(argv[8]),
                                        rb_num2dbl(argv[9]), rb_num2dbl(argv[10]), valueToColor(argv[11]),
                                        z, mode
                                        );
    
    return Qnil;
    
    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_mouse_x(VALUE self)
{
    TRY_CPP;

    return rb_float_new(getWindow(self).input().mouseX());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_mouse_y(VALUE self)
{
    TRY_CPP;

    return rb_float_new(getWindow(self).input().mouseY());

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_button_down_qm(VALUE self, VALUE buttonV)
{
    TRY_CPP;

    unsigned id = NUM2UINT(buttonV);
    return getWindow(self).input().down(id) ? Qtrue : Qfalse;

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_button_id_to_char(VALUE self, VALUE buttonV)
{
    TRY_CPP;

    unsigned id = NUM2UINT(buttonV);
    char buf[2] = { 0, 0 };
    buf[0] = getWindow(self).input().idToChar(id);
    if (buf[0] == 0)
        return Qnil;
    return rb_str_new2(buf);

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_char_to_button_id(VALUE self, VALUE charV)
{
    TRY_CPP;

    std::string charStr = valueToString(charV);
    if (charStr.length() != 1)
        throw std::invalid_argument("Not a character string: " + charStr);
    unsigned id = getWindow(self).input().charToId(charStr[0]);
    if (id == 0)
        return Qnil;
    return rb_uint2inum(id);

    TRANSLATE_TO_RB;
}

VALUE RubyGosu::Window_width(VALUE self)
{
    return UINT2NUM(getWindow(self).graphics().width());
}

VALUE RubyGosu::Window_height(VALUE self)
{
    return UINT2NUM(getWindow(self).graphics().height());
}

Gosu::Window& RubyGosu::getWindow(VALUE rubyWindow)
{
    return *getWindowRef(rubyWindow);
}

boost::shared_ptr<Gosu::Window> RubyGosu::getWindowRef(VALUE rubyWindow)
{
    WindowPtr ptr = get<WindowPtr>(rubyWindow);
    if (!ptr)
        throw std::logic_error("Tried to access uninitialized window");
    return ptr;
}
