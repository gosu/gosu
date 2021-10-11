#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_ANDROID)
// TODO: This is effectively a stub to enable linking without fontconfig available

#include "TrueTypeFont.hpp"
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>

#include <map>
using namespace std;

const unsigned char* Gosu::ttf_data_by_name(const string& font_name, unsigned font_flags)
{
    __android_log_print(android_LogPriority::ANDROID_LOG_ERROR, "Gosu", "Failed to load font: %s, IO is not yet supported!", &font_name);
    // throw runtime_error("Android doesn't support IO yet!");
    return ttf_data_from_file("daniel.ttf");
}

static const unsigned char* ttf_data_of_default_sans_serif_font()
{
    const unsigned char* ttf_ptr = nullptr;

    return ttf_ptr;
}

const unsigned char* Gosu::ttf_fallback_data()
{
    // Prefer Arial Unicode MS as a fallback because it covers a lot of Unicode in a single file.
    // This is also the fallback font on Windows and macOS.
    static const unsigned char* arial_unicode = ttf_data_by_name("Arial Unicode MS", 0);
    if (arial_unicode) return arial_unicode;

    // DejaVu has at least some Unicode coverage (though no CJK).
    static const unsigned char* deja_vu = ttf_data_by_name("DejaVu", 0);
    if (deja_vu) return deja_vu;

    // Unifont has pretty good Unicode coverage, but looks extremely ugly.
    static const unsigned char* unifont = ttf_data_by_name("Unifont", 0);
    if (unifont) return unifont;

    // If none of the fonts above work, try to use the default sans-serif font.
    static const unsigned char* default_font = ttf_data_of_default_sans_serif_font();
    if (default_font) return default_font;

    // If nothing else works, try to load a file from this hardcoded path.
    return ttf_data_from_file("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
}

string Gosu::default_font_name()
{
    // Liberation Sans was designed to have the same metrics as Arial, which is the default
    // font on macOS and Windows.
    return "Liberation Sans";
}

#endif
