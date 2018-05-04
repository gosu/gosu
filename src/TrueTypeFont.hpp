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
        std::shared_ptr<Impl> pimpl;
        
    public:
        TrueTypeFont(const unsigned char* ttf_data, std::shared_ptr<TrueTypeFont> fallback);
        
        //! Returns the width a string will occupy on a bitmap when rendered at the given height.
        //! The caller must ensure that the ttf_data pointer will remain valid indefinitely.
        double draw_text(const std::u32string& text, double height,
                         Bitmap* bitmap, double x, double y, Color c);
        
        //! Returns true if the supplied buffer seems to be a font of the given name.
        static bool verify_font_name(const unsigned char* ttf_data, const std::string& font_name);
    };
    
    TrueTypeFont& font_by_name(const std::string& font_name, unsigned font_flags);
    
    //! Loads the contents of a file into memory and returns a pointer to it.
    //! The pointer is guaranteed to be valid indefinitely.
    //! In case of failure, this method must not return nullptr, but raise an exception.
    //! Note that this method does not accept any font flags, and so it will always load the first
    //! font in a TTC font collection.
    const unsigned char* ttf_data_from_file(const std::string& filename);
    
    //! This method loads a TODO
    //! This method has a different implementation on each platform.
    //! In case of failure, this method returns nullptr.
    const unsigned char* ttf_data_by_name(const std::string& font_name, unsigned font_flags);
    
    //! This method has a different implementation on each platform.
    //! In case of failure, this method must not return nullptr, but raise an exception.
    const unsigned char* ttf_fallback_data();

    // TODO still true? ↓
    // These functions do not yet support Gosu::FontFlags. This is fine for system fonts like Arial,
    // where the callers of these methods will typically load the correct file (e.g. ArialBold.ttf).
    // However, games which ship with their own font files (which is a good idea) can't use bold or
    // italic text using <b> or <i> markup because there is no way to associate one TTF file as the
    // "bold variant" of another.
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
    
}
