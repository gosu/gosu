#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_X)

#include "TrueTypeFont.hpp"

#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>

#include <fontconfig/fontconfig.h>

#include <map>
using namespace std;

const unsigned char* Gosu::ttf_data_by_name(const string& font_name, unsigned font_flags)
{
    // TODO: Make this cache thread-safe.
    static map<pair<string, unsigned>, const unsigned char*> ttf_file_cache;

    auto& ttf_ptr = ttf_file_cache[make_pair(font_name, font_flags)];
    if (ttf_ptr) return ttf_ptr;

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
                }
                else {
                    diff += abs(weight - 80);
                }
                if (font_flags & Gosu::FF_ITALIC) {
                    diff += abs(slant - 100);
                }
                else {
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
                ttf_ptr = ttf_data_from_file(best_filename.c_str());
            }
            
            FcFontSetDestroy(fonts);
        }
        
        FcObjectSetDestroy(props);
        FcPatternDestroy(pattern);
    }

    return ttf_ptr;
}

const unsigned char* Gosu::ttf_fallback_data()
{
    // Prefer Unifont as a fallback because it covers a lot of Unicode.
    static const unsigned char* unifont = ttf_data_by_name("Unifont", 0);
    if (unifont) return unifont;
    
    return ttf_data_from_file("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
}

string Gosu::default_font_name()
{
    return "Liberation";
}

#endif
