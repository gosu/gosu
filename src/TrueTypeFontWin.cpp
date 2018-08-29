#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include "TrueTypeFont.hpp"

#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "WinUtility.hpp"

#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>

#include <cstdlib>
#include <cwchar>
#include <map>
#include <memory>
using namespace std;

const unsigned char* Gosu::ttf_data_by_name(const string& font_name, unsigned font_flags)
{
    // TODO: Make this cache thread-safe.
    static map<pair<string, unsigned>, shared_ptr<Buffer>> ttf_file_cache;
    
    auto& buffer_ptr = ttf_file_cache[make_pair(font_name, font_flags)];
    if (buffer_ptr) return static_cast<const unsigned char*>(buffer_ptr->data());

    LOGFONT logfont = {
        20 /* arbitrary font height */, 0, 0, 0,
        (font_flags & Gosu::FF_BOLD) ? FW_BOLD : FW_NORMAL,
        (font_flags & Gosu::FF_ITALIC) ? TRUE : FALSE,
        (font_flags & Gosu::FF_UNDERLINE) ? TRUE : FALSE,
        FALSE /* no strikethrough */,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE
    };
    
    wstring wfont_name = utf8_to_wstring(font_name);
    wcsncpy(logfont.lfFaceName, wfont_name.c_str(), LF_FACESIZE);
    logfont.lfFaceName[LF_FACESIZE - 1] = 0;
    
    if (HFONT font = CreateFontIndirect(&logfont)) {
        if (HDC hdc = GetDC(0)) {
            HFONT last_font = (HFONT) SelectObject(hdc, (HGDIOBJ) font);
            auto ttf_buffer_size = GetFontData(hdc, 0, 0, nullptr, 0);
            if (ttf_buffer_size != GDI_ERROR) {
                auto buffer = make_shared<Gosu::Buffer>();
                buffer->resize(ttf_buffer_size);
                if (GetFontData(hdc, 0, 0, buffer->data(), buffer->size()) != GDI_ERROR) {
                    auto data = static_cast<const unsigned char*>(buffer->data());
                    if (TrueTypeFont::verify_font_name(data, font_name)) {
                        buffer_ptr = buffer;
                    }
                }
            }
            SelectObject(hdc, (HGDIOBJ) last_font);
            ReleaseDC(0, hdc);
        }
        DeleteObject((HGDIOBJ) font);
    }

    return buffer_ptr ? static_cast<const unsigned char*>(buffer_ptr->data()) : nullptr;
}

const unsigned char* Gosu::ttf_fallback_data()
{
    // Prefer Arial Unicode MS as a fallback because it covers a lot of Unicode.
    static const unsigned char* arial_unicode = ttf_data_by_name("Arial Unicode MS", 0);
    if (arial_unicode) return arial_unicode;
    
    static const char* windir = getenv("WINDIR");
    if (windir) ttf_data_from_file(string(windir) + "/Fonts/arial.ttf");
    return ttf_data_from_file("C:/Windows/Fonts/arial.ttf");
}

string Gosu::default_font_name()
{
    return "Arial";
}

#endif
