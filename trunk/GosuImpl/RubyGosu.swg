// SWIG improvements related to Ruby:
// - %predicate instead of %rename
// - use Python names for op overloading
// - %trackobjects

%include exception.i  

// The docs make it seem like this is a generally good idea.
%feature("compactdefaultargs");

// Overriding virtuals is only necessary in Gosu::Window.
%feature("director") Gosu::Window;

// Resolve typedefs that SWIG doesn't recognize.
%apply unsigned char { boost::uint8_t };
%apply unsigned long { boost::uint32_t };

// Custom typemaps for wchar/wstring.
#pragma SWIG nowarn=-490,-319
%typemap(in) wchar_t {
    VALUE localTemporary = rb_obj_as_string($input);
    $1 = Gosu::utf8ToWstring(StringValueCStr(localTemporary)).at(0);
}
%typemap(out) wchar_t {
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

%typemap(in) Gosu::Button {
	$1 = Gosu::Button(NUM2LONG($input));
}
%typemap(out) Gosu::Button {
	$result = LONG2NUM($1.getId());
}
%typemap(directorin) Gosu::Button {
	$input = LONG2NUM($1.getId());
}

// Typemaps for enums that should be given in as symbols.
%typemap(in) Gosu::AlphaMode {
    VALUE localTemporary = rb_obj_as_string($input);
    if (!strcmp(StringValueCStr(localTemporary), "default"))
        $1 = Gosu::amDefault;
    else if (!strcmp(StringValueCStr(localTemporary), "additive"))
        $1 = Gosu::amDefault;
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
#include <Gosu/Timing.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>

#include <sstream>
%}

// Exception wrapping
%exception {
    try {
        $action
    } catch(const std::runtime_error& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    }
}

// Miscellaneous functions (timing, math)
%ignore Gosu::Sleep;
%include "../Gosu/Timing.hpp"
%ignore Gosu::pi;
%include "../Gosu/Math.hpp"
%ignore Gosu::textWidth;
%ignore Gosu::createText;
%ignore Gosu::drawText;
%include "../Gosu/Text.hpp"


// Graphics:

// ZPos, AlphaMode, FontFlags, TextAlign
%include "../Gosu/GraphicsBase.hpp"

// Color
%rename("alpha=") setAlpha;
%rename("red=") setRed;
%rename("green=") setGreen;
%rename("blue=") setBlue;
%include "std_string.i"
%include "../Gosu/Color.hpp"
%extend Gosu::Color {
    std::string toS() const {
        std::ostringstream stream;
        stream << "(" << static_cast<int>($self->alpha())
               << "," << static_cast<int>($self->red()) 
               << "," << static_cast<int>($self->green())
               << "," << static_cast<int>($self->blue()) << ")";
        return stream.str();
    }
}

// Font
// Hackishly allow the user to pass Window& instead of Graphics&.
%ignore Gosu::Font::Font(Graphics& graphics, const std::wstring& fontName, unsigned height);
%include "../Gosu/Font.hpp"
%extend Gosu::Font {
    Font(Gosu::Window& window, const std::wstring& fontName, unsigned height) {
        return new Gosu::Font(window.graphics(), fontName, height);
    }
}

// Image
// Hackishly allow the user to pass Window& instead of Graphics&.
// Also, provide convenience functions.

// Typemap to return an array of images (for loadTiles)
%typemap(out) std::vector<Gosu::Image*> {
    $result = rb_ary_new2($1.size());
    for (unsigned i = 0; i < $1.size(); i++) {
        VALUE curImg = SWIG_NewPointerObj(SWIG_as_voidptr((*&$1)[i]), SWIGTYPE_p_Gosu__Image, SWIG_POINTER_OWN);
        rb_ary_store($result, i, curImg);
    }
}

%ignore Gosu::Image::Image(Graphics& graphics, const std::wstring& filename, bool hardBorders = false);
%ignore Gosu::Image::Image(Graphics& graphics, const std::wstring& filename, unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight, bool hardBorders = false);
%ignore Gosu::Image::Image(Graphics& graphics, const Bitmap& source, bool hardBorders = false);
%ignore Gosu::Image::Image(Graphics& graphics, const Bitmap& source, unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight, bool hardBorders = false);
%include "../Gosu/Image.hpp"
%extend Gosu::Image {
    Image(Gosu::Window& window, const std::string& filename, bool hardBorders = false) {
        return new Gosu::Image(window.graphics(), Gosu::utf8ToWstring(filename), hardBorders);
    }
    Image(Gosu::Window& window, const std::string& filename, bool hardBorders,
          unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight) {
        return new Gosu::Image(window.graphics(), Gosu::utf8ToWstring(filename),
                               srcX, srcY, srcWidth, srcHeight, hardBorders);
    }
    void drawAsQuad(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, AlphaMode mode = Gosu::amDefault) {
        $self->getData().draw(x1, y1, c1, x2, y2, c2, x3, y3, c3, x4, y4, c4, z, mode);
    }
    %newobject fromBlob;
    static Gosu::Image* fromBlob(Gosu::Window& window, const std::string& blob, int width, int height,
                                 bool hardBorders = false) {
        if (width * height * 4 != blob.length())
            throw std::logic_error("Blob length mismatch!");
        Gosu::Bitmap bmp;
        bmp.resize(width, height);
        const unsigned* rgbaIter = reinterpret_cast<const unsigned*>(blob.data());
        for (unsigned y = 0; y < height; ++y)
            for (unsigned x = 0; x < width; ++x)
            {
                unsigned rgba = *rgbaIter;
                bmp.setPixel(x, y, Gosu::Color(rgba).abgr()); //((rgba & 0xff000000) >> 8) | ((rgba & 0x00ffffff) << 8));
                ++rgbaIter;
            }
        return new Gosu::Image(window.graphics(), bmp, hardBorders);
    }
    %newobject fromText;
    static Gosu::Image* fromText(Gosu::Window& window, const std::wstring& text,
                                   const std::wstring& fontName, unsigned fontHeight,
                                   unsigned lineSpacing, unsigned maxWidth, TextAlign align) {
        Gosu::Bitmap bmp = Gosu::createText(text, fontName, fontHeight, lineSpacing, maxWidth, align);
        return new Gosu::Image(window.graphics(), bmp);
    }
    static std::vector<Gosu::Image*> loadTiles(Gosu::Window& window, const std::wstring& filename,
                                               int tileWidth, int tileHeight, bool hardBorders) {
        Gosu::Bitmap bmp = Gosu::quickLoadBitmap(filename);
        std::vector<Gosu::Image*> vec;
        // TODO: const correctness (<- did I mean exception safety?)
        Gosu::imagesFromTiledBitmap(window.graphics(), bmp,
                                    tileWidth, tileHeight, hardBorders, vec);
        return vec;        
    }
}

// Audio:

%ignore Gosu::Audio;
%ignore Gosu::Sample::Sample(Audio& audio, const std::wstring& filename);
%ignore Gosu::Sample::Sample(Audio& audio, Reader reader);
%ignore Gosu::Song::Song(Audio& audio, const std::wstring& filename);
%ignore Gosu::Song::Song(Audio& audio, Type type, Reader reader);
%rename("playing?") playing;
%include "../Gosu/Audio.hpp"
%extend Gosu::Sample {
    Sample(Gosu::Window& window, const std::string& filename) {
        return new Gosu::Sample(window.audio(), Gosu::utf8ToWstring(filename));
    }
}
%extend Gosu::Song {
    Song(Gosu::Window& window, const std::string& filename) {
        return new Gosu::Song(window.audio(), Gosu::utf8ToWstring(filename));
    }
}


// Input and Window:

// Button ID constants
%include "../Gosu/ButtonsMac.hpp"
// Backwards compatibility: import the constants into Gosu::Button.
%init %{
    rb_eval_string("module Gosu::Button; Gosu.constants.each { |c| const_set(c, Gosu.const_get(c)) }; end");
%}

// Window
%rename("caption=") setCaption;
%rename("button_down?") isButtonDown;
%include "../Gosu/Window.hpp"
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
    Gosu::Button charToButtonId(wchar_t ch) {
        return $self->input().charToId(ch);
    }
    wchar_t buttonIdToChar(Gosu::Button btn) {
        return $self->input().idToChar(btn);
    }
	double mouseX() const {
		return $self->input().mouseX();
	}
	double mouseY() const {
		return $self->input().mouseY();
	}
	int width() const {
		return $self->graphics().width();
	}
	int height() const {
		return $self->graphics().height();
	}
};
