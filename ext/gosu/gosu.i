%trackobjects;
%include exception.i

// Don't complain about ignored operators.
#pragma SWIG nowarn=378

// The docs make it seem like this is a generally good idea.
%feature("compactdefaultargs");

// Overriding virtuals is necessary in Gosu::Window.
%feature("director") Gosu::Window;

// Overriding virtuals is necessary in Gosu::TextInput.
%feature("director") Gosu::TextInput;

// Resolve typedefs that SWIG doesn't recognize.
%apply unsigned char { std::uint8_t };
%apply unsigned long { std::uint32_t };

// Custom typemap to enforce UTF-8 encoding on all created strings.
%typemap(out) std::string {
    $result = rb_str_new2($1.c_str());
    ENFORCE_UTF8($result);
}

%typemap(in) Gosu::Button {
    $1 = NIL_P($input) ? Gosu::NO_BUTTON : Gosu::Button(NUM2LONG($input));
}
%typemap(out) Gosu::Button {
    $result = $1 == Gosu::NO_BUTTON ? Qnil : LONG2NUM($1);
}
%typemap(directorin) Gosu::Button {
    $input = $1 == Gosu::NO_BUTTON ? Qnil : LONG2NUM($1);
}

// Typemaps for enums that should be given in as symbols.
%typemap(in) Gosu::BlendMode {
    const char* cstr = Gosu::cstr_from_symbol($input);
    
    if (!strcmp(cstr, "default")) {
        $1 = Gosu::BM_DEFAULT;
    }
    else if (!strcmp(cstr, "add") || !strcmp(cstr, "additive")) {
        $1 = Gosu::BM_ADD;
    }
    else if (!strcmp(cstr, "multiply")) {
        $1 = Gosu::BM_MULTIPLY;
    }
    else {
        SWIG_exception_fail(SWIG_ValueError, "invalid blend mode (expected one of :default, :add, "
                            ":multiply)");
    }
}
%typemap(in) Gosu::Alignment {
    const char* cstr = Gosu::cstr_from_symbol($input);

    if (!strcmp(cstr, "left")) {
        $1 = Gosu::AL_LEFT;
    }
    else if (!strcmp(cstr, "center")) {
        $1 = Gosu::AL_CENTER;
    }
    else if (!strcmp(cstr, "right")) {
        $1 = Gosu::AL_RIGHT;
    }
    else if (!strcmp(cstr, "justify")) {
        $1 = Gosu::AL_JUSTIFY;
    }
    else {
        SWIG_exception_fail(SWIG_ValueError, "invalid text alignment (expected one of :left, "
                            ":center, :right, :justify)");
    }
}

// Allow integral constants to be passed in place of Color values.
%typemap(in) Gosu::Color {    
    if (TYPE($input) == T_FIXNUM || TYPE($input) == T_BIGNUM) {
        $1 = Gosu::Color(NUM2ULONG($input));
    }
    else {
        void* ptr;
        int res = SWIG_ConvertPtr($input, &ptr, SWIGTYPE_p_Gosu__Color, 0);
        if (!SWIG_IsOK(res)) {
            SWIG_exception_fail(SWIG_ValueError, "invalid value");
        }
        else if (ptr == nullptr) {
            SWIG_exception_fail(SWIG_ValueError, "invalid null reference of type Gosu::Color");
        }
        else {
            $1 = *reinterpret_cast<Gosu::Color*>(ptr);
        }
    }
}

// Make color channels less strict.
%typemap(in) Gosu::Color::Channel {
    $1 = std::clamp<int>(NUM2ULONG($input), 0, 255);
}
// To allow for overloading with Channel values.
%typemap(typecheck) Gosu::Color::Channel {
    $1 = !!rb_respond_to($input, rb_intern("to_i"));
}

