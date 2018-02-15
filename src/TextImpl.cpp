#include "TextImpl.hpp"
#include <Gosu/Utility.hpp>
#include <map>

// Disable comma warnings in stb headers.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcomma"
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

using namespace std;

namespace Gosu
{
    class StbTrueTypeFont
    {
        stbtt_fontinfo info;
        // The ascent in internal font metrics (= arbitrary integer scale), that is, the part of the
        // font above the baseline, which TrueType considers to be at y = 0.
        int ascent;
        // Scaling factor from internal font metrics (= arbitrary integer scale) to a font with
        // height = 1px.
        float base_scale;
        
    public:
        StbTrueTypeFont(const unsigned char* ttf_data)
        {
            // This will always use the first font in a font collection; see comment in font().
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
        int draw_text(const string& text, int font_height, Bitmap* bitmap, int x, int y, Color c)
        {
            if (text.empty()) return 0;
            
            float scale = base_scale * font_height;
            
            // utf8_to_wstring is wasteful, we should just iterate directly through the UTF-8 here.
            wstring codepoints = utf8_to_wstring(text);

            // This is the running cursor variable at which text is rendered. It is a float because
            // this code uses stb_truetype's subpixel rendering.
            float fx = x;
            int last_glyph = 0;
            int last_advance = 0;

            for (wchar_t codepoint : codepoints) {
                int glyph = stbtt_FindGlyphIndex(&info, codepoint);
                // Skip characters that the font doesn't contain - this is the place where we could
                // implement font fallback logic.
                // Note that not even the last_glyph variable is being updated in this case.
                if (glyph == 0) continue;

                if (last_glyph) {
                    fx += stbtt_GetGlyphKernAdvance(&info, last_glyph, glyph) * scale;
                }

                int advance;
                stbtt_GetGlyphHMetrics(&info, glyph, &advance, nullptr);
                
                if (bitmap) {
                    // Do not take lsb into account here: it seems to correspond to the 'xoff' value
                    // returned by stbtt_GetGlyphBitmapSubpixel, and adding both adds too much
                    // spacing between letters.
                    // Ref: https://github.com/nothings/stb/issues/281#issuecomment-361264014
                    draw_glyph(*bitmap, fx, y, c, glyph, scale);
                }
                
                fx += advance * scale;
                last_glyph = glyph;
                last_advance = advance;
            }
            
            // The last rendered glyph affects how we calculate the resulting text width.
            if (last_glyph) {
                int x = static_cast<int>(fx);
                int last_xoff, last_width;
                stbtt_GetGlyphBitmapSubpixel(&info, scale, scale, fx - x, 0, last_glyph,
                                             &last_width, nullptr, &last_xoff, nullptr);
                // Move the cursor to the right if pixels have been touched by draw_glyph that are
                // to the right of the current cursor.
                // If the last character extends to the right of the cursor, then this prevents the
                // rightmost pixels from being truncated.
                // If the last character was whitespace, then last_width will be 0 (no pixel data)
                // and the cursor is what counts.
                fx = max<float>(fx, fx - last_advance * scale + last_xoff + last_width);
            }

            // Never return a negative value from this method because it is used to determine bitmap
            // dimensions.
            return max<int>(0, ceil(fx));
        }
        
        void draw_glyph(Bitmap& bitmap, double fx, int y, Gosu::Color c, int glyph, double scale)
        {
            int x = static_cast<int>(fx);
            int w, h, xoff, yoff;

            // As an optimization, this method/class could try to re-use a buffer for rasterization
            // instead of having stb_truetype allocate a fresh one for each draw_glyph call.
            unsigned char* pixels = stbtt_GetGlyphBitmapSubpixel(&info, scale, scale, fx - x, 0,
                                                                 glyph, &w, &h, &xoff, &yoff);
            
            blend_into_bitmap(bitmap, pixels, x + xoff, y + ascent * scale + yoff, w, h, c);

            free(pixels);
        }
        
        // This implements the "over" alpha compositing operator, see:
        // https://en.wikipedia.org/wiki/Alpha_compositing
        void blend_into_bitmap(Bitmap& bitmap, unsigned char* pixels, int x, int y, int w, int h,
                               Gosu::Color c)
        {
            int stride = w;

            // Instead of transferring all pixels in the range [0; w) x [0; h) into the bitmap, clip
            // these values because Bitmap::set_pixel does not perform bounds checking.

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
                    unsigned char pixel = pixels[(src_y + rel_y) * stride + (src_x + rel_x)];
                    unsigned char inv_pixel = 255 - pixel;
                    
                    Color color = bitmap.get_pixel(x + rel_x, y + rel_y);
                    color.set_alpha((pixel * c.alpha() + inv_pixel * color.alpha()) / 255);
                    color.set_red  ((pixel * c.red()   + inv_pixel * color.red())   / 255);
                    color.set_green((pixel * c.green() + inv_pixel * color.green()) / 255);
                    color.set_blue ((pixel * c.blue()  + inv_pixel * color.blue())  / 255);

                    bitmap.set_pixel(x + rel_x, y + rel_y, color);
                }
            }
        }
    };
    
    StbTrueTypeFont& font_for_data(const void* ttf_data)
    {
        auto data = static_cast<const unsigned char*>(ttf_data);
        
        // Note: This cache is not yet thread-safe.
        static map<const unsigned char*, StbTrueTypeFont> fonts;
        
        auto iterator = fonts.find(data);
        if (iterator == fonts.end()) {
            iterator = fonts.emplace(data, data).first;
        }
        return iterator->second;
    }
}

int Gosu::text_width_ttf(const void* ttf_data, int font_height,
                         const string& text)
{
    return font_for_data(ttf_data).draw_text(text, font_height, nullptr, 0, 0, Color());
}

int Gosu::draw_text_ttf(const void* ttf_data, int font_height,
                        const string& text, Bitmap& bitmap, int x, int y, Color c)
{
    return font_for_data(ttf_data).draw_text(text, font_height, &bitmap, x, y, c);
}

bool Gosu::verify_font_name(const void* ttf_data, const std::string &font_name)
{
    auto data = static_cast<const unsigned char*>(ttf_data);
    return stbtt_FindMatchingFont(data, (font_name).c_str(), STBTT_MACSTYLE_NONE) >= 0 ||
        stbtt_FindMatchingFont(data, (font_name).c_str(), STBTT_MACSTYLE_DONTCARE) >= 0;
}
