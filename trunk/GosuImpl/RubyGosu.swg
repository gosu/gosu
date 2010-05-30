// Mini Swig Style Improvement List:
// - %predicate instead of %rename
// - use Python names for op overloading

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
%apply unsigned char { boost::uint8_t };
%apply unsigned long { boost::uint32_t };

// Custom typemaps for wchar/wstring.
#pragma SWIG nowarn=-490,-319
%typemap(in) wchar_t {
    VALUE localTemporary = rb_obj_as_string($input);
    std::wstring localTemporary2 = Gosu::utf8ToWstring(StringValueCStr(localTemporary));
	$1 = localTemporary2.empty() ? 0 : localTemporary2.at(0);
}
%typemap(out) wchar_t {
    if ($1 == 0)
        $result = Qnil;
    else
        $result = rb_str_new2(Gosu::wstringToUTF8(std::wstring(1, $1)).c_str());
}
%typemap(in) const std::wstring& (std::wstring temp) {
    VALUE localTemporary = rb_obj_as_string($input);
    temp = Gosu::utf8ToWstring(StringValueCStr(localTemporary));
    $1 = &temp;
}
%apply const std::wstring& { std::wstring, const std::string };
%typemap(out) std::wstring {
    $result = rb_str_new2(Gosu::wstringToUTF8($1).c_str());
}
%typemap(out) const std::wstring& {
    $result = rb_str_new2(Gosu::wstringToUTF8($1).c_str());
}
%typemap(directorin) const std::wstring& {
    $input = rb_str_new2(Gosu::wstringToUTF8($1).c_str());
}
%typemap(directorout) std::wstring {
    VALUE localTemporary = rb_obj_as_string($1);
    $result = Gosu::utf8ToWstring(StringValueCStr(localTemporary));;
}

%typemap(in) Gosu::Button {
    if (NIL_P($input))
        $1 = Gosu::noButton;
    else
        $1 = Gosu::Button(NUM2LONG($input));
}
%typemap(out) Gosu::Button {
	$result = LONG2NUM($1.id());
    if ($result == -1)
        $result = Qnil;
}
%typemap(directorin) Gosu::Button {
	$input = LONG2NUM($1.id());
    if ($input == -1)
        $input = Qnil;
}

// Typemaps for enums that should be given in as symbols.
%typemap(in) Gosu::AlphaMode {
    VALUE localTemporary = rb_obj_as_string($input);
    if (!strcmp(StringValueCStr(localTemporary), "default"))
        $1 = Gosu::amDefault;
    else if (!strcmp(StringValueCStr(localTemporary), "additive"))
        $1 = Gosu::amAdditive;
    else if (!strcmp(StringValueCStr(localTemporary), "multiply"))
        $1 = Gosu::amMultiply;
    else
        SWIG_exception_fail(SWIG_ValueError, "invalid alpha mode");
}
%typemap(in) Gosu::TextAlign {
    VALUE localTemporary = rb_obj_as_string($input);
    if (!strcmp(StringValueCStr(localTemporary), "left"))
        $1 = Gosu::taLeft;
    else if (!strcmp(StringValueCStr(localTemporary), "center"))
        $1 = Gosu::taCenter;
    else if (!strcmp(StringValueCStr(localTemporary), "right"))
        $1 = Gosu::taRight;
    else if (!strcmp(StringValueCStr(localTemporary), "justify"))
        $1 = Gosu::taJustify;
    else
        SWIG_exception_fail(SWIG_ValueError, "invalid text align");
}
%typemap(in) Gosu::Song::Type {
    VALUE localTemporary = rb_obj_as_string($input);
    if (!strcmp(StringValueCStr(localTemporary), "stream"))
        $1 = Gosu::Song::stStream;
    else if (!strcmp(StringValueCStr(localTemporary), "module"))
        $1 = Gosu::Song::stModule;
    else
        SWIG_exception_fail(SWIG_ValueError, "invalid song type");
}

// Allow integral constants to be passed in place of Color values.
%typemap(in) Gosu::Color {    
    void* ptr;
    int res = SWIG_ConvertPtr($input, &ptr, SWIGTYPE_p_Gosu__Color, 0);
    if (!SWIG_IsOK(res))
        // TODO: error checking
        $1 = Gosu::Color(NUM2UINT($input));
    else if (!ptr)
        SWIG_exception_fail(SWIG_ValueError, "invalid null reference of type Gosu::Color");
    else
        $1 = *reinterpret_cast<Gosu::Color*>(ptr);
}

