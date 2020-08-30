#include "TrueTypeFont.hpp"
#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>
#include <stdexcept>

// Disable comma warnings in stb headers.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomma"
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <map>
using namespace std;

struct Gosu::TrueTypeFont::Impl
{
    stbtt_fontinfo info;
    shared_ptr<TrueTypeFont> fallback;
    
    // The ascent in internal font metrics (an arbitrary integer scale), that is, the part of the
    // font above the baseline, which TrueType considers to be at y = 0.
    int ascent;
    
    // Scaling factor from internal font metrics (an arbitrary integer scale) to a font with
    // height = 1px.
    double base_scale;
    
    Impl(const unsigned char* ttf_data, shared_ptr<TrueTypeFont> fallback)
    : fallback(move(fallback))
    {
        auto offset = stbtt_GetFontOffsetForIndex(ttf_data, 0);
        int success = stbtt_InitFont(&info, ttf_data, offset);
        if (!success) throw runtime_error("Invalid TrueType font data");
        
        // Calculate metrics.
        int descent, lineGap;
        stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
        int height = ascent - descent + lineGap;
        base_scale = 1.0f / height;
    }
    
    // This method always measures text, and also draws it if (bitmap != nullptr).
    double draw_text(const u32string& text, bool is_end, double height,
                     Bitmap* bitmap, double x, double y, Color c)
    {
        if (text.empty()) return 0;

        // The 'x' parameter is used as the running cursor variable in this method.
        
        double scale = base_scale * height;
        int last_glyph = 0;
        int last_advance = 0;

        for (u32string::size_type index = 0; index < text.size(); ++index) {
            auto codepoint = text[index];
            // Silently skip control characters, including the \r in Windows-style line breaks.
            if (codepoint < ' ') continue;
            
            int glyph = stbtt_FindGlyphIndex(&info, codepoint);
            // Handle missing characters in this font...
            if (glyph == 0) {
                if (fallback) {
                    // Missing characters often come in clusters, so build a substring of
                    // codepoints that this font doesn't contain and then defer to the fallback
                    // font.
                    u32string fallback_string = string_of_missing_glyphs(text, index);
                    index += fallback_string.length();
                    x = fallback->pimpl->draw_text(fallback_string, index == text.size(), height,
                                                   bitmap, x, y, c);
                    last_glyph = 0;
                }
                continue;
            }
            
            if (last_glyph) {
                x += stbtt_GetGlyphKernAdvance(&info, last_glyph, glyph) * scale;
            }
            
            // Now finally draw the glyph (if a bitmap was passed).
            if (bitmap) {
                // Do not take lsb into account when positioning the glyph: It seems to correspond
                // to the 'xoff' value returned by stbtt_GetGlyphBitmapSubpixel, and adding both
                // adds too much spacing between letters.
                // Ref: https://github.com/nothings/stb/issues/281#issuecomment-361264014
                draw_glyph(*bitmap, x, y, c, glyph, scale);
            }
            
            int advance;
            stbtt_GetGlyphHMetrics(&info, glyph, &advance, nullptr);
            
            x += advance * scale;
            last_glyph = glyph;
            last_advance = advance;
        }
        
        // If this is the end of the string, we need to take another look at the last glyph to avoid
        // cutting off some pixels that extend to the right of the character.
        if (is_end && last_glyph) {
            int ix = static_cast<int>(x);
            int last_xoff, last_width;
            // TODO: Don't allocate a buffer just to get metrics!
            free(stbtt_GetGlyphBitmapSubpixel(&info, scale, scale, x - ix, 0, last_glyph,
                                              &last_width, nullptr, &last_xoff, nullptr));
            // Move the cursor to the right if pixels have been touched by draw_glyph that are
            // to the right of the current cursor.
            // If the last character extends to the right of the cursor, then this prevents the
            // rightmost pixels from being truncated.
            // If the last character was whitespace, then last_width will be 0 (no pixel data)
            // and the cursor is what counts.
            x = max<double>(x, x - last_advance * scale + last_xoff + last_width);
        }
        
        // Never return a negative value from this method because it is used to determine bitmap
        // dimensions.
        return max<double>(0, x);
    }
    
    u32string string_of_missing_glyphs(const u32string& text, u32string::size_type from_index)
    {
        u32string result;
        
        for (u32string::size_type index = from_index; index < text.size(); ++index) {
            auto codepoint = text[index];
            // Stop as soon as a glyph (except control characters) is available in the current font.
            if (codepoint >= ' ' && stbtt_FindGlyphIndex(&info, codepoint) != 0) break;
            
            result.push_back(codepoint);
        }
        
        return result;
    }
    
    void draw_glyph(Bitmap& bitmap, double fx, double fy, Color c, int glyph, double scale)
    {
        int x = static_cast<int>(fx);
        int y = static_cast<int>(fy);
        int w, h, xoff, yoff;
        
        // As an optimization, this method/class could try to re-use a buffer for rasterization
        // instead of having stb_truetype allocate a fresh one for each draw_glyph call.
        unsigned char* pixels = stbtt_GetGlyphBitmapSubpixel(&info, scale, scale, fx - x, fy - y,
                                                             glyph, &w, &h, &xoff, &yoff);
        
        blend_into_bitmap(bitmap, pixels, x + xoff, y + ascent * scale + yoff, w, h, c);
        
        free(pixels);
    }
    
