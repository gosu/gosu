#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_X)

#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include "TrueTypeFont.hpp"
#include <fontconfig/fontconfig.h>
#include <map>
#include <mutex>

const std::uint8_t* Gosu::ttf_data_by_name(const std::string& font_name, unsigned font_flags)
{
    static std::map<std::pair<std::string, unsigned>, const std::uint8_t*> ttf_file_cache;
    static std::recursive_mutex mutex;
    std::scoped_lock lock(mutex);

    auto& ttf_ptr = ttf_file_cache[make_pair(font_name, font_flags)];
    if (ttf_ptr) {
        return ttf_ptr;
    }

    static FcConfig* config = FcInitLoadConfigAndFonts();
    if (config) {
        FcPattern* pattern;
        // Our search pattern does not include weight or slant so that we can compromise on these.
        pattern = FcPatternBuild(nullptr, FC_FAMILY, FcTypeString,
                                 font_name.empty() ? "sans-serif" : font_name.c_str(), FC_OUTLINE,
                                 FcTypeBool, FcTrue, // exclude bitmap fonts
                                 nullptr);

        FcObjectSet* props = FcObjectSetBuild(FC_FILE, FC_WEIGHT, FC_SLANT, nullptr);

        if (FcFontSet* fonts = FcFontList(config, pattern, props)) {
            // Among all matching fonts, find the variant that is the best fit for our font_flags.
            std::string best_filename;
            int best_diff;

            for (int i = 0; i < fonts->nfont; ++i) {
                int weight, slant;

                FcPatternGetInteger(fonts->fonts[i], FC_WEIGHT, 0, &weight);
                FcPatternGetInteger(fonts->fonts[i], FC_SLANT, 0, &slant);

                // Difference between found font weight/slant and desired weight/slant.
                // Lower is better, so find the font with the lowest diff.
                int diff = 0;
                if (font_flags & Gosu::FF_BOLD) {
                    diff += abs(weight - FC_WEIGHT_BOLD);
                }
                else {
                    diff += abs(weight - FC_WEIGHT_REGULAR);
                }
                if (font_flags & Gosu::FF_ITALIC) {
                    diff += abs(slant - FC_SLANT_ITALIC);
                }
                else {
                    diff += abs(slant - FC_SLANT_ROMAN);
                }

                // Penalize OTF fonts since we are not really good at handling these.
                FcChar8* file;
                FcPatternGetString(fonts->fonts[i], FC_FILE, 0, &file);
                if (has_extension(reinterpret_cast<char*>(file), ".otf")) {
                    diff += 10000;
                }

                if (best_filename.empty() || diff < best_diff) {
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

static const std::uint8_t* ttf_data_of_default_sans_serif_font()
{
    const std::uint8_t* ttf_ptr = nullptr;

    // At this point, we already have an open FcConfig, and can pass nullptr to these functions.
    FcPattern* pattern = FcNameParse(reinterpret_cast<const FcChar8*>("sans-serif"));
    FcConfigSubstitute(nullptr, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);
    FcResult match_result;
    pattern = FcFontMatch(nullptr, pattern, &match_result);
    if (match_result == FcResultMatch) {
        FcChar8* filename;
        if (FcPatternGetString(pattern, FC_FILE, 0, &filename) == FcResultMatch) {
            ttf_ptr = Gosu::ttf_data_from_file(reinterpret_cast<char*>(filename));
        }
    }
    FcPatternDestroy(pattern);

    return ttf_ptr;
}

const std::uint8_t* Gosu::ttf_fallback_data()
{
    // Prefer Arial Unicode MS as a fallback because it covers a lot of Unicode in a single file.
    // This is also the fallback font on Windows and macOS.
    static const std::uint8_t* arial_unicode = ttf_data_by_name("Arial Unicode MS", 0);
    if (arial_unicode) {
        return arial_unicode;
    }

    // DejaVu has at least some Unicode coverage (though no CJK).
    static const std::uint8_t* deja_vu = ttf_data_by_name("DejaVu", 0);
    if (deja_vu) {
        return deja_vu;
    }

    // Unifont has pretty good Unicode coverage, but looks extremely ugly.
    static const std::uint8_t* unifont = ttf_data_by_name("Unifont", 0);
    if (unifont) {
        return unifont;
    }

    // If none of the fonts above work, try to use the default sans-serif font.
    static const std::uint8_t* default_font = ttf_data_of_default_sans_serif_font();
    if (default_font) {
        return default_font;
    }

    // If nothing else works, try to load a file from this hardcoded path.
    return ttf_data_from_file("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
}

std::string Gosu::default_font_name()
{
    // Liberation Sans was designed to have the same metrics as Arial, which is the default
    // font on macOS and Windows.
    return "Liberation Sans";
}

#endif
