//#define _WIN32_WINNT 0x0500 
#include <windows.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>

#include "../WinUtility.hpp"
#include "TextSDLTTF.hpp"

#include <cstdlib>
#include <cwchar>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <set>

std::wstring Gosu::defaultFontName()
{
    return L"Arial";
}

namespace Gosu
{
    std::wstring getNameFromTTFFile(const std::wstring& filename);

#if !defined(GOSU_IS_UWP)
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

                BITMAPCOREHEADER coreHeader;
                coreHeader.bcSize     = sizeof coreHeader;
                coreHeader.bcWidth    = width;
                coreHeader.bcHeight   = height;
                coreHeader.bcPlanes   = 1;
                coreHeader.bcBitCount = 24;

                bitmap = ::CreateDIBSection(dc,
                    reinterpret_cast<BITMAPINFO*>(&coreHeader), DIB_RGB_COLORS,
                    reinterpret_cast<VOID**>(&pixels), 0, 0);
                if (bitmap == 0)
                {
                    ::DeleteDC(dc);
                    Win::throwLastError("creating a bitmap");
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

            void selectFont(std::wstring fontName, unsigned fontHeight,
                unsigned fontFlags) const
            {
                // TODO for ASYNC support:
                // Use a lock on both maps.

                // Note:
                // The caching of opened fonts didn't really show improved text rendering
                // performance on my test system.
                // In case of trouble, it can be taken out without worrying too much.

                static std::map<std::wstring, std::wstring> customFonts;
                
                if (fontName.find(L"/") != std::wstring::npos)
                {
                    if (customFonts.count(fontName) == 0)
                    {
                        AddFontResourceEx(fontName.c_str(), FR_PRIVATE, 0);
                        fontName = customFonts[fontName] = getNameFromTTFFile(fontName);
                    }
                    else
                        fontName = customFonts[fontName];
                }
                
				static std::map<std::pair<std::wstring, unsigned>, HFONT> loadedFonts;
                
                HFONT font;
				std::pair<std::wstring, unsigned> key =
					std::make_pair(fontName, fontHeight | fontFlags << 16);
                if (loadedFonts.count(key) == 0)
                {
                    LOGFONT logfont = { fontHeight, 0, 0, 0,
                        fontFlags & ffBold ? FW_BOLD : FW_NORMAL,
                        fontFlags & ffItalic ? TRUE : FALSE,
                        fontFlags & ffUnderline ? TRUE : FALSE,
                        FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                        DEFAULT_PITCH | FF_DONTCARE };
                    
                    // Don't rely on wcsncpy being in std::...
                    using namespace std;
                    wcsncpy(logfont.lfFaceName, fontName.c_str(), LF_FACESIZE);
                    logfont.lfFaceName[LF_FACESIZE - 1] = 0;
                    
                    font = loadedFonts[key] = Win::check(::CreateFontIndirect(&logfont),
                        "creating font object for " + narrow(fontName));
                }
                else
                    font = loadedFonts[key];

                ::SelectObject(dc, font);
            }
        };
    }
#endif

};

#if defined(GOSU_IS_UWP)

unsigned Gosu::textWidth(const std::wstring& text,
	const std::wstring& fontName, unsigned fontHeight,
	unsigned fontFlags)
{
	if (text.find_first_of(L"\r\n") != std::wstring::npos)
		throw std::invalid_argument("the argument to textWidth cannot contain line breaks");

		return SDLTTFRenderer(fontName, fontHeight).textWidth(text);
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
	Color c, const std::wstring& fontName, unsigned fontHeight,
	unsigned fontFlags)
{
	if (text.find_first_of(L"\r\n") != std::wstring::npos)
		throw std::invalid_argument("the argument to drawText cannot contain line breaks");

		SDLTTFRenderer(fontName, fontHeight).drawText(bitmap, text, x, y, c);
}
#else
unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to textWidth cannot contain line breaks");
    
    WinBitmap helper(1, 1);
    helper.selectFont(fontName, fontHeight, fontFlags);
    
    SIZE size;
    if (!::GetTextExtentPoint32(helper.context(), text.c_str(), text.length(), &size))
        Win::throwLastError("calculating the width of a text");
    
    return size.cx;
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to drawText cannot contain line breaks");
    
    unsigned width = textWidth(text, fontName, fontHeight, fontFlags);

    WinBitmap helper(width, fontHeight);
    helper.selectFont(fontName, fontHeight, fontFlags);

    if (::SetTextColor(helper.context(), 0xffffff) == CLR_INVALID)
        Win::throwLastError("setting the text color");

    Win::check(::SetBkMode(helper.context(), TRANSPARENT),
        "setting a bitmap's background mode to TRANSPARENT");

    ::ExtTextOut(helper.context(), 0, 0, 0, 0, text.c_str(), text.length(), 0);
    
    for (unsigned relY = 0; relY < fontHeight; ++relY)
        for (unsigned relX = 0; relX < width; ++relX)
        {
            Color pixel = c;
            Color::Channel srcAlpha = GetPixel(helper.context(), relX, relY) & 0xff;
            if (srcAlpha == 0)
                continue;
            pixel = multiply(c, Color(srcAlpha, 255, 255, 255));
            if (pixel != 0 && x + relX >= 0 && x + relX < bitmap.width() &&
                y + relY >= 0 && y + relY < bitmap.height())
                bitmap.setPixel(x + relX, y + relY, pixel);
        }
}
#endif