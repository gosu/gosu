#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#define _WIN32_WINNT 0x0500
#include <windows.h>

#include "WinUtility.hpp"

#include "TextImpl.hpp"

#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>

#include <cstdlib>
#include <cwchar>
#include <map>
#include <memory>
#include <stdexcept>

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
    
    LOGFONT logfont = {
        20 /* arbitrary font height */, 0, 0, 0,
        (font_flags & Gosu::FF_BOLD) ? FW_BOLD : FW_NORMAL,
        (font_flags & Gosu::FF_ITALIC) ? TRUE : FALSE,
        (font_flags & Gosu::FF_UNDERLINE) ? TRUE : FALSE,
        FALSE /* no strikethrough */,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE
    };
    
    wstring wfont_name = Gosu::utf8_to_wstring(font_name);
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
                    if (Gosu::verify_font_name(buffer->data(), font_name)) {
                        ttf_buffers[key] = buffer;
                    }
                }
            }
            SelectObject(hdc, (HGDIOBJ) last_font);
            ReleaseDC(0, hdc);
        }
        DeleteObject((HGDIOBJ) font);
    }

    if (!ttf_buffers[key] && font_flags != 0) {
        ttf_buffers[key] = find_ttf_data(font_name, 0);
    }
    if (!ttf_buffers[key] && font_name != Gosu::default_font_name()) {
        ttf_buffers[key] = find_ttf_data(Gosu::default_font_name(), font_flags);
    }
    if (!ttf_buffers[key]) {
        if (const char* windir = getenv("WINDIR")) {
            ttf_buffers[key] = find_ttf_data(string(windir) + "/Fonts/arial.ttf", 0);
        } else {
            ttf_buffers[key] = find_ttf_data("C:/Windows/Fonts/arial.ttf", 0);
        }
    }
    
    return ttf_buffers[key];
}

string Gosu::default_font_name()
{
    return "Arial";
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