// Header inclusion (order irrelevant)
%module(directors="1") gosu
%{
    
// Escape from ruby.h macro hell on Windows.
#undef accept
#undef write
#undef close
#undef read
#undef bind
#undef send
#undef sleep
#undef Sleep
#undef int8_t
#undef uint8_t
#undef int16_t
#undef uint16_t
#undef int32_t
#undef uint32_t
#undef int64_t
#undef uint64_t

// Escape from windows.h macro hell.
#define NOMINMAX
#ifdef min
#undef min
#undef max
#endif

#include <Gosu/Gosu.hpp>

namespace Gosu
{
    void enable_undocumented_retrofication()
    {
        extern bool undocumented_retrofication;
        undocumented_retrofication = true;
    }
    
    void al_shutdown();
}

#include <cstring>
#include <ctime>
#include <sstream>

#define ENFORCE_UTF8(val) rb_funcall(val, rb_intern("force_encoding"), 1, rb_str_new2("UTF-8"))

namespace Gosu
{
    void call_ruby_block(VALUE block)
    {
        rb_funcall(block, rb_intern("call"), 0);
    }

    void load_bitmap(Gosu::Bitmap& bitmap, VALUE val)
    {
        // Try to treat as filename first.
        if (rb_respond_to(val, rb_intern("to_str"))) {
            VALUE to_str = rb_funcall(val, rb_intern("to_str"), 0);
            const char* filename = StringValuePtr(to_str);
            bitmap = Gosu::load_image_file(filename);
            return;
        }

        // Otherwise, try to call .to_blob on it (works with RMagick, TexPlay etc).
        VALUE conversion = rb_str_new2("to_blob { self.format = 'RGBA'; self.depth = 8 }");
        VALUE blob = rb_obj_instance_eval(1, &conversion, val);
        Check_Type(blob, T_STRING);

        int width  = NUM2ULONG(rb_funcall(val, rb_intern("columns"), 0));
        int height = NUM2ULONG(rb_funcall(val, rb_intern("rows"), 0));
        
        std::size_t size = width * height * 4;
        bitmap.resize(width, height, Gosu::Color::NONE);
        if (RSTRING_LEN(blob) == size) {
            // 32 bit per pixel, assume R8G8B8A8
            std::memcpy(bitmap.data(), reinterpret_cast<const unsigned*>(RSTRING_PTR(blob)), size);
        }
        else if (RSTRING_LEN(blob) == size * sizeof(float)) {
            // 128 bit per channel, assume float/float/float/float
            const float* in = reinterpret_cast<const float*>(RSTRING_PTR(blob));
            Gosu::Color::Channel* out = reinterpret_cast<Gosu::Color::Channel*>(bitmap.data());
            for (int i = size; i > 0; --i) {
                *(out++) = static_cast<Color::Channel>(*(in++) * 255);
            }
        }
        else {
            throw std::logic_error("Blob length mismatch");
        }
    }
    
    const char* cstr_from_symbol(VALUE symbol)
    {
        Check_Type(symbol, T_SYMBOL);
        return rb_id2name(SYM2ID(symbol));
    }
}
    
// Global input functions

namespace Gosu
{
    Gosu::Button char_to_button_id(std::string ch)
    {
        return Gosu::Input::char_to_id(ch);
    }
    
    std::string button_id_to_char(Gosu::Button btn)
    {
        return Gosu::Input::id_to_char(btn);
    }
    
    bool is_button_down(Gosu::Button btn)
    {
        return Gosu::Input::down(btn);
    }

    VALUE button_name(Gosu::Button btn)
    {
      std::string result = Gosu::Input::button_name(btn);
      return result.empty() ? Qnil : rb_str_new2(result.c_str());
    }

    VALUE gamepad_name(int index)
    {
      std::string result = Gosu::Input::gamepad_name(index);
      return result.empty() ? Qnil : rb_str_new2(result.c_str());
    }

    double axis(Gosu::Button btn)
    {
        return Gosu::Input::axis(btn);
    }
}

// Global graphics functions

namespace Gosu
{
    void draw_line(double x1, double y1, Gosu::Color c1,
                   double x2, double y2, Gosu::Color c2,
                   Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT)
    {
        Gosu::Graphics::draw_line(x1, y1, c1, x2, y2, c2, z, mode);
    }
    
    void draw_triangle(double x1, double y1, Gosu::Color c1,
                       double x2, double y2, Gosu::Color c2,
                       double x3, double y3, Gosu::Color c3,
                       Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT)
    {
        Gosu::Graphics::draw_triangle(x1, y1, c1, x2, y2, c2, x3, y3, c3, z, mode);
    }
    
    void draw_quad(double x1, double y1, Gosu::Color c1,
                   double x2, double y2, Gosu::Color c2,
                   double x3, double y3, Gosu::Color c3,
                   double x4, double y4, Gosu::Color c4,
                   Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT)
    {
        Gosu::Graphics::draw_quad(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode);
    }
    
    void draw_rect(double x, double y, double width, double height, Gosu::Color c,
                   Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT)
    {
        Gosu::Graphics::draw_rect(x, y, width, height, c, z, mode);
    }
    
    void flush()
    {
        return Gosu::Graphics::flush();
    }
    
    void unsafe_gl()
    {
        Gosu::Graphics::gl([] { rb_yield(Qnil); });
    }
    
    void unsafe_gl(Gosu::ZPos z)
    {
        VALUE block = rb_block_proc();
        Gosu::Graphics::gl(z, [block] {
            Gosu::call_ruby_block(block);
        });
    }
    
    void clip_to(double x, double y, double width, double height)
    {
        Gosu::Graphics::clip_to(x, y, width, height, [] { rb_yield(Qnil); });
    }
    
    Gosu::Image* record(int width, int height) {
        return new Gosu::Image(Gosu::Graphics::record(width, height, [] { rb_yield(Qnil); }));
    }
    
    Gosu::Image* render(int width, int height, VALUE options = 0) {
        unsigned image_flags = 0;
        
        if (options) {
            Check_Type(options, T_HASH);
            
            VALUE keys = rb_funcall(options, rb_intern("keys"), 0, NULL);
            int keys_size = NUM2INT(rb_funcall(keys, rb_intern("size"), 0, NULL));
            
            for (int i = 0; i < keys_size; ++i) {
                VALUE key = rb_ary_entry(keys, i);
                const char* key_string = Gosu::cstr_from_symbol(key);
                
                VALUE value = rb_hash_aref(options, key);
                if (!strcmp(key_string, "retro")) {
                    if (RTEST(value)) image_flags |= Gosu::IF_RETRO;
                }
                else {
                    static bool issued_warning = false;
                    if (!issued_warning) {
                        issued_warning = true;
                        rb_warn("Unknown keyword argument: :%s", key_string);
                    }
                }
            }
        }
        
        return new Gosu::Image(Gosu::Graphics::render(width, height, [] { rb_yield(Qnil); }, image_flags));
    }
    
    // This method cannot be called "transform" because then it would be an ambiguous overload of
    // Gosu::Transform Gosu::transform(...).
    // So it has to be renamed via %rename below... :( - same for the other transformations.
    
    void transform_for_ruby(double m0, double m1, double m2, double m3, double m4, double m5,
        double m6, double m7, double m8, double m9, double m10, double m11, double m12, double m13,
        double m14, double m15)
    {
        Gosu::Transform transform = {
            m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11, m12, m13, m14, m15
        };
        Gosu::Graphics::transform(transform,
                                  [] { rb_yield(Qnil); });
    }
    
    void rotate_for_ruby(double angle, double around_x = 0, double around_y = 0)
    {
        Gosu::Graphics::transform(Gosu::rotate(angle, around_x, around_y),
                                  [] { rb_yield(Qnil); });
    }
    
    void scale_for_ruby(double factor)
    {
        Gosu::Graphics::transform(Gosu::scale(factor),
                                  [] { rb_yield(Qnil); });
    }
    
    void scale_for_ruby(double scale_x, double scale_y)
    {
        Gosu::Graphics::transform(Gosu::scale(scale_x, scale_y),
                                  [] { rb_yield(Qnil); });
    }
    
    void scale_for_ruby(double scale_x, double scale_y, double around_x, double around_y)
    {
        Gosu::Graphics::transform(Gosu::scale(scale_x, scale_y, around_x, around_y),
                                  [] { rb_yield(Qnil); });
    }
    
    void translate_for_ruby(double x, double y)
    {
        Gosu::Graphics::transform(Gosu::translate(x, y),
                                  [] { rb_yield(Qnil); });
    }
}
%}

