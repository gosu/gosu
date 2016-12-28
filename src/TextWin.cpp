#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#define _WIN32_WINNT 0x0500 
#include <windows.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include "WinUtility.hpp"
#include <cstdlib>
#include <cwchar>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <set>

std::wstring Gosu::default_font_name()
{
    return L"Arial";
}

namespace Gosu
{
    std::wstring get_name_from_ttf_file(const std::wstring& filename);

    namespace
    {
        class WinBitmap
        {
            WinBitmap(const WinBitmap&);
            WinBitmap& operator=(const WinBitmap&);
            
            HDC dc;
            HBITMAP bitmap;
            char* pixels;

        public:
            WinBitmap(unsigned width, unsigned height)
            {
                dc = Win::check(::CreateCompatibleDC(0),
                    "creating a device context");

                BITMAPCOREHEADER core_header;
                core_header.bcSize     = sizeof core_header;
                core_header.bcWidth    = width;
                core_header.bcHeight   = height;
                core_header.bcPlanes   = 1;
                core_header.bcBitCount = 24;

                bitmap = ::CreateDIBSection(dc,
                    reinterpret_cast<BITMAPINFO*>(&core_header), DIB_RGB_COLORS,
                    reinterpret_cast<VOID**>(&pixels), 0, 0);
                if (bitmap == 0)
                {
                    ::DeleteDC(dc);
                    Win::throw_last_error("creating a bitmap");
                }

                ::SelectObject(dc, bitmap);

                HBRUSH brush = ::CreateSolidBrush(0x000000);
                RECT rc = { 0, 0, width, height };
                ::FillRect(dc, &rc, brush);
                ::DeleteObject(brush);
            }

            ~WinBitmap()
            {
                ::DeleteObject(bitmap);
                ::SelectObject(dc, ::GetStockObject(SYSTEM_FONT));
                ::DeleteDC(dc);
            }

            HDC context() const
            {
                return dc;
            }

            HBITMAP handle() const
            {
                return bitmap;
            }

            void select_font(std::wstring font_name, unsigned font_height,
                unsigned font_flags) const
            {
                // TODO for ASYNC support:
                // Use a lock on both maps.

                // Note:
                // The caching of opened fonts didn't really show improved text rendering
                // performance on my test system.
                // In case of trouble, it can be taken out without worrying too much.

                static std::map<std::wstring, std::wstring> custom_fonts;
                
                if (font_name.find(L"/") != std::wstring::npos)
                {
                    if (custom_fonts.count(font_name) == 0)
                    {
                        AddFontResourceEx(font_name.c_str(), FR_PRIVATE, 0);
                        font_name = custom_fonts[font_name] = get_name_from_ttf_file(font_name);
                    }
                    else
                        font_name = custom_fonts[font_name];
                }
                
				static std::map<std::pair<std::wstring, unsigned>, HFONT> loaded_fonts;
                
                HFONT font;
				std::pair<std::wstring, unsigned> key =
					std::make_pair(font_name, font_height | font_flags << 16);
                if (loaded_fonts.count(key) == 0)
                {
                    LOGFONT logfont = { font_height, 0, 0, 0,
                        font_flags & FF_BOLD ? FW_BOLD : FW_NORMAL,
                        font_flags & FF_ITALIC ? TRUE : FALSE,
                        font_flags & FF_UNDERLINE ? TRUE : FALSE,
                        FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                        DEFAULT_PITCH | FF_DONTCARE };
                    
                    // Don't rely on wcsncpy being in std::...
                    using namespace std;
                    wcsncpy(logfont.lfFaceName, font_name.c_str(), LF_FACESIZE);
                    logfont.lfFaceName[LF_FACESIZE - 1] = 0;
                    
                    font = loaded_fonts[key] = Win::check(::CreateFontIndirect(&logfont),
                        "creating font object for " + narrow(font_name));
                }
                else
                    font = loaded_fonts[key];

                ::SelectObject(dc, font);
            }
        };
    }
};

unsigned Gosu::text_width(const std::wstring& text,
    const std::wstring& font_name, unsigned font_height, unsigned font_flags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to text_width cannot contain line breaks");
    
    WinBitmap helper(1, 1);
    helper.select_font(font_name, font_height, font_flags);
    
    SIZE size;
    if (!::GetTextExtentPoint32(helper.context(), text.c_str(), text.length(), &size))
        Win::throw_last_error("calculating the width of a text");
    
    return size.cx;
}

void Gosu::draw_text(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& font_name, unsigned font_height,
    unsigned font_flags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to draw_text cannot contain line breaks");
    
    unsigned width = text_width(text, font_name, font_height, font_flags);

    WinBitmap helper(width, font_height);
    helper.select_font(font_name, font_height, font_flags);

    if (::SetTextColor(helper.context(), 0xffffff) == CLR_INVALID)
        Win::throw_last_error("setting the text color");

    Win::check(::SetBkMode(helper.context(), TRANSPARENT),
        "setting a bitmap's background mode to TRANSPARENT");

    ::ExtTextOut(helper.context(), 0, 0, 0, 0, text.c_str(), text.length(), 0);
    
    for (unsigned rel_y = 0; rel_y < font_height; ++rel_y)
        for (unsigned rel_x = 0; rel_x < width; ++rel_x)
        {
            Color pixel = c;
            Color::Channel src_alpha = GetPixel(helper.context(), rel_x, rel_y) & 0xff;
            if (src_alpha == 0)
                continue;
            pixel = multiply(c, Color(src_alpha, 255, 255, 255));
            if (pixel != 0 && x + rel_x >= 0 && x + rel_x < bitmap.width() &&
                y + rel_y >= 0 && y + rel_y < bitmap.height())
                bitmap.set_pixel(x + rel_x, y + rel_y, pixel);
        }
}

#endif
