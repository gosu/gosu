//! \file Text.hpp
//! Functions to output text on bitmaps.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <string>

namespace Gosu
{
    //! Returns the name of a neutral font that is available on the current platform.
    std::string default_font_name();

    //! Returns the width an unformatted line of text would span on a bitmap if it were drawn using
    //! draw_text with the same arguments. This is a very low-level function that does not
    //! understand any of Gosu's HTML-like markup.
    //! \param text Unformatted text.
    //! \param font_name Name of a system font, or a filename to a TTF file (must contain '/' or '.').
    int text_width(const std::string& text, const std::string& font_name, int font_height,
                   unsigned font_flags = 0);

    //! Draws a line of unformatted text on a bitmap. This is a very low-level function that does
    //! not understand any of Gosu's HTML-like markup.
    //! \param text Unformatted text.
    //! \param font_name Name of a system font, or fileame of a TTF file (must contain '/' or
    //!        '.').
    //! \param font_height Height, in pixels, of the text.
    //! \param font_flags Binary combination of members of the FontFlags enum.
    int draw_text(Bitmap& bitmap, const std::string& text, int x, int y, Color c,
                  const std::string& font_name, int font_height, unsigned font_flags = 0);

    //! Creates a bitmap that is filled with a line of formatted text given to the function.
    //! The line can contain line breaks and HTML-like markup.
    //! \param text Formatted text.
    //! \param font_name Name of a system font, or fileame of a TTF file (must contain '/' or
    //!        '.').
    //! \param font_height Height of the font in pixels.
    //! \param font_flags Binary combination of members of the FontFlags enum.
    Bitmap create_text(const std::string& text, const std::string& font_name, int font_height,
                       unsigned font_flags = 0);

    //! Creates a bitmap that is filled with the formatted text given to the function.
    //! The line can contain line breaks and HTML-like markup.
    //! \param text Formatted text.
    //! \param font_name Name of a system font, or fileame of a TTF file (must contain '/' or
    //!        '.').
    //! \param font_height Height of the font in pixels.
    //! \param line_spacing Spacing between two lines of text in pixels. Can be negative to make
    //!        text stick together more closely.
    //! \param width Width of the bitmap that will be returned.
    //!        Text will be split into multiple lines to avoid drawing over the right border.
    //!        When a single word is too long, it will be truncated.
    //! \param font_flags Binary combination of members of the FontFlags enum.
    Bitmap create_text(const std::string& text, const std::string& font_name, int font_height,
                       int line_spacing, int width, Alignment align, unsigned font_flags = 0);
}
