//! \file Text.hpp
//! Functions to output text on bitmaps.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <string>

namespace Gosu
{
    //! Returns the name of a neutral font that is available on the current
    //! platform.
    std::wstring defaultFontName();

    //! Returns the width an unformatted line of text would span on a bitmap if it were drawn using
    //! drawText with the same arguments. This is a very low-level function that does not understand
    //! any of Gosu's HTML-like markup.
    //! \param text Unformatted text.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/').
    unsigned textWidth(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Draws a line of unformatted text on a bitmap. This is a very low-level function that does not understand
    //! any of Gosu's HTML-like markup.
    //! \param text Unformatted text.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/').
    //! \param fontHeight Height, in pixels, of the text.
    //! \param fontFlags Binary combination of members of the FontFlags
    //! enum.
    void drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
        Color c, const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Creates a bitmap that is filled with a line of formatted text given to the function.
    //! The line can contain line breaks and HTML-like markup.
    //! \param text Formatted text.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/').
    //! \param fontHeight Height of the font in pixels.
    //! \param fontFlags Binary combination of members of the FontFlags
    //! enum.
    Bitmap createText(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight,
        unsigned fontFlags = 0);

    //! Creates a bitmap that is filled with the formatted text given to the function.
    //! The line can contain line breaks and HTML-like markup.
    //! \param text Formatted text.
    //! \param fontName Name of a system font, or a filename to a TTF file (must contain '/').
    //! \param fontHeight Height of the font in pixels.
    //! \param lineSpacing Spacing between two lines of text in pixels. Can be negative to make
    //! text stick together more closely.
    //! \param width Width of the bitmap that will be returned. Text
    //! will be split into multiple lines to avoid drawing over the right
    //! border. When a single word is too long, it will be truncated.
    //! \param fontFlags Binary combination of members of the FontFlags
    //! enum.
    Bitmap createText(const std::wstring& text,
        const std::wstring& fontName, unsigned fontHeight, 
        int lineSpacing, unsigned width, TextAlign align,
        unsigned fontFlags = 0);
    
    //! Registers a new HTML-style entity that can subsequently be used
    //! with Gosu::Font and Gosu::createText. The name is given without & and ;.
    void registerEntity(const std::wstring& name, const Bitmap& replacement);
}
