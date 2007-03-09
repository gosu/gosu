#include <Gosu/Bitmap.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/WinUtility.hpp>
#include <boost/utility.hpp>
#include <cstdlib>
#include <cwchar>
#include <algorithm>
#include <stdexcept>

std::wstring Gosu::defaultFontName()
{
    return L"Arial";
}

// IMPR: throwLastError/check is used a lot here, Win9x doesn't provide GDI
// error codes though. Wait until 9x is dead or fix this!

namespace Gosu
{
    namespace
    {
        class WinBitmap : boost::noncopyable
        {
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
                ::DeleteObject(::SelectObject(dc, ::GetStockObject(SYSTEM_FONT)));
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

            /*char* data() const
            {
                return pixels;
            }*/

            void selectFont(const std::wstring& fontName, unsigned fontHeight,
                unsigned fontFlags) const
            {
                // IMPR: Maybe remember the last font we had and don't
                // recreate it every time?

                LOGFONT logfont = { fontHeight, 0, 0, 0,
                    fontFlags & ffBold ? FW_BOLD : FW_NORMAL,
                    fontFlags & ffItalic ? TRUE : FALSE,
                    fontFlags & ffUnderline ? TRUE : FALSE,
                    FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                    CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,
                    DEFAULT_PITCH | FF_DONTCARE };

                // IMPR: This truncates. Is this reasonable? Should we rather
                // throw something?
                /*std::*/wcsncpy(logfont.lfFaceName, fontName.c_str(), LF_FACESIZE);
                logfont.lfFaceName[LF_FACESIZE - 1] = 0;

                HFONT newFont = Win::check(::CreateFontIndirect(&logfont),
                    "creating font object for " + narrow(fontName));

                HFONT oldFont = reinterpret_cast<HFONT>(::SelectObject(dc, newFont));
                Win::check(oldFont, "selecting the font object for " + narrow(fontName));
                ::DeleteObject(oldFont);
            }
        };
    }
};

unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
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
    unsigned width = textWidth(text, fontName, fontHeight, fontFlags);

    WinBitmap helper(width, fontHeight);
    helper.selectFont(fontName, fontHeight, fontFlags);

    if (::SetTextColor(helper.context(), 0xffffff) == CLR_INVALID)
        Win::throwLastError("setting the text color");

    Win::check(::SetBkMode(helper.context(), TRANSPARENT),
        "setting a bitmap's background mode to TRANSPARENT");

    ::TextOut(helper.context(), 0, 0, text.c_str(), text.length());

    for (unsigned relY = 0; relY < fontHeight; ++relY)
        for (unsigned relX = 0; relX < width; ++relX)
        {
            COLORREF winPixel = ::GetPixel(helper.context(), relX, relY);
            // IMPR: Make better use of c.alpha()?
            // Maybe even add an AlphaMode parameter?
            if (winPixel != 0 && x + relX >= 0 && x + relX < bitmap.width() &&
                y + relY >= 0 && y + relY < bitmap.height())
                bitmap.setPixel(x + relX, y + relY, c);
        }
}