// Header inclusion (order irrelevant)
%module(directors="1") gosu
%{
// Avoid Ruby Macro Hell on Windows...
#undef write
#undef close
#undef read
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

//#ifndef WIN32
//#include <Gosu/Async.hpp>
//#endif

#include <Gosu/Audio.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/Font.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/TextInput.hpp>
#include <Gosu/Timing.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Version.hpp>
#include <Gosu/Window.hpp>

// For Ruby-only MAX_TEXTURE_SIZE
#include <Gosu/../GosuImpl/Graphics/Texture.hpp>
namespace Gosu {
    void enableUndocumentedRetrofication() { extern bool undocumentedRetrofication; undocumentedRetrofication = true; }
    unsigned __maxTextureSize() { return Gosu::Texture::maxTextureSize(); }
}

#include <sstream>

// New Ruby 1.9 syntax (for compilation with Ruby 1.8)
#ifndef RSTRING_LEN
# define RSTRING_LEN(x) RSTRING(x)->len
#endif
#ifndef RSTRING_PTR
# define RSTRING_PTR(x) RSTRING(x)->ptr
#endif
#ifndef RARRAY_LEN
# define RARRAY_LEN(x) RARRAY(x)->len
#endif
#ifndef RARRAY_PTR
# define RARRAY_PTR(x) RARRAY(x)->ptr
#endif

namespace GosusDarkSide
{
    // TODO: Find a way for this to fit into Gosu's design.
    // This can point to a function that wants to be called every
    // frame, e.g. rb_thread_schedule.
    typedef void (*HookOfHorror)();
    extern HookOfHorror oncePerTick;
    
    void yieldToOtherRubyThreads()
    {
		rb_thread_schedule();
    }
}

// Allow filenames and RMagick Images to be passed where Bitmaps are needed.
namespace Gosu
{
    Gosu::Bitmap loadBitmap(VALUE val)
    {
        // Try to treat as filename first.
        if (rb_respond_to(val, rb_intern("to_str")))
        {
            VALUE to_str = rb_funcall(val, rb_intern("to_str"), 0);
            const char* filename = StringValuePtr(to_str);
            return quickLoadBitmap(Gosu::utf8ToWstring(filename));
        }

        // Otherwise, try to call .to_blob on it.
        // (Works with RMagick).
        VALUE conversion = rb_str_new2("to_blob { self.format = 'RGBA'; self.depth = 8 }");
        VALUE blob = rb_obj_instance_eval(1, &conversion, val);
        Check_SafeStr(blob);
        unsigned width = NUM2UINT(rb_funcall(val, rb_intern("columns"), 0));
        unsigned height = NUM2UINT(rb_funcall(val, rb_intern("rows"), 0));
                                 
        if (width * height * 4 != RSTRING_LEN(blob))
            throw std::logic_error("Blob length mismatch!");
        Bitmap result;
        result.resize(width, height);
        const unsigned* rgbaIter = reinterpret_cast<const unsigned*>(RSTRING_PTR(blob));
        for (unsigned y = 0; y < height; ++y)
            for (unsigned x = 0; x < width; ++x)
            {
                unsigned rgba = *rgbaIter;
                result.setPixel(x, y, Gosu::Color(rgba).abgr()); // swap R and B
                ++rgbaIter;
            }
        return result;
    }
}
%}

