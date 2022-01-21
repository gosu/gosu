#pragma once

#include <Gosu/Bitmap.hpp>
#include <Gosu/IO.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Gosu
{
    class TrueTypeFont
    {
        struct Impl;
        std::shared_ptr<Impl> m_impl;

    public:
        /// The caller must ensure that the ttf_data pointer will outlive this object.
        TrueTypeFont(const unsigned char* ttf_data, std::shared_ptr<TrueTypeFont> fallback);

        /// Returns the right edge of a string when rendered onto a bitmap at the given position,
        /// and with the given height.
        /// If (bitmap != nullptr), the text is also rendered onto the bitmap.
        double draw_text(const std::u32string& text, double height, //
                         Bitmap* bitmap, double x, double y, Color c);

        /// Returns true if the supplied buffer seems to be a font of the given name.
        static bool matches(const unsigned char* ttf_data, //
                            const std::string& font_name, unsigned font_flags);
    };

    TrueTypeFont& font_by_name(const std::string& font_name, unsigned font_flags);

    /// Loads the contents of a file into memory and returns a pointer to it.
    /// The pointer is guaranteed to be valid indefinitely.
    /// In case of failure, this method must not return nullptr, but raise an exception.
    /// Note that this method does not accept any font flags, and so it will always load the first
    /// font in a TTC font collection.
    ///
    /// This function does not yet support Gosu::FontFlags, and consequently, custom TTF fonts do
    /// not support markup or bold/italic text right now.
    ///
    /// Options for the future:
    /// 1. Use stbtt_FindMatchingFont. This will only work for TTC font collections, and we will
    ///    have to patch stb_truetype to look for fonts only based on `int flags`, while ignoring
    ///    the name of fonts inside a bundle (because we don't want to deal with strings).
    /// 2. Maybe Gosu should accept filename patterns like "LibreBaskerville-*.ttf" as the font
    ///    name and then replace the * with "Regular", "Bold", "Italic" etc.?
    /// 3. As a last resort, Gosu could implement faux bold and faux italics. I think faux
    ///    underlines are a must anyway, since no font provides a dedicated TTF file for that.
    /// These options are not mutually exclusive.
    const unsigned char* ttf_data_from_file(const std::string& filename);

    /// This method loads a system font with the given flags.
    /// This method has a different implementation on each platform.
    /// In case of failure, this method returns nullptr.
    const unsigned char* ttf_data_by_name(const std::string& font_name, unsigned font_flags);

    /// Returns the TTF data of a font that supports as many glyphs as possible.
    /// This method has a different implementation on each platform.
    /// In case of failure, this method must not return nullptr, but raise an exception.
    const unsigned char* ttf_fallback_data();
}