// Exception wrapping
%exception {
    try {
        $action
    }
    catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// Version constant
%rename("MAJOR_VERSION") GOSU_MAJOR_VERSION;
%rename("MINOR_VERSION") GOSU_MINOR_VERSION;
%rename("POINT_VERSION") GOSU_POINT_VERSION;
%ignore Gosu::VERSION;
%ignore Gosu::LICENSES;
%constant std::string VERSION = Gosu::VERSION;
%constant std::string LICENSES = Gosu::LICENSES;
%include "../../include/Gosu/Version.hpp"

// Miscellaneous functions (timing, math)
%include "../../include/Gosu/Timing.hpp"
%ignore Gosu::distance_sqr;
%ignore Gosu::wrap;
%ignore Gosu::radians_to_gosu;
%ignore Gosu::gosu_to_radians;
%include "../../include/Gosu/Math.hpp"
%ignore Gosu::text_width;
%ignore Gosu::draw_text;
%ignore Gosu::layout_text;
%ignore Gosu::layout_markup;
%include "../../include/Gosu/Text.hpp"


// Graphics:

// BlendMode, FontFlags, Alignment, ...
%ignore Gosu::BlendMode;
%ignore Gosu::FontFlags;
%ignore Gosu::Alignment;
%ignore Gosu::ImageFlags;
%ignore Gosu::concat;
%ignore Gosu::translate;
%ignore Gosu::rotate;
%ignore Gosu::scale;
%include "../../include/Gosu/GraphicsBase.hpp"

// For screen_width/screen_height
%ignore Gosu::Graphics;
%ignore Gosu::BorderFlags;
%ignore Gosu::MAX_TEXTURE_SIZE;
%include "../../include/Gosu/Graphics.hpp"

%constant unsigned MAX_TEXTURE_SIZE = Gosu::MAX_TEXTURE_SIZE;

%typemap(out) std::vector<std::string> {
    $result = rb_ary_new2($1.size());
    for (std::size_t i = 0; i < $1.size(); i++) {
        rb_ary_store($result, i, rb_str_new2((*&$1)[i].c_str()));
    }
}

namespace Gosu
{
    std::vector<std::string> user_languages();
    void enable_undocumented_retrofication();
}

// Color
%rename("hue=") set_hue;
%rename("saturation=") set_saturation;
%rename("value=") set_value;
%include "std_string.i"

%ignore Gosu::Color::Channel;
%ignore Gosu::Color::NONE;
%ignore Gosu::Color::BLACK;
%ignore Gosu::Color::GRAY;
%ignore Gosu::Color::WHITE;
%ignore Gosu::Color::AQUA;
%ignore Gosu::Color::RED;
%ignore Gosu::Color::GREEN;
%ignore Gosu::Color::BLUE;
%ignore Gosu::Color::YELLOW;
%ignore Gosu::Color::FUCHSIA;
%ignore Gosu::Color::CYAN;

%include "../../include/Gosu/Color.hpp"

%extend Gosu::Color {
    static Gosu::Color rgb(Gosu::Color::Channel r, Gosu::Color::Channel g,
        Gosu::Color::Channel b)
    {
        return Gosu::Color{r, g, b};
    }
    
    static Gosu::Color rgba(Gosu::Color::Channel r, Gosu::Color::Channel g,
        Gosu::Color::Channel b, Gosu::Color::Channel a)
    {
        return Gosu::Color{r, g, b}.with_alpha(a);
    }
    
    static Gosu::Color rgba(std::uint32_t rgba)
    {
        const std::uint32_t argb = (rgba >> 8) & 0xffffff | ((rgba & 0xff) << 24);
        return Gosu::Color{argb};
    }
    
    static Gosu::Color argb(Gosu::Color::Channel a, Gosu::Color::Channel r,
        Gosu::Color::Channel g, Gosu::Color::Channel b)
    {
        return Gosu::Color{r, g, b}.with_alpha(a);
    }
    
    static Gosu::Color argb(std::uint32_t argb)
    {
        return Gosu::Color{argb};
    }

    static Color from_ahsv(Channel alpha, double h, double s, double v)
    {
        return Gosu::Color::from_hsv(h, s, v).with_alpha(alpha);
    }
    
    std::uint32_t to_i() const
    {
      return $self->argb();
    }
    
    Gosu::Color dup() const
    {
        return *$self;
    }

    std::string inspect() const
    {
        char buffer[32];
        // snprintf is either a member of std:: or a #define for ruby_snprintf.
        using namespace std;
        snprintf(buffer, sizeof buffer, "#<Gosu::Color:ARGB=0x%02x_%02x%02x%02x>",
            $self->alpha, $self->red, $self->green, $self->blue);
        return buffer;
    }
    
    bool operator==(VALUE other) const
    {
        if (TYPE(other) == T_FIXNUM || TYPE(other) == T_BIGNUM) {
            return *$self == Gosu::Color(NUM2ULONG(other));
        }
        void* ptr;
        int res = SWIG_ConvertPtr(other, &ptr, SWIGTYPE_p_Gosu__Color, 0);
        return SWIG_IsOK(res) && ptr && *$self == *reinterpret_cast<Gosu::Color*>(ptr);
    }
}

// Font
%ignore Gosu::Font::Font(int height, const std::string& font_name, unsigned flags);
%ignore Gosu::Font::text_width(const std::string& text) const;
%ignore Gosu::Font::markup_width(const std::string& markup) const;
%ignore Gosu::Font::set_image(std::string codepoint, unsigned font_flags, const Gosu::Image& image);

%include "../../include/Gosu/Font.hpp"
%extend Gosu::Font {
    Font(Gosu::Window& window, const std::string& font_name, int height)
    {
        return new Gosu::Font(height, font_name);
    }
    
    Font(int height, VALUE options = 0)
    {
        std::string font_name = Gosu::default_font_name();
        unsigned font_flags = 0;
        
        if (options) {
            Check_Type(options, T_HASH);
            
            VALUE keys = rb_funcall(options, rb_intern("keys"), 0, NULL);
            int keys_size = NUM2INT(rb_funcall(keys, rb_intern("size"), 0, NULL));
            
            for (int i = 0; i < keys_size; ++i) {
                VALUE key = rb_ary_entry(keys, i);
                const char* key_string = Gosu::cstr_from_symbol(key);
                
                VALUE value = rb_hash_aref(options, key);
                if (!strcmp(key_string, "name")) {
                    VALUE rb_string = rb_obj_as_string(value);
                    font_name = StringValueCStr(rb_string);
                }
                else if (!strcmp(key_string, "bold")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_BOLD;
                }
                else if (!strcmp(key_string, "italic")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_ITALIC;
                }
                else if (!strcmp(key_string, "underline")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_UNDERLINE;
                }
                else {
                    static bool issued_warning = false;
                    if (!issued_warning) {
                        issued_warning = true;
                        rb_warn("Unknown keyword argument: :%s", key_string);
                    }
                }
            }
        }
        
        return new Gosu::Font(height, font_name, font_flags);
    }
    
    double text_width(const std::string& markup, double scale_x = 1.0)
    {
      static bool issued_warning = false;
      if (scale_x != 1.0 && !issued_warning) {
          issued_warning = true;
          rb_warn("The second argument to Gosu::Font#text_width is deprecated, multiply the result instead");
      }
      return $self->text_width(markup) * scale_x;
    }

    double markup_width(const std::string& markup, double scale_x = 1.0)
    {
      static bool issued_warning = false;
      if (scale_x != 1.0 && !issued_warning) {
          issued_warning = true;
          rb_warn("The second argument to Gosu::Font#markup_width is deprecated, multiply the result instead");
      }
      return $self->markup_width(markup) * scale_x;
    }
}

%ignore Gosu::ImageData;
%include "../../include/Gosu/ImageData.hpp"

// Image

// Typemap to return an array of images (for load_tiles)
%typemap(out) std::vector<Gosu::Image> {
    $result = rb_ary_new2($1.size());
    for (unsigned i = 0; i < $1.size(); i++) {
        VALUE image = SWIG_NewPointerObj(SWIG_as_voidptr(new Gosu::Image((*&$1)[i])),
                                         SWIGTYPE_p_Gosu__Image, SWIG_POINTER_OWN);
        rb_ary_store($result, i, image);
    }
}

%ignore Gosu::Image::Image();
%ignore Gosu::Image::Image(const std::string& filename, unsigned flags);
%ignore Gosu::Image::Image(const std::string& filename, int src_x, int src_y,
                           int src_width, int src_height, unsigned flags);
%ignore Gosu::Image::Image(const Bitmap& source, unsigned flags);
%ignore Gosu::Image::Image(const Bitmap& source, int src_x, int src_y,
                           int src_width, int src_height, unsigned flags);
%ignore Gosu::Image::Image(std::unique_ptr<ImageData>&& data);
%ignore Gosu::load_tiles;
%include "../../include/Gosu/Image.hpp"
%extend Gosu::Image {
    Image(VALUE source, VALUE options = 0)
    {
        Gosu::Bitmap bmp;
        Gosu::load_bitmap(bmp, source);
        
        int src_x = 0, src_y = 0;
        int src_width = bmp.width(), src_height = bmp.height();
        unsigned flags = 0;
        
        if (options) {
            Check_Type(options, T_HASH);
            
            VALUE keys = rb_funcall(options, rb_intern("keys"), 0, NULL);
            int keys_size = NUM2INT(rb_funcall(keys, rb_intern("size"), 0, NULL));
            
            for (int i = 0; i < keys_size; ++i) {
                VALUE key = rb_ary_entry(keys, i);
                const char* key_string = Gosu::cstr_from_symbol(key);
                
                VALUE value = rb_hash_aref(options, key);
                if (!strcmp(key_string, "tileable")) {
                    if (RTEST(value)) flags |= Gosu::IF_TILEABLE;
                }
                else if (!strcmp(key_string, "retro")) {
                    if (RTEST(value)) flags |= Gosu::IF_RETRO;
                }
                else if (!strcmp(key_string, "rect")) {
                    Check_Type(value, T_ARRAY);
                    
                    int rect_size = NUM2INT(rb_funcall(value, rb_intern("size"), 0, NULL));
                    if (rect_size != 4) {
                        rb_raise(rb_eArgError, "Argument passed to :rect must be a four-element "
                                               "Array [x, y, width, height]");
                    }
                    
                    src_x      = NUM2INT(rb_ary_entry(value, 0));
                    src_y      = NUM2INT(rb_ary_entry(value, 1));
                    src_width  = NUM2INT(rb_ary_entry(value, 2));
                    src_height = NUM2INT(rb_ary_entry(value, 3));
                }
                else {
                    static bool issued_warning = false;
                    if (!issued_warning) {
                        issued_warning = true;
                        rb_warn("Unknown keyword argument: :%s", key_string);
                    }
                }
            }
        }
        
        return new Gosu::Image(bmp, src_x, src_y, src_width, src_height, flags);
    }
    
    void draw_as_quad(double x1, double y1, Color c1,
                      double x2, double y2, Color c2,
                      double x3, double y3, Color c3,
                      double x4, double y4, Color c4,
                      ZPos z, BlendMode mode = Gosu::BM_DEFAULT)
    {
        $self->data().draw(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode);
    }
    
    %newobject gl_tex_info;
    Gosu::GLTexInfo* gl_tex_info() const
    {
        const Gosu::GLTexInfo* info = $self->data().gl_tex_info();
        return info ? new Gosu::GLTexInfo(*info) : nullptr;
    }
    
    %newobject subimage;
    Gosu::Image* subimage(int x, int y, int w, int h)
    {
        std::unique_ptr<Gosu::ImageData> image_data = $self->data().subimage(x, y, w, h);
        return image_data.get() ? new Gosu::Image(std::move(image_data)) : nullptr;
    }
    
    %newobject from_text;
    static Gosu::Image* from_text(const std::string& text, double font_height, VALUE options = 0)
    {
        std::string font = Gosu::default_font_name();
        int width = -1;
        double spacing = 0;
        Gosu::Alignment align = Gosu::AL_LEFT;
        unsigned image_flags = 0;
        unsigned font_flags = 0;

        if (options) {
            Check_Type(options, T_HASH);

            VALUE keys = rb_funcall(options, rb_intern("keys"), 0, NULL);
            int keys_size = NUM2INT(rb_funcall(keys, rb_intern("size"), 0, NULL));

            for (int i = 0; i < keys_size; ++i) {
                VALUE key = rb_ary_entry(keys, i);
                const char* key_string = Gosu::cstr_from_symbol(key);

                VALUE value = rb_hash_aref(options, key);
                if (!strcmp(key_string, "font")) {
                    font = StringValuePtr(value);
                }
                else if (!strcmp(key_string, "bold")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_BOLD;
                }
                else if (!strcmp(key_string, "italic")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_ITALIC;
                }
                else if (!strcmp(key_string, "underline")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_UNDERLINE;
                }
                else if (!strcmp(key_string, "align")) {
                    const char* cstr = Gosu::cstr_from_symbol(value);

                    if (!strcmp(cstr, "left")) {
                        align = Gosu::AL_LEFT;
                    }
                    else if (!strcmp(cstr, "center")) {
                        align = Gosu::AL_CENTER;
                    }
                    else if (!strcmp(cstr, "right")) {
                        align = Gosu::AL_RIGHT;
                    }
                    else if (!strcmp(cstr, "justify")) {
                        align = Gosu::AL_JUSTIFY;
                    }
                    else {
                        rb_raise(rb_eArgError, "Argument passed to :align must be a valid text "
                                               "alignment (:left, :center, :right, :justify)");
                    }
                }
                else if (!strcmp(key_string, "width")) {
                    width = NUM2INT(value);
                }
                else if (!strcmp(key_string, "spacing")) {
                    spacing = NUM2DBL(value);
                }
                else if (!strcmp(key_string, "retro")) {
                    if (RTEST(value)) image_flags |= Gosu::IF_RETRO;
                }
                else {
                    static bool issued_warning = false;
                    if (!issued_warning) {
                        issued_warning = true;
                        rb_warn("Unknown keyword argument: :%s", key_string);
                    }
                }
            }
        }

        Gosu::Bitmap bitmap = Gosu::layout_text(text, font, font_height, spacing, width,
                                                align, font_flags);
        return new Gosu::Image(bitmap, image_flags);
    }

    // TODO: Reduce duplication between from_text and from_markup.
    %newobject from_markup;
    static Gosu::Image* from_markup(const std::string& markup, double font_height, VALUE options = 0)
    {
        std::string font = Gosu::default_font_name();
        int width = -1;
        double spacing = 0;
        Gosu::Alignment align = Gosu::AL_LEFT;
        unsigned image_flags = 0;
        unsigned font_flags = 0;
        
        if (options) {
            Check_Type(options, T_HASH);
            
            VALUE keys = rb_funcall(options, rb_intern("keys"), 0, NULL);
            int keys_size = NUM2INT(rb_funcall(keys, rb_intern("size"), 0, NULL));
            
            for (int i = 0; i < keys_size; ++i) {
                VALUE key = rb_ary_entry(keys, i);
                const char* key_string = Gosu::cstr_from_symbol(key);
                
                VALUE value = rb_hash_aref(options, key);
                if (!strcmp(key_string, "font")) {
                    font = StringValuePtr(value);
                }
                else if (!strcmp(key_string, "bold")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_BOLD;
                }
                else if (!strcmp(key_string, "italic")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_ITALIC;
                }
                else if (!strcmp(key_string, "underline")) {
                    if (RTEST(value)) font_flags |= Gosu::FF_UNDERLINE;
                }
                else if (!strcmp(key_string, "align")) {
                    const char* cstr = Gosu::cstr_from_symbol(value);

                    if (!strcmp(cstr, "left")) {
                        align = Gosu::AL_LEFT;
                    }
                    else if (!strcmp(cstr, "center")) {
                        align = Gosu::AL_CENTER;
                    }
                    else if (!strcmp(cstr, "right")) {
                        align = Gosu::AL_RIGHT;
                    }
                    else if (!strcmp(cstr, "justify")) {
                        align = Gosu::AL_JUSTIFY;
                    }
                    else {
                        rb_raise(rb_eArgError, "Argument passed to :align must be a valid text "
                                 "alignment (:left, :center, :right, :justify)");
                    }
                }
                else if (!strcmp(key_string, "width")) {
                    width = NUM2INT(value);
                }
                else if (!strcmp(key_string, "spacing")) {
                    spacing = NUM2DBL(value);
                }
                else if (!strcmp(key_string, "retro")) {
                    if (RTEST(value)) image_flags |= Gosu::IF_RETRO;
                }
                else {
                    static bool issued_warning = false;
                    if (!issued_warning) {
                        issued_warning = true;
                        rb_warn("Unknown keyword argument: :%s", key_string);
                    }
                }
            }
        }
        
        Gosu::Bitmap bitmap = Gosu::layout_markup(markup, font, font_height, spacing, width,
                                                  align, font_flags);
        return new Gosu::Image(bitmap, image_flags);
    }
    
    static std::vector<Gosu::Image> load_tiles(VALUE source, int tile_width, int tile_height,
        VALUE options = 0)
    {
        Gosu::Bitmap bmp;
        Gosu::load_bitmap(bmp, source);
        
        unsigned flags = 0;
        
        if (options) {
            Check_Type(options, T_HASH);
            
            VALUE keys = rb_funcall(options, rb_intern("keys"), 0, NULL);
            int keys_size = NUM2INT(rb_funcall(keys, rb_intern("size"), 0, NULL));
            
            for (int i = 0; i < keys_size; ++i) {
                VALUE key = rb_ary_entry(keys, i);
                const char* key_string = Gosu::cstr_from_symbol(key);
                
                VALUE value = rb_hash_aref(options, key);
                if (!strcmp(key_string, "tileable")) {
                    if (RTEST(value)) flags |= Gosu::IF_TILEABLE;
                }
                else if (!strcmp(key_string, "retro")) {
                    if (RTEST(value)) flags |= Gosu::IF_RETRO;
                }
                else {
                    static bool issued_warning = false;
                    if (!issued_warning) {
                        issued_warning = true;
                        rb_warn("Unknown keyword argument: :%s", key_string);
                    }
                }
            }
        }
        return Gosu::load_tiles(bmp, tile_width, tile_height, flags);
    }
    
    static std::vector<Gosu::Image> load_tiles(Gosu::Window& window, VALUE source,
        int tile_width, int tile_height, bool tileable)
    {
        Gosu::Bitmap bmp;
        Gosu::load_bitmap(bmp, source);
        
        return Gosu::load_tiles(bmp, tile_width, tile_height,
            tileable ? Gosu::IF_TILEABLE : Gosu::IF_SMOOTH);
    }
    
    VALUE to_blob() const
    {
        Gosu::Bitmap bmp = $self->data().to_bitmap();
        auto size = bmp.width() * bmp.height() * sizeof(Gosu::Color);
        return rb_str_new(reinterpret_cast<const char*>(bmp.data()), size);
    }
    
    int columns() const
    {
        return $self->width();
    }
    
    int rows() const
    {
        return $self->height();
    }
    
    void save(const std::string& filename) const
    {
        Gosu::save_image_file($self->data().to_bitmap(), filename);
    }
    
    void insert(VALUE source, int x, int y)
    {
        Gosu::Bitmap bmp;
        Gosu::load_bitmap(bmp, source);
        $self->data().insert(bmp, x, y);
    }
    
    // This is a very low-tech helper that maps the image's alpha channel to ASCII art.
    // Inspired by the sample code in stb_truetype.h <3
    std::string inspect(int max_width = 80) const
    {
        try {
            Gosu::Bitmap bmp = $self->data().to_bitmap();
            // This is the scaled image width inside the ASCII art border, so make sure
            // there will be room for a leading and trailing '#' character.
            int w = std::clamp<int>(max_width - 2, 0, bmp.width());
            // For images with width == 0, the output will have one line per pixel.
            // Otherwise, scale proportionally.
            int h = (w ? bmp.height() * w / bmp.width() : bmp.height());
            
            // This is the length of one row in the string output, including the border
            // and a trailing newline.
            int stride = w + 3;
            std::string str(stride * (h + 2), '#');
            str[stride - 1] = '\n'; // first newline
            str.back()      = '\n'; // last newline

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    int scaled_x = x * bmp.width()  / w;
                    int scaled_y = y * bmp.height() / h;
                    int alpha3bit = bmp.get_pixel(scaled_x, scaled_y).alpha / 32;
                    str[(y + 1) * stride + (x + 1)] = " .:ioVM@"[alpha3bit];
                }
                str[(y + 1) * stride + (w + 2)] = '\n'; // newline after row of pixels
            }
            return str;
        }
        catch (...) {
            return "<Gosu::Image without bitmap representation>";
        }
    }
}

