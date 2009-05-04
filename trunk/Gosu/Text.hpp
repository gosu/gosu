//! \file Text.hpp
//! Functions to output text on bitmaps.

#ifndef GOSU_TEXT_HPP
#define GOSU_TEXT_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <string>

namespace Gosu
{
    //! Returns the name of a neutral font that is available on the current
    //! platform.
    std::wstring defaultFontName();

    //! Returns the width a text would span on a bitmap if it were drawn
    //! using drawText with the same arguments.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
    unsigned textWidth(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Draws a line of text on a bitmap.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
    //! \param fontHeight Height, in pixels, of the text.
    //! \param fontFlags Binary combination of members of the FontFlags
    //! enum.
    void drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
        Color c, const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Creates a bitmap that is filled with a line of text given to the function.
    //! The line cannot contain line breaks.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
    //! \param fontHeight Height of the font in pixels.
    //! \param fontFlags Binary combination of members of the FontFlags
    //! enum.
    Bitmap createText(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Creates a bitmap that is filled with the text given to the function.
    //! The text may contain line breaks.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/', does not work on Linux).
    //! \param fontHeight Height of the font in pixels.
    //! \param lineSpacing Spacing between two lines of text in pixels.
    //! \param maxWidth Width of the bitmap that will be returned. Text
    //! will be split into multiple lines to avoid drawing over the right
    //! border. When a single word is too long, it will be truncated.
    //! \param fontFlags Binary combination of members of the FontFlags
    //! enum.
    Bitmap createText(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight, 
        unsigned lineSpacing, unsigned maxWidth, TextAlign align,
        unsigned fontFlags = 0);
}

#endif
