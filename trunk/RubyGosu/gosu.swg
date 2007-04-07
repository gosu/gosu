// The docs make it seem like this is a generally good idea.
%feature("compactdefaultargs");

// Resolve typedefs that SWIG doesn't recognize.
%apply unsigned char { boost::uint8_t };
%apply unsigned long { boost::uint32_t };

// Custom typemaps for wchar/wstring.
#pragma SWIG nowarn=-490
%typemap(in) wchar_t {
    VALUE localTemporary = rb_obj_as_string($input);
    $1 = Gosu::widen(StringValueCStr(localTemporary)).at(0);
}
%typemap(out) wchar_t {
    $result = rb_str_new2(Gosu::narrow(std::wstring(1, $1)).c_str());
}
%typemap(in) const std::wstring& (std::wstring temp) {
    VALUE localTemporary = rb_obj_as_string($input);
    temp = Gosu::widen(StringValueCStr(localTemporary));
    $1 = &temp;
}
%typemap(out) std::wstring {
    $result = rb_str_new2(Gosu::narrow($1).c_str());
}

// Header inclusion (order irrelevant)
%module gosu
%{
#include <Gosu/Audio.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/Font.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>
    
#include <sstream>
    
// Local helper function to load an image from a filename, regardless of format.
Gosu::Bitmap loadImage(const std::wstring& filename) {
    Gosu::Bitmap result;
    
    Gosu::Buffer buf;
    Gosu::loadFile(buf, filename);
    
    char formatTester[2];
    buf.frontReader().read(formatTester, sizeof formatTester);
    if (formatTester[0] == 'B' && formatTester[1] == 'M')
    {
        Gosu::loadFromBMP(result, buf.frontReader());
        applyColorKey(result, Gosu::Colors::fuchsia);
    }
    else
        Gosu::loadFromPNG(result, buf.frontReader());
    
    return result;
}
%}

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
        stream << "(" << static_cast<int>($self->alpha()) << "," << $self->red() << "," 
               << $self->green() << "," << $self->blue() << ")";
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
// Also, translate filenames into images and add convenience functions.
%include "std_vector.i"
%template(ImageVector) std::vector<Gosu::Image*>;
%ignore Gosu::Image::Image(Graphics& graphics, const Bitmap& source, bool hardBorders = false);
%ignore Gosu::Image::Image(Graphics& graphics, const Bitmap& source, unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight, bool hardBorders = false);
%include "../Gosu/Image.hpp"
%extend Gosu::Image {
    Image(Gosu::Window& window, const std::wstring& filename, bool hardBorders = false) {
        return new Gosu::Image(window.graphics(), loadImage(filename), hardBorders);
    }
    Image(Gosu::Window& window, const std::wstring& filename,
          unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
          bool hardBorders = false) {
        return new Gosu::Image(window.graphics(), loadImage(filename),
                               srcX, srcY, srcWidth, srcHeight, hardBorders);
    }
    static Gosu::Image* createText(Gosu::Window& window, const std::wstring& text,
                                   const std::wstring& fontName, unsigned fontHeight,
                                   unsigned lineSpacing, unsigned maxWidth, TextAlign align) {
        Gosu::Bitmap bmp = Gosu::createText(text, fontName, fontHeight, lineSpacing, maxWidth, align);
        return new Gosu::Image(window.graphics(), bmp);
    }
    static std::vector<Gosu::Image*> loadTiles(Gosu::Window& window, const std::wstring& filename,
                                               int tileWidth, int tileHeight, bool hardBorders) {
        Gosu::Bitmap bmp = loadImage(filename);
        std::vector<Gosu::Image*> vec;
        // TODO: const correctness
        Gosu::imagesFromTiledBitmap(window.graphics(), bmp,
                                    tileWidth, tileHeight, hardBorders, vec);
        return vec;        
    }
}

// Audio:

%ignore Gosu::Audio;
%ignore Gosu::Sample::Sample(Audio& audio, Reader reader);
%ignore Gosu::Song::Song(Audio& audio, Type type, Reader reader);
%include "../Gosu/Audio.hpp"
%extend Gosu::Sample {
    Sample(Gosu::Window& window, const std::wstring& filename) {
        Gosu::Buffer buffer;
        Gosu::loadFile(buffer, filename);
        return new Gosu::Sample(window.audio(), buffer.frontReader());
    }
}
%extend Gosu::Song {
    Song(Gosu::Window& window, Type type, const std::wstring& filename) {
        Gosu::Buffer buffer;
        Gosu::loadFile(buffer, filename);
        return new Gosu::Song(window.audio(), type, buffer.frontReader());
    }
}


// Input and Window:

// Button ID constants
%include "../Gosu/ButtonsMac.hpp"

// Window
%rename("caption=") setCaption;
%rename("button_down?") isButtonDown;
%include "../Gosu/Window.hpp"
%extend Gosu::Window {
    void drawQuad(double x1, double y1, Gosu::Color c1,
                  double x2, double y2, Gosu::Color c2,
                  double x3, double y3, Gosu::Color c3,
                  double x4, double y4, Gosu::Color c4,
                  Gosu::ZPos z = 0, Gosu::AlphaMode mode = Gosu::amDefault) {
        $self->graphics().drawQuad(x1, y1, c1, x2, y2, c2,
                                   x3, y3, c3, x4, y4, c4,
                                   z, mode);
    }
    bool isButtonDown(unsigned long btnId) const {
        return $self->input().down(btnId);
    }
    unsigned long charToButtonId(wchar_t ch) {
        return $self->input().charToId(ch);
    }
    wchar_t buttonIdToChar(unsigned long btnId) {
        return $self->input().idToChar(btnId);
    }
};