// Inspection:

%include "../../include/Gosu/Inspection.hpp"


// Audio:

%ignore Gosu::Channel::Channel();
%ignore Gosu::Channel::Channel(int, int);
%ignore Gosu::Channel::current_channel();
%ignore Gosu::Sample::Sample();
%ignore Gosu::Sample::Sample(Reader reader);
%ignore Gosu::Song::Song(Reader reader);
%rename("playing?") playing;
%rename("paused?") paused;
%rename("volume=") set_volume;
%rename("pan=") set_pan;
%rename("speed=") set_speed;
%include "../../include/Gosu/Audio.hpp"

// Input and Window:

// Button ID constants
%ignore Gosu::KB_RANGE_BEGIN;
%ignore Gosu::KB_RANGE_END;
%ignore Gosu::MS_RANGE_BEGIN;
%ignore Gosu::MS_RANGE_END;
%ignore Gosu::GP_RANGE_BEGIN;
%ignore Gosu::GP_RANGE_END;
%ignore Gosu::GP_AXES_RANGE_BEGIN;
%ignore Gosu::GP_AXES_RANGE_END;
%ignore Gosu::KB_NUM;
%ignore Gosu::MS_NUM;
%ignore Gosu::GP_NUM;
%ignore Gosu::GP_NUM_PER_GAMEPAD;
%ignore Gosu::NUM_GAMEPADS;
%ignore Gosu::NUM_AXES;
%ignore Gosu::NO_BUTTON;
%include "../../include/Gosu/Buttons.hpp"
%init %{
    // Call srand() so that Gosu::random() is actually random in Ruby scripts
    std::srand(static_cast<unsigned>(std::time(0)));
    std::rand(); // and flush the first value
%}

