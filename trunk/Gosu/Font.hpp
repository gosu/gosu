//! \file Font.hpp
//! Interface of the Font class.

#ifndef GOSU_FONT_HPP
#define GOSU_FONT_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

namespace Gosu
{
    //! A font can be used to draw text on a Graphics object very flexibly.
    //! Fonts are ideal for small texts that change regularly. For large,
    //! static texts you should use createBitmap and turn the result into
    //! an image.
    class Font
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        //! Constructs a font that can be drawn onto the graphics object.
        //! \param fontName Name of a system font, or a filename to a TTF
        //!        file (must contain '/', does not work on Linux).
        //! \param fontHeight Height of the font, in pixels.
        //! \param fontFlags Flags used to render individual characters of
        //!        the font.
        Font(Graphics& graphics, const std::wstring& fontName,
            unsigned fontHeight, unsigned fontFlags = ffBold);
        ~Font();
        
        //! Returns the name of the font that was used to create it.
        std::wstring name() const;
        
        //! Returns the height of the font, in pixels.
        unsigned height() const;
        
        //! Returns the flags used to create the font characters.
        unsigned flags() const;
        
        //! Returns the width, in pixels, the given text would occupy if drawn.
        double textWidth(const std::wstring& text, double factorX = 1) const;
        
        //! Draws text so the top left corner of the text is at (x; y).
        void draw(const std::wstring& text, double x, double y, ZPos z,
            double factorX = 1, double factorY = 1,
            Color c = Color::WHITE, AlphaMode mode = amDefault) const;
        
        //! Draws text at a position relative to (x; y).
        //! \param relX Determines where the text is drawn horizontally. If
        //! relX is 0.0, the text will be to the right of x, if it is 1.0,
        //! the text will be to the left of x, if it is 0.5, it will be
        //! centered on x. Of course, all real numbers are possible values.
        //! \param relY See relX.
        void drawRel(const std::wstring& text, double x, double y, ZPos z,
            double relX, double relY, double factorX = 1, double factorY = 1,
            Color c = Color::WHITE, AlphaMode mode = amDefault) const;
        
        //! Analogous to draw, but rotates the text by a given angle.
        void drawRot(const std::wstring& text, double x, double y, ZPos z, double angle,
            double factorX = 1, double factorY = 1,
            Color c = Color::WHITE, AlphaMode mode = amDefault) const;
    };
}

#endif
