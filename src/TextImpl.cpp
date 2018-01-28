#include "TextImpl.hpp"
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <map>
#include <memory>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

using namespace std;

namespace Gosu
{
    class StbTrueTypeFont
    {
        // Use a unique_ptr to prevent the Buffer from being copied or moved, which would invalidate
        // the info.data pointer.
        unique_ptr<Buffer> buffer;
        stbtt_fontinfo info;
        // The ascent in internal font metrics (= arbitrary integer scale), that is, the part of the
        // font above the baseline, which TrueType consider to be at y = 0.
        int ascent;
        // Scaling factor from internal font metrics (= arbitrary integer scale) to a font with
        // height = 1px.
        double base_scale;
        
    public:
        StbTrueTypeFont(const string& filename)
        : buffer(new Buffer())
        {
            load_file(*buffer, filename);
            
            // This will always use the first font in a font collection; see comment in font().
            auto* data = static_cast<const unsigned char*>(buffer->data());
            auto offset = stbtt_GetFontOffsetForIndex(data, 0);
            int success = stbtt_InitFont(&info, data, offset);
            if (!success) throw runtime_error("Cannot load TTF/OTF font: " + filename);
            
            // Calculate metrics.
            int descent, lineGap;
            stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
            int height = ascent - descent + lineGap;
            base_scale = 1.0 / height;
            // Not sure if we also need to call stbtt_GetFontVMetricsOS2 and somehow merge results?
        }
        
        // This method always measures text, and also draws it if (bitmap != nullptr).
        int draw_text(const string& text, int font_height, Bitmap* bitmap, int x, int y, Color c)
        {
            if (text.empty()) return 0;
            
            auto scale = base_scale * font_height;
            
            // TODO: utf8_to_wstring is wasteful, just iterate directly through the UTF-8 here.
            wstring codepoints = utf8_to_wstring(text);
            
            // lsb (the "left side bearing") will usually be 0, but it could be a negative value if
            // the first letter extends to the left of its bounding box.
            // ref: https://github.com/nothings/stb/issues/281#issuecomment-209403157
            int first_glyph_lsb;
            stbtt_GetCodepointHMetrics(&info, codepoints.front(), nullptr, &first_glyph_lsb);
            
            int width = -first_glyph_lsb;
            int last_glyph = 0;

            for (wchar_t codepoint : codepoints) {
                int glyph = stbtt_FindGlyphIndex(&info, codepoint);
                if (glyph == 0) continue;
                
                int advance, lsb;
                stbtt_GetGlyphHMetrics(&info, glyph, &advance, &lsb);
                
                if (bitmap) {
                    draw_glyph(*bitmap, x + (width + lsb) * scale, y, glyph, scale);
                }
                
                width += advance;
                if (last_glyph) {
                    width += stbtt_GetGlyphKernAdvance(&info, last_glyph, glyph);
                }
                last_glyph = glyph;
            }
            return max<int>(0, ceil(width * scale));
        }
        
        void draw_glyph(Bitmap& bitmap, double x, int y, int glyph, double scale)
        {
            int ix = static_cast<int>(x);
            int w, h, xoff, yoff;

            // As an optimization, this method/class could try to re-use a buffer for rasterization
            // instead of having stb_truetype allocate a fresh one for each draw_glyph call.
            unsigned char* pixels = stbtt_GetGlyphBitmapSubpixel(&info, scale, scale, x - ix, 0,
                                                                 glyph, &w, &h, &xoff, &yoff);
            
            blend_into_bitmap(bitmap, pixels, ix + xoff, y + ascent * scale + yoff, w, h);

            free(pixels);
        }
        
        // This implements the "over" alpha compositing operator, see:
        // https://en.wikipedia.org/wiki/Alpha_compositing
        void blend_into_bitmap(Bitmap& bitmap, unsigned char* pixels, int x, int y, int w, int h)
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
                    color.set_alpha(min<int>(255, (pixel * 255 + color.alpha() * inv_pixel) / 255));
                    color.set_red  (min<int>(255, (pixel * 255 + color.red()   * inv_pixel) / 255));
                    color.set_green(min<int>(255, (pixel * 255 + color.green() * inv_pixel) / 255));
                    color.set_blue (min<int>(255, (pixel * 255 + color.blue()  * inv_pixel) / 255));

                    bitmap.set_pixel(x + rel_x, y + rel_y, color);
                }
            }
        }
    };
    
    map<pair<string, unsigned>, StbTrueTypeFont> fonts;
    
    StbTrueTypeFont& font(const string& filename, unsigned font_flags)
    {
        if (font_flags >= FF_COMBINATIONS) {
            throw invalid_argument("Invalid font_flags: " + to_string(font_flags));
        }
        
        // TODO: The code in this file does not support B/I/U formatting for custom TTF fonts.
        // Options for the future:
        // 1. Use stbtt_FindMatchingFont. This will only work for TTC font collections, and we will
        //    have to patch stb_truetype to look for fonts only based on `int flags`, while ignoring
        //    the name of fonts inside a bundle (who wants to deal with strings, anyway).
        // 2. Maybe Gosu should accept filename patterns like "LibreBaskerville-*.ttf" as the font
        //    name and then replace the * with "Regular", "Bold", "Italic" etc.?
        // 3. As a last resort, Gosu could implement faux bold and faux italics. I think faux
        //    underlines are a must anyway, since no font provides a dedicated TTF file for that.
        font_flags = 0;
        
        // This appears to be the best way to implement lazy-loading using std::map/C++11.
        auto key = make_pair(filename, font_flags);
        auto iter = fonts.lower_bound(key);
        if (iter->first != key) {
            iter = fonts.emplace_hint(iter, key, StbTrueTypeFont(filename));
        }
        return iter->second;
    }
}

int Gosu::text_width_ttf(const string& text,
                         const string& filename, int font_height, unsigned font_flags)
{
    return font(filename, font_flags).draw_text(text, font_height, nullptr, 0, 0, Color());
}

void Gosu::draw_text_ttf(Bitmap& bitmap, const string& text, int x, int y, Color c,
                         const string& filename, int font_height, unsigned font_flags)
{
    font(filename, font_flags).draw_text(text, font_height, &bitmap, x, y, c);
}
