//! \file Text.hpp
//! Functions to output text on bitmaps.

#ifndef GOSU_TEXT_HPP
#define GOSU_TEXT_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <string>

namespace Gosu
{
    //! Returns the name of a neutral font that is available on the current
    //! platform.
    std::wstring defaultFontName();

    //! Returns the width a text would span on a bitmap if it were drawn
    //! using drawText with the same arguments.
    unsigned textWidth(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Draws a line of text on a bitmap.
    //! \param fontHeight Height, in pixels, of the text.
    //! \param fontFlags Binary combination of members of the FontFlags
    //! enum.
    void drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
        Color c, const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Creates a bitmap that is filled with the text given to the function.
    //! The text may contain line breaks.
    //! \param fontHeight Height of the font in pixels.
    //! \param lineSpacing Spacing between two lines of text in pixels.
    //! \param maxWidth Maximal width of the bitmap that will be returned. Text
    //! will be split into multiple lines to avoid drawing over the right
    //! border. When a single word is too long, it will be truncated.
    Bitmap createText(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight, 
        unsigned lineSpacing, unsigned maxWidth, TextAlign align,
        unsigned fontFlags = 0);
    
    //! Returns the height that a text block created by createText with the
    //! same arguments would give.
    //! Experimental for now, used for finding the right font size to fill
    //! a given block of text. If a better method for this is included into
    //! Gosu, this function will become deprecated and, later, removed.
    unsigned textHeight(const std::wstring& text,
          const std::wstring& fontName, unsigned fontHeight, 
          unsigned lineSpacing, unsigned maxWidth, unsigned fontFlags = 0);
}

#endif
