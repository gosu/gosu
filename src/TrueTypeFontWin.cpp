#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include "TrueTypeFont.hpp"
#include "Log.hpp"

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

    log("Trying to find a font named '%s', flags=%x", font_name.c_str(), font_flags);

    LOGFONT logfont = {
        0, 0, 0, 0,
        (font_flags & Gosu::FF_BOLD) ? FW_BOLD : FW_NORMAL,
        static_cast<BYTE>((font_flags & Gosu::FF_ITALIC) ? TRUE : FALSE),
        static_cast<BYTE>((font_flags & Gosu::FF_UNDERLINE) ? TRUE : FALSE),
        0 /* no strikethrough */,
        ANSI_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
        DEFAULT_PITCH
    };
    
    wstring wfont_name = utf8_to_utf16(font_name);
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
                    if (font_name.empty() || TrueTypeFont::matches(data, font_name, font_flags)) {
                        buffer_ptr = buffer;
                        log("Found a matching file (%d bytes)", (int) buffer->size());
                    }
                    else {
                        log("Internal font name did not match; discarding result");
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
    
    // Otherwise, just try to find *any* font.
    static const unsigned char* any_font = ttf_data_by_name("", 0);
    if (any_font) return any_font;
    
    static const char* windir = getenv("WINDIR");
    if (windir) return ttf_data_from_file(string(windir) + "/Fonts/arial.ttf");
    
    return ttf_data_from_file("C:/Windows/Fonts/arial.ttf");
}

string Gosu::default_font_name()
{
    return "Arial";
}

#endif
