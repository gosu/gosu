#pragma once

#include <Gosu/Bitmap.hpp>
#include <string>

namespace Gosu
{
    // These functions provide an internal facade around stb_truetype. Maybe it would be nicer if
    // this was a TrueTypeFont class instead of a few C-ish helper functions, but I want the flow
    // of information within Gosu to settle before I refactor this any more.
    // Example: Maybe it would be better if these platform-agnostic helpers here would call the
    // platform-specific methods for loading font files instead of the other way around, because the
    // current design makes it hard to implement things like fallback fonts.
    //
    // These functions do not yet support Gosu::FontFlags. This is fine for system fonts like Arial,
    // where the callers of these methods will typically load the correct file (e.g. ArialBold.ttf).
    // However, games which ship with their own font files (which is a good idea) can't use bold or
    // italic text now using <b> or <i> markup because there is no way to associate one TTF file as
    // the "bold variant" of another. (Of course games can manually use two different font files
    // instead of using Gosu's markup, so this is no showstopper.)
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
    
    // Returns the width a piece of text will occupy on a bitmap when rendered.
    // The caller must ensure that the ttf_data pointer will remain valid until the process exists.
    int text_width_ttf(const void* ttf_data, int font_height,
                       const std::string& text);

    // Renders a string onto a bitmap.
    // Returns the width a piece of text occupies on the bitmap.
    // The caller must ensure that the ttf_data pointer will remain valid until the process exists.
    int draw_text_ttf(const void* ttf_data, int font_height,
                      const std::string& text, Bitmap& bitmap, int x, int y, Color c);
    
    // Returns true if the supplied buffer seems to be a font of the given name.
    bool verify_font_name(const void* ttf_data, const std::string& font_name);
}