    void blend_into_bitmap(Bitmap& bitmap, const unsigned char* pixels, int x, int y, int w, int h,
                           Color c)
    {
        int stride = w;
        
        // Instead of transferring all pixels in the range [0; w) x [0; h) into the bitmap, clip
        // these values because Bitmap::blend_pixel does not perform bounds checking.
        
        int src_x = 0;
        if (x < 0) {
            src_x -= x;
            w += x;
            x = 0;
        }
        
        int src_y = 0;
        if (y < 0) {
            src_y -= y;
            h += y;
            y = 0;
        }
        
        w = min<int>(w, bitmap.width() - x);
        h = min<int>(h, bitmap.height() - y);
        
        for (int rel_y = 0; rel_y < h; ++rel_y) {
            for (int rel_x = 0; rel_x < w; ++rel_x) {
                int pixel = pixels[(src_y + rel_y) * stride + src_x + rel_x];
                Color c_with_alpha(pixel * c.alpha() / 255, c.red(), c.green(), c.blue());
                bitmap.blend_pixel(x + rel_x, y + rel_y, c_with_alpha);
            }
        }
    }
};

Gosu::TrueTypeFont::TrueTypeFont(const unsigned char* ttf_data, shared_ptr<TrueTypeFont> fallback)
: pimpl(new Impl(ttf_data, fallback))
{
}

double Gosu::TrueTypeFont::draw_text(const u32string &text, double height,
                                     Bitmap *bitmap, double x, double y, Color c)
{
    return pimpl->draw_text(text, true, height, bitmap, x, y, c);
}

bool Gosu::TrueTypeFont::matches(const unsigned char* ttf_data,
                                 const string& font_name, unsigned font_flags)
{
    // Gosu::FontFlags uses the same values as the STBTT_ macros, except for this one.
    int flags = (font_flags == 0 ? STBTT_MACSTYLE_NONE : font_flags);

    return stbtt_FindMatchingFont(ttf_data, font_name.c_str(), flags) >= 0 ||
           stbtt_FindMatchingFont(ttf_data, font_name.c_str(), STBTT_MACSTYLE_DONTCARE) >= 0;
}

static Gosu::TrueTypeFont& font_with_stack(vector<const unsigned char*> ttf_stack)
{
    // TODO: Make this cache thread-safe.
    static map<const unsigned char*, shared_ptr<Gosu::TrueTypeFont>> cache_by_data;
    
    // Filter out any fonts that could not be found, as well as duplicates.
    auto end = unique(ttf_stack.begin(), ttf_stack.end());
    end = remove(ttf_stack.begin(), end, nullptr);
    ttf_stack.erase(end, ttf_stack.end());
    
    // This cannot happen because ttf_stack contains ttf_fallback_data(), which never returns null.
    if (ttf_stack.empty()) throw logic_error("Empty font stack");
    
    shared_ptr<Gosu::TrueTypeFont> head_of_stack = nullptr;
    for (const unsigned char* ttf_data : ttf_stack) {
        auto& font_ptr = cache_by_data[ttf_data];
        if (!font_ptr) {
            font_ptr = make_shared<Gosu::TrueTypeFont>(ttf_data, head_of_stack);
        }
        head_of_stack = font_ptr;
    }
    return *head_of_stack;
}

Gosu::TrueTypeFont& Gosu::font_by_name(const string& font_name, unsigned font_flags)
{
    // TODO: Make this cache thread-safe.
    static map<pair<string, unsigned>, TrueTypeFont*> cache_by_name_and_flags;
    
    auto& font_ptr = cache_by_name_and_flags[make_pair(font_name, font_flags)];
    if (!font_ptr) {
        // Build a stack from worst-case fallback to the desired font.
        vector<const unsigned char*> ttf_stack;
        ttf_stack.push_back(ttf_fallback_data());
        ttf_stack.push_back(ttf_data_by_name(default_font_name(), 0));
        if (font_flags != 0) ttf_stack.push_back(ttf_data_by_name(default_font_name(), font_flags));
        
        if (font_name.find_first_of("./\\") != string::npos) {
            // A filename? Load it and add it to the stack.
            ttf_stack.push_back(ttf_data_from_file(font_name));
        }
        else if (font_name != default_font_name()) {
            // A font name? Add it to the stack, both with font_flags and without.
            ttf_stack.push_back(ttf_data_by_name(font_name, 0));
            if (font_flags != 0) ttf_stack.push_back(ttf_data_by_name(font_name, font_flags));
        }
        
        font_ptr = &font_with_stack(move(ttf_stack));
    }
    
    return *font_ptr;
}

const unsigned char* Gosu::ttf_data_from_file(const string& filename)
{
    // TODO: Make this cache thread-safe.
    static map<string, shared_ptr<Buffer>> ttf_file_cache;
    
    auto& buffer_ptr = ttf_file_cache[filename];
    if (!buffer_ptr) {
        buffer_ptr = make_shared<Buffer>();
        try {
            load_file(*buffer_ptr, filename);
        }
        catch (...) {
            // Prevent partially loaded files from getting stuck in the cache.
            buffer_ptr = nullptr;
            throw;
        }
    }
    return static_cast<const unsigned char*>(buffer_ptr->data());
}
