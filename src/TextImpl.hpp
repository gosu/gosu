#pragma once

#include <Gosu/Bitmap.hpp>
#include <string>

namespace Gosu
{
    // These functions do not yet support Gosu::FontFlags. This is fine for system fonts like Arial,
    // where the callers of these methods will typically load the correct file (e.g. ArialBold.ttf).
    // However, games which ship with their own font files (which is a good idea) can't use bold or
    // italic text now because there is no way to associate one TTF file as the "bold variant" of
    // another.
    //
    // Options for the future:
    // 1. Use stbtt_FindMatchingFont. This will only work for TTC font collections, and we will
    //    have to patch stb_truetype to look for fonts only based on `int flags`, while ignoring
    //    the name of fonts inside a bundle (who wants to deal with strings, anyway).
    // 2. Maybe Gosu should accept filename patterns like "LibreBaskerville-*.ttf" as the font
    //    name and then replace the * with "Regular", "Bold", "Italic" etc.?
    // 3. As a last resort, Gosu could implement faux bold and faux italics. I think faux
    //    underlines are a must anyway, since no font provides a dedicated TTF file for that.
    // These options are not mutually exclusive.
    
    // The caller must ensure that the ttf_data pointer will remain valid until the process exists.
    int text_width_ttf(const void* ttf_data, int font_height,
                       const std::string& text);

    // The caller must ensure that the ttf_data pointer will remain valid until the process exists.
    void draw_text_ttf(const void* ttf_data, int font_height,
                       const std::string& text, Bitmap& bitmap, int x, int y, Color c);
    
    // Returns true if the supplied buffer seems to be a font of the given name.
    bool verify_font_name(const void* ttf_data, const std::string& font_name);
}
