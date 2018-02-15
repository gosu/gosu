#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_X)

#include <Gosu/Text.hpp>
#include <Gosu/IO.hpp>

#include "TextImpl.hpp"

#include <fontconfig/fontconfig.h>

#include <cmath>
#include <map>
#include <stdexcept>
#include <string>

using namespace std;


static shared_ptr<Gosu::Buffer> find_ttf_data(string font_name, unsigned font_flags)
{
    static map<pair<string, unsigned>, shared_ptr<Gosu::Buffer>> ttf_buffers;

    auto key = make_pair(font_name, font_flags);
    
    auto iterator = ttf_buffers.find(key);
    if (iterator != ttf_buffers.end()) {
        return iterator->second;
    }
    
    // A filename? Just load it.
    if (font_name.find_first_of("./\\") != string::npos) {
        auto buffer = make_shared<Gosu::Buffer>();
        Gosu::load_file(*buffer, font_name);
        // Assign this buffer to all combinations of the given filename, since Gosu::FontFlags are
        // not yet supported for custom TTF files - this avoids loading the same file twice.
        for (unsigned flags = 0; flags < Gosu::FF_COMBINATIONS; ++flags) {
            ttf_buffers[make_pair(font_name, flags)] = buffer;
        }
        return buffer;
    }
    
    static FcConfig* config = FcInitLoadConfigAndFonts();
    if (config) {
        // Our search pattern does not include weight or slant so that we can compromise on these.
        FcPattern* pattern = FcPatternBuild(nullptr,
                                            FC_FAMILY,  FcTypeString,  font_name.c_str(),
                                            FC_OUTLINE, FcTypeBool,    FcTrue, /* no bitmap fonts */
                                            nullptr);
        FcObjectSet* props = FcObjectSetBuild(FC_FILE, FC_WEIGHT, FC_SLANT, nullptr);

        if (FcFontSet* fonts = FcFontList(config, pattern, props)) {
            // Among all matching fonts, find the variant that is the best fit for our font_flags.
            string best_filename;
            int best_diff;
            
            for (int i = 0; i < fonts->nfont; ++i) {
                int weight, slant;
                
                FcPatternGetInteger(fonts->fonts[i], FC_WEIGHT, 0, &weight);
                FcPatternGetInteger(fonts->fonts[i], FC_SLANT, 0, &slant);
                
                // Difference between found font weight/slant and desired weight/slant.
                // Lower is better, so find the font with the lowest diff.
                int diff = 0;
                if (font_flags & Gosu::FF_BOLD) {
                    diff += abs(weight - 200);
                } else {
                    diff += abs(weight - 80);
                }
                if (font_flags & Gosu::FF_ITALIC) {
                    diff += abs(slant - 100);
                } else {
                    diff += abs(slant - 0);
                }
                
                if (best_filename.empty() || diff < best_diff) {
                    FcChar8 *file;
                    FcPatternGetString(fonts->fonts[i], FC_FILE, 0, &file);
                    best_filename = reinterpret_cast<char*>(file);
                    best_diff = diff;
                }
            }
            if (!best_filename.empty()) {
                ttf_buffers[key] = find_ttf_data(best_filename.c_str(), font_flags);
            }
            
            FcFontSetDestroy(fonts);
        }
        
        FcObjectSetDestroy(props);
        FcPatternDestroy(pattern);
    }

    if (!ttf_buffers[key] && font_flags != 0) {
        ttf_buffers[key] = find_ttf_data(font_name, 0);
    }
    if (!ttf_buffers[key] && font_name != Gosu::default_font_name()) {
        ttf_buffers[key] = find_ttf_data(Gosu::default_font_name(), font_flags);
    }
    if (!ttf_buffers[key]) {
        ttf_buffers[key] = find_ttf_data("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 0);
    }
    
    return ttf_buffers[key];
}

string Gosu::default_font_name()
{
    return "Liberation";
}

int Gosu::text_width(const string& text, const string& font_name,
                     int font_height, unsigned font_flags)
{
    if (font_flags >= FF_COMBINATIONS) {
        throw invalid_argument("Invalid font_flags: " + to_string(font_flags));
    }
    
    if (text.find_first_of("\r\n") != string::npos) {
        throw invalid_argument("text_width cannot handle line breaks");
    }

    Gosu::Buffer& ttf_data = *find_ttf_data(font_name, font_flags);
    return text_width_ttf(static_cast<unsigned char*>(ttf_data.data()), font_height,
                          text);
}

int Gosu::draw_text(Bitmap& bitmap, const string& text, int x, int y, Color c,
                    const string& font_name, int font_height, unsigned font_flags)
{
    if (font_flags >= FF_COMBINATIONS) {
        throw invalid_argument("Invalid font_flags: " + to_string(font_flags));
    }
    
    if (text.find_first_of("\r\n") != text.npos) {
        throw invalid_argument("the argument to draw_text cannot contain line breaks");
    }
    
    Gosu::Buffer& ttf_data = *find_ttf_data(font_name, font_flags);
    return draw_text_ttf(static_cast<unsigned char*>(ttf_data.data()), font_height,
                         text, bitmap, x, y, c);
}

#endif