// Exception wrapping
%exception {
    try {
        $action
    } catch(const std::runtime_error& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// Version constant
%rename("MAJOR_VERSION") GOSU_MAJOR_VERSION;
%rename("MINOR_VERSION") GOSU_MINOR_VERSION;
%rename("POINT_VERSION") GOSU_POINT_VERSION;
%rename("VERSION") GOSU_VERSION;
%include "../Gosu/Version.hpp"

// Miscellaneous functions (timing, math)
%ignore Gosu::sleep;
%include "../Gosu/Timing.hpp"
%ignore Gosu::pi;
%ignore Gosu::distanceSqr;
%ignore Gosu::round;
%ignore Gosu::trunc;
%ignore Gosu::boundBy;
%ignore Gosu::clamp;
%ignore Gosu::wrap;
%ignore Gosu::radiansToGosu;
%ignore Gosu::gosuToRadians;
%include "../Gosu/Math.hpp"
%ignore Gosu::textWidth;
%ignore Gosu::createText;
%ignore Gosu::textHeight;
%ignore Gosu::drawText;
%include "../Gosu/Text.hpp"


// Graphics:

// ZPos, AlphaMode, FontFlags, TextAlign
%ignore Gosu::zImmediate;
%ignore Gosu::AlphaMode;
%ignore Gosu::FontFlags;
%ignore Gosu::TextAlign;
%ignore Gosu::BorderFlags;
%include "../Gosu/GraphicsBase.hpp"

// For screenWidth/screenHeight
%ignore Gosu::Graphics;
%ignore Gosu::BorderFlags;
//%ignore Gosu::MAX_TEXTURE_SIZE;
%include "../Gosu/Graphics.hpp"

// More Ubuntu libgl initialization fail workarounds

//%constant unsigned MAX_TEXTURE_SIZE = Gosu::Texture::maxTextureSize();

namespace Gosu {
    void enableUndocumentedRetrofication() { extern bool undocumentedRetrofication; undocumentedRetrofication = true; }
    unsigned __maxTextureSize() { return Gosu::Texture::maxTextureSize(); }
}

// Color
%rename("alpha=") setAlpha;
%rename("red=") setRed;
%rename("green=") setGreen;
%rename("blue=") setBlue;
%rename("hue=") setHue;
%rename("saturation=") setSaturation;
%rename("value=") setValue;
%include "std_string.i"

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

%include "../Gosu/Color.hpp"

%extend Gosu::Color {
    Gosu::Color dup() const {
        return *$self;
    }

    std::string toS() const {
        std::ostringstream stream;
        stream << "(ARGB: " << static_cast<int>($self->alpha())
               << "/" << static_cast<int>($self->red()) 
               << "/" << static_cast<int>($self->green())
               << "/" << static_cast<int>($self->blue()) << ")";
        return stream.str();
    }
}

// Font
// Hackishly allow the user to pass Window& instead of Graphics&.
%ignore Gosu::Font::Font(Graphics& graphics, const std::wstring& fontName,
    unsigned height, unsigned flags);
%include "../Gosu/Font.hpp"
%extend Gosu::Font {
    Font(Gosu::Window& window, const std::wstring& fontName, unsigned height) {
        return new Gosu::Font(window.graphics(), fontName, height);
    }
}

// AsyncResult

//#ifndef WIN32
//%ignore Gosu::asyncNewImage;
//%ignore Gosu::asyncNewImage_Impl;
//%ignore Gosu::AsyncResult::takeValue;
//%include "../Gosu/Async.hpp"
//%extend Gosu::AsyncResult {
//    %newobject value;
//    Gosu::Image* value() {
//        return $self->takeValue().release();
//    }
//}
//%template(AsyncImageResult) Gosu::AsyncResult<Gosu::Image>;
//#endif

%ignore Gosu::ImageData;
%rename("tex_name") texName;
%include "../Gosu/ImageData.hpp"

// Image
// Allow the user to pass Window& instead of Graphics&.
// Also, provide convenience functions.

// Typemap to return an array of images (for loadTiles)
%typemap(out) std::vector<Gosu::Image*> {
    $result = rb_ary_new2($1.size());
    for (unsigned i = 0; i < $1.size(); i++) {
        VALUE curImg = SWIG_NewPointerObj(SWIG_as_voidptr((*&$1)[i]), SWIGTYPE_p_Gosu__Image, SWIG_POINTER_OWN);
        rb_ary_store($result, i, curImg);
    }
}

%ignore Gosu::Image::Image(Graphics& graphics, const std::wstring& filename, bool tileable = false);
%ignore Gosu::Image::Image(Graphics& graphics, const std::wstring& filename, unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight, bool tileable = false);
%ignore Gosu::Image::Image(Graphics& graphics, const Bitmap& source, bool tileable = false);
%ignore Gosu::Image::Image(Graphics& graphics, const Bitmap& source, unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight, bool tileable = false);
%ignore Gosu::Image::Image(std::auto_ptr<ImageData> data);
%include "../Gosu/Image.hpp"
%extend Gosu::Image {
    Image(Gosu::Window& window, VALUE source, bool tileable = false) {
        return new Gosu::Image(window.graphics(),
            Gosu::loadBitmap(source), tileable);
    }
    Image(Gosu::Window& window, VALUE source, bool tileable,
          unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight) {
        return new Gosu::Image(window.graphics(), Gosu::loadBitmap(source),
            srcX, srcY, srcWidth, srcHeight, tileable);
    }
    #ifndef WIN32
//    %newobject asyncNew;
//    static Gosu::AsyncResult<Gosu::Image>* asyncNew(Gosu::Window& window, const std::wstring& filename) {
//        return new Gosu::AsyncResult<Gosu::Image>(Gosu::asyncNewImage(window, filename));
//    }
    #endif
    void drawAsQuad(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, AlphaMode mode = Gosu::amDefault) {
        $self->getData().draw(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode);
    }
    %newobject glTexInfo;
    Gosu::GLTexInfo* glTexInfo() const {
        boost::optional<Gosu::GLTexInfo> info = $self->getData().glTexInfo();
        if (info)
            return new Gosu::GLTexInfo(*info);
        else
            return 0;
    }
    %newobject fromText4;
    static Gosu::Image* fromText4(Gosu::Window& window, const std::wstring& text,
                                 const std::wstring& fontName, unsigned fontHeight) {
        Gosu::Bitmap bmp = Gosu::createText(text, fontName, fontHeight);
        return new Gosu::Image(window.graphics(), bmp);
    }
    %newobject fromText7;
    static Gosu::Image* fromText7(Gosu::Window& window, const std::wstring& text,
                                 const std::wstring& fontName, unsigned fontHeight,
                                 unsigned lineSpacing, unsigned maxWidth, TextAlign align) {
        Gosu::Bitmap bmp = Gosu::createText(text, fontName, fontHeight, lineSpacing, maxWidth, align);
        return new Gosu::Image(window.graphics(), bmp);
    }
    static std::vector<Gosu::Image*> loadTiles(Gosu::Window& window, VALUE source,
                                               int tileWidth, int tileHeight, bool tileable) {
        std::vector<Gosu::Image*> vec;
        // TODO: const correctness (<- did I mean exception safety?)
        Gosu::imagesFromTiledBitmap(window.graphics(), Gosu::loadBitmap(source),
                                    tileWidth, tileHeight, tileable, vec);
        return vec;        
    }
}

// Audio:

%ignore Gosu::Audio;
%Ignore Gosu::SongType;
%ignore Gosu::Sample::Sample(Audio& audio, const std::wstring& filename);
%ignore Gosu::Sample::Sample(Audio& audio, Reader reader);
%ignore Gosu::Sample::Sample(Reader reader);
%ignore Gosu::Song::Song(Audio& audio, const std::wstring& filename);
%ignore Gosu::Song::Song(Audio& audio, Type type, Reader reader);
%ignore Gosu::Song::Song(Type type, Reader reader);
%rename("playing?") playing;
%rename("paused?") paused;
%rename("volume=") changeVolume;
%rename("pan=") changePan;
%rename("speed=") changeSpeed;
%include "../Gosu/Audio.hpp"

// Input and Window:

// Button ID constants
// Does not matter that it's the Mac file, %include only analyses symbols to export, not their values.
%include "../Gosu/ButtonsMac.hpp"
%init %{
    GosusDarkSide::oncePerTick = GosusDarkSide::yieldToOtherRubyThreads;
%}

// TextInput
%ignore Gosu::TextInput::feedNSEvent(void*);
%ignore Gosu::TextInput::feedMessage(unsigned long, unsigned long, unsigned long);
%ignore Gosu::TextInput::feedButtonId(unsigned);
%ignore Gosu::TextInput::caretPos() const;
%ignore Gosu::TextInput::selectionStart() const;
%rename("text=") setText;
%rename("caret_pos=") setCaretPos;
%rename("selection_start=") setSelectionStart;
%include "../Gosu/TextInput.hpp"
%extend Gosu::TextInput {
    // Fix indices into UTF-8 string
    unsigned caret_pos() const {
        return Gosu::wstringToUTF8($self->text().substr(0, $self->caretPos())).size();
    }
    unsigned selection_start() const {
        return Gosu::wstringToUTF8($self->text().substr(0, $self->selectionStart())).size();
    }
};

// Window
%rename("caption=") setCaption;
%rename("button_down?") isButtonDown;
%rename("text_input=") setTextInput;
%rename("mouse_x=") setMouseX;
%rename("mouse_y=") setMouseY;
%rename("needs_redraw?") needsRedraw;
%rename("fullscreen?") fullscreen;
%markfunc Gosu::Window "markWindow";
%include "../Gosu/Window.hpp"

%header %{
    static void markWindow(void* window) {
        Gosu::TextInput* ti = static_cast<Gosu::Window*>(window)->input().textInput();
        if (VALUE ti_value = SWIG_RubyInstanceFor(ti))
            rb_gc_mark(ti_value);
    }
%}

%extend Gosu::Window {
    void drawLine(double x1, double y1, Gosu::Color c1,
                  double x2, double y2, Gosu::Color c2,
                  Gosu::ZPos z = 0, Gosu::AlphaMode mode = Gosu::amDefault) {
        $self->graphics().drawLine(x1, y1, c1, x2, y2, c2,
                                   z, mode);
    }
    void drawTriangle(double x1, double y1, Gosu::Color c1,
                      double x2, double y2, Gosu::Color c2,
                      double x3, double y3, Gosu::Color c3,
                      Gosu::ZPos z = 0, Gosu::AlphaMode mode = Gosu::amDefault) {
        $self->graphics().drawTriangle(x1, y1, c1, x2, y2, c2, x3, y3, c3,
                                       z, mode);
    }
    /*void drawRect(double x, double y, double w, double h, Gosu::Color c,
                  Gosu::ZPos z = 0, Gosu::AlphaMode mode = Gosu::amDefault) {
        $self->graphics().drawRect(x, y, w, h, c, z, mode);
    }*/
    void drawQuad(double x1, double y1, Gosu::Color c1,
                  double x2, double y2, Gosu::Color c2,
                  double x3, double y3, Gosu::Color c3,
                  double x4, double y4, Gosu::Color c4,
                  Gosu::ZPos z = 0, Gosu::AlphaMode mode = Gosu::amDefault) {
        $self->graphics().drawQuad(x1, y1, c1, x2, y2, c2,
                                   x3, y3, c3, x4, y4, c4,
                                   z, mode);
    }
    bool isButtonDown(Gosu::Button btn) const {
        return $self->input().down(btn);
    }
    static Gosu::Button charToButtonId(wchar_t ch) {
        return Gosu::Input::charToId(ch);
    }
    static wchar_t buttonIdToChar(Gosu::Button btn) {
        return Gosu::Input::idToChar(btn);
    }
    
    TextInput* textInput() const {
        return $self->input().textInput();
    }
    
    void setTextInput(TextInput* ti) {
        $self->input().setTextInput(ti);
    }
    
	double mouseX() const {
		return $self->input().mouseX();
	}
	double mouseY() const {
		return $self->input().mouseY();
	}
    void setMousePosition(double x, double y) {
        $self->input().setMousePosition(x, y);
    }
    void setMouseX(double x) {
        $self->input().setMousePosition(x, $self->input().mouseY());
    }
    void setMouseY(double y) {
        $self->input().setMousePosition($self->input().mouseX(), y);
    }
    
	int width() const {
		return $self->graphics().width();
	}
	int height() const {
		return $self->graphics().height();
	}
    bool fullscreen() const {
        return $self->graphics().fullscreen();
    }
    
    void gl() {
        $self->graphics().beginGL();
        rb_yield(Qnil);
        $self->graphics().endGL();
    }
    void clipTo(int x, int y, unsigned width, unsigned height) {
        $self->graphics().beginClipping(x, y, width, height);
        rb_yield(Qnil);
        $self->graphics().endClipping();
    }
    %newobject record;
    Gosu::Image* record() {
        $self->graphics().beginRecording();
        rb_yield(Qnil);
        return new Gosu::Image($self->graphics().endRecording());
    }
};