// TextInput
%ignore Gosu::TextInput::feed_sdl_event(void*);
%ignore Gosu::TextInput::caret_pos() const;
%ignore Gosu::TextInput::set_caret_pos(unsigned);
%ignore Gosu::TextInput::selection_start() const;
%ignore Gosu::TextInput::set_selection_start(unsigned);
%rename("text=") set_text;
%rename("caret_pos=") set_caret_pos;
%rename("selection_start=") set_selection_start;
%include "../../include/Gosu/TextInput.hpp"

// Make sure that the indices in caret_pos/selection_start are valid Ruby string indices.
// (In C++, these methods return/accept UTF-8 byte indices.)
%extend Gosu::TextInput {
    VALUE caret_pos()
    {
        std::string prefix = $self->text().substr(0, $self->caret_pos());
        VALUE rb_prefix = rb_str_new2(prefix.c_str());
        ENFORCE_UTF8(rb_prefix);
        return rb_funcall(rb_prefix, rb_intern("length"), 0);
    }
    
    void set_caret_pos(VALUE caret_pos)
    {
        VALUE rb_text = rb_str_new2($self->text().c_str());
        VALUE rb_prefix = rb_funcall(rb_text, rb_intern("slice"), 2, LONG2NUM(0), caret_pos);
        std::string prefix = StringValueCStr(rb_prefix);
        $self->set_caret_pos(std::min(prefix.length(), $self->text().length()));
    }
    
    VALUE selection_start()
    {
        std::string prefix = $self->text().substr(0, $self->selection_start());
        VALUE rb_prefix = rb_str_new2(prefix.c_str());
        ENFORCE_UTF8(rb_prefix);
        return rb_funcall(rb_prefix, rb_intern("length"), 0);
    }
    
    void set_selection_start(VALUE selection_start)
    {
        VALUE rb_text = rb_str_new2($self->text().c_str());
        VALUE rb_prefix = rb_funcall(rb_text, rb_intern("slice"), 2, LONG2NUM(0), selection_start);
        std::string prefix = StringValueCStr(rb_prefix);
        $self->set_selection_start(std::min(prefix.length(), $self->text().length()));
    }
};

