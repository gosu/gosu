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
    //! draw_text with the same arguments.
    //! \param text A UCS-4 string, normalization: NFC.
    //! \param font_name Name of a system font, or filename of a TTF file (must contain '/' or '.').
    double text_width(const std::u32string& text, const std::string& font_name, double font_height,
                      unsigned font_flags = 0);

    //! Draws a line of plain text on a bitmap. This is a low-level function that does not
    //! understand any of Gosu's HTML-like markup.
    //! \param text A UCS-4 string, normalization: NFC.
    //! \param font_name Name of a system font, or filename of a TTF file (must contain '/' or '.').
    //! \param font_height Height, in pixels, of the text.
    //! \param font_flags Binary combination of members of the FontFlags enum.
    double draw_text(Bitmap& bitmap, double x, double y, Color c, const std::u32string& text,
                     const std::string& font_name, double font_height, unsigned font_flags = 0);

    //! Creates a bitmap that is filled with the plain text given to the function.
    //! The line can contain line breaks and HTML-like markup.
    //! \param text Plain text.
    //! \param font_name Name of a system font, or filename of a TTF file (must contain '/' or '.').
    //! \param font_height Height of the font in pixels.
    //! \param line_spacing Spacing between two lines of text in pixels. Can be negative to make
    //!        text stick together more closely.
    //! \param width Width of the bitmap that will be returned.
    //!        Text will be split into multiple lines to avoid drawing over the right border.
    //!        When a single word is too long, it will be truncated.
    //!        A width smaller than 0 indicates that lines should not be wrapped, and the resulting
    //!        bitmap will be as wide as the widest line.
    //! \param font_flags Binary combination of members of the FontFlags enum.
    Bitmap layout_text(const std::string& text, const std::string& font_name,
                       double font_height, double line_spacing = 0,
                       int width = -1, Alignment align = AL_LEFT, unsigned font_flags = 0);
    
    //! Creates a bitmap that is filled with the formatted text given to the function.
    //! The line can contain line breaks and HTML-like markup.
    //! \param markup Formatted text.
    //! \param font_name Name of a system font, or filename of a TTF file (must contain '/' or '.').
    //! \param font_height Height of the font in pixels.
    //! \param line_spacing Spacing between two lines of text in pixels. Can be negative to make
    //!        text stick together more closely.
    //! \param width Width of the bitmap that will be returned.
    //!        Text will be split into multiple lines to avoid drawing over the right border.
    //!        When a single word is too long, it will be truncated.
    //!        A width smaller than 0 indicates that lines should not be wrapped, and the resulting
    //!        bitmap will be as wide as the widest line.
    //! \param font_flags Binary combination of members of the FontFlags enum.
    Bitmap layout_markup(const std::string& markup, const std::string& font_name,
                         double font_height, double line_spacing = 0,
                         int width = -1, Alignment align = AL_LEFT, unsigned font_flags = 0);
}