// Window
%ignore Gosu::WindowFlags;
%ignore Gosu::Window::resize;
%rename("width=") set_width;
%rename("height=") set_height;
%rename("fullscreen=") set_fullscreen;
%rename("resizable=") set_resizable;
%rename("borderless=") set_borderless;
%rename("update_interval=") set_update_interval;
%rename("caption=") set_caption;
%rename("text_input=") set_text_input;
%rename("mouse_x=") set_mouse_x;
%rename("mouse_y=") set_mouse_y;
%rename("needs_cursor?") needs_cursor;
%rename("needs_redraw?") needs_redraw;
%rename("close!") force_close;
%rename("fullscreen?") fullscreen;
%rename("resizable?") resizable;
%rename("borderless?") borderless;
%markfunc Gosu::Window "mark_window";
%include "../../include/Gosu/Window.hpp"

%header %{
    // Also mark the TextInput instance alive when the window is being marked.
    static void mark_window(void* window)
    {
        Gosu::TextInput* cpp_instance = static_cast<Gosu::Window*>(window)->input().text_input();
        if (VALUE ruby_instance = SWIG_RubyInstanceFor(cpp_instance)) {
            rb_gc_mark(ruby_instance);
        }
    }
%}

%extend Gosu::Window {
    void set_width(unsigned width)
    {
        $self->resize(width, $self->height(), $self->fullscreen());
    }
    
    void set_height(unsigned height)
    {
        $self->resize($self->width(), height, $self->fullscreen());
    }
    
    void set_fullscreen(bool fullscreen)
    {
        $self->resize($self->width(), $self->height(), fullscreen);
    }
    
    TextInput* text_input() const
    {
        return $self->input().text_input();
    }
    
    void set_text_input(TextInput* ti)
    {
        $self->input().set_text_input(ti);
    }
    
    double mouse_x() const
    {
        return $self->input().mouse_x();
    }
    
    double mouse_y() const
    {
        return $self->input().mouse_y();
    }
    
    void set_mouse_position(double x, double y)
    {
        $self->input().set_mouse_position(x, y);
    }
    
    void set_mouse_x(double x)
    {
        $self->input().set_mouse_position(x, $self->input().mouse_y());
    }
    
    void set_mouse_y(double y)
    {
        $self->input().set_mouse_position($self->input().mouse_x(), y);
    }
    
    void force_close()
    {
        $self->Gosu::Window::close();
    }
};

// Global input functions
namespace Gosu
{
    Gosu::Button char_to_button_id(std::string ch);
    std::string button_id_to_char(Gosu::Button btn);
    %rename("button_down?") is_button_down;
    bool is_button_down(Gosu::Button btn);
    VALUE button_name(Gosu::Button btn);
    VALUE gamepad_name(int index);
    double axis(Gosu::Button btn);
}

// Global graphics functions
namespace Gosu
{
    void draw_line(double x1, double y1, Gosu::Color c1,
                   double x2, double y2, Gosu::Color c2,
                   Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT);

    void draw_triangle(double x1, double y1, Gosu::Color c1,
                       double x2, double y2, Gosu::Color c2,
                       double x3, double y3, Gosu::Color c3,
                       Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT);

    void draw_quad(double x1, double y1, Gosu::Color c1,
                   double x2, double y2, Gosu::Color c2,
                   double x3, double y3, Gosu::Color c3,
                   double x4, double y4, Gosu::Color c4,
                   Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT);

    void draw_rect(double x, double y, double width, double height, Gosu::Color c,
                   Gosu::ZPos z = 0, Gosu::BlendMode mode = Gosu::BM_DEFAULT);

    void flush();
    void unsafe_gl();
    void unsafe_gl(Gosu::ZPos z);
    void clip_to(double x, double y, double width, double height);

    %newobject record;
    Gosu::Image* record(int width, int height);
    
    %newobject render;
    Gosu::Image* render(int width, int height, VALUE options = 0);
    
    %rename("transform") transform_for_ruby;
    %rename("rotate")    rotate_for_ruby;
    %rename("scale")     scale_for_ruby;
    %rename("translate") translate_for_ruby;

    void transform_for_ruby(double m0, double m1, double m2, double m3, double m4, double m5,
        double m6, double m7, double m8, double m9, double m10, double m11, double m12, double m13,
        double m14, double m15);
    void rotate_for_ruby(double angle, double around_x = 0, double around_y = 0);
    void scale_for_ruby(double factor);
    void scale_for_ruby(double factor_x, double factor_y);
    void scale_for_ruby(double factor_x, double factor_y, double around_x, double around_y);
    void translate_for_ruby(double x, double y);
}
