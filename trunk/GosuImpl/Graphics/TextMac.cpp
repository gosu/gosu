#include <Gosu/Bitmap.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/IO.hpp>
#include <boost/utility.hpp>
#include <boost/cstdint.hpp>
#include <cmath>
#include <stdexcept>
#include <map>
#include <vector>
#include <ApplicationServices/ApplicationServices.h>

std::wstring Gosu::defaultFontName()
{
    // Could choose Arial to stay consistent with Windows, but since the font sizes will
    // be messed up anyway...
    return L"Helvetica";
}

namespace Gosu
{
    std::vector<unsigned short> wstringToUniChars(const std::wstring& ws);
}

#include <iostream>
#include <ostream>
#include <sstream>
void throwError(OSStatus status, unsigned line)
{
    std::ostringstream str;
    str << "Error on line " << line << " (Code " << status << "): "
        << GetMacOSStatusErrorString(status)
        << " (" << GetMacOSStatusCommentString(status) << ")";
    throw std::runtime_error(str.str());
}

#define checkErr(status) if (!(status)) {} else throwError(status, __LINE__)

namespace
{
    class MacBitmap : boost::noncopyable
    {
        boost::uint32_t* buf;
        unsigned width, height;
        CGContextRef ctx;

    public:
        MacBitmap(boost::uint32_t* buf, unsigned width, unsigned height)
        : buf(buf), width(width), height(height)
        {
            CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceUserRGB); 
            ctx = CGBitmapContextCreate (buf, width, height, 8, width * 4, colorSpace,
                kCGImageAlphaPremultipliedLast);

            CGColorSpaceRelease( colorSpace ); 
        }

        ~MacBitmap()
        {
            CGContextRelease(ctx);
        }

        CGContextRef context() const
        {
            return ctx;
        }
    };
    
    struct CachedFontInfo
    {
        ATSUFontID fontId;
        double heightAt1Pt;
        double descentAt1Pt;
    };
    CachedFontInfo& getFont(const std::wstring& fontName)
    {
        static std::map<std::wstring, CachedFontInfo> fonts;
        
        if (fonts.count(fontName))
            return fonts[fontName];
        
        // Get reference to font, loaded from the system or a file.
        ATSFontRef atsRef;
        if (fontName.find(L"/") == std::wstring::npos)
        {
            // System font
            CFStringRef cfName = CFStringCreateWithCString(NULL, Gosu::narrow(fontName).c_str(), kCFStringEncodingASCII);
            atsRef = ATSFontFindFromName(cfName, kATSOptionFlagsDefault);
            if (!atsRef)
                throw std::runtime_error("Cannot find font " + Gosu::narrow(fontName));
            CFRelease(cfName);
        }
        else
        {
            // Filename to font
            Gosu::Buffer buf;
            Gosu::loadFile(buf, fontName);
            
            ATSFontContainerRef container; 
            checkErr( ATSFontActivateFromMemory(buf.data(), buf.size(),
                          kATSFontContextLocal, kATSFontFormatUnspecified, 
                          NULL, kATSOptionFlagsDefault, &container) );
            
            ATSFontRef fontRefs[1024]; 
            ItemCount  fontCount; 
            checkErr( ATSFontFindFromContainer(container, kATSOptionFlagsDefault, 
                          1024, fontRefs, &fontCount) );
            if (fontCount == 0)
                throw std::runtime_error("No font found in " + Gosu::narrow(fontName));

            atsRef = fontRefs[0];
        }
        
        // Calculate metrics (for space allocations) and create CachedFontInfo entry.
        CachedFontInfo newFont;
        newFont.fontId = FMGetFontFromATSFontRef(atsRef);

        ATSFontMetrics metrics;
        checkErr(ATSFontGetHorizontalMetrics(newFont.fontId, kATSOptionFlagsDefault, &metrics));
        newFont.heightAt1Pt = metrics.ascent - metrics.descent;
        newFont.descentAt1Pt = -metrics.descent;
        fonts[fontName] = newFont;
        return fonts[fontName];
    }
    
    class ATSULayoutAndStyle
    {
        ATSUStyle style;
        ATSUTextLayout layout;
        std::vector<UniChar> utf16; // More like UCS-2-INTERNAL.

        template<typename T>
        void setAttribute(ATSUAttributeTag tag, T value)
        {
            ByteCount size = sizeof value;
            ATSUAttributeValuePtr ptr = &value;
            checkErr( ATSUSetAttributes(style, 1, &tag, &size, &ptr) );
        }
        
        template<typename T>
        void setLayoutControl(ATSUAttributeTag tag, T value)
        {
            ByteCount size = sizeof value;
            ATSUAttributeValuePtr ptr = &value;
            checkErr( ATSUSetLayoutControls(layout, 1, &tag, &size, &ptr) );
        }
        
    public:
        ATSULayoutAndStyle(const std::wstring& text, const std::wstring& fontName, unsigned fontHeightPx, unsigned fontFlags)
        {
            utf16 = Gosu::wstringToUniChars(text);
        
            checkErr( ATSUCreateStyle(&style) );
            
            CachedFontInfo& font = getFont(fontName);
            
            setAttribute<ATSUFontID>(kATSUFontTag, font.fontId);
            
            setAttribute<Fixed>(kATSUSizeTag, X2Fix(fontHeightPx / font.heightAt1Pt));
            if (fontFlags & Gosu::ffBold)
                setAttribute<Boolean>(kATSUQDBoldfaceTag, TRUE);
            if (fontFlags & Gosu::ffItalic)
                setAttribute<Boolean>(kATSUQDItalicTag, TRUE);
            if (fontFlags & Gosu::ffUnderline)
                setAttribute<Boolean>(kATSUQDUnderlineTag, TRUE);

            UniCharCount runLength = utf16.size();
            checkErr( ATSUCreateTextLayoutWithTextPtr(&utf16[0], kATSUFromTextBeginning,
                            kATSUToTextEnd, utf16.size(), 1, &runLength, &style, &layout) );
        }
        
        ~ATSULayoutAndStyle()
        {
            checkErr( ATSUDisposeStyle(style) );
            checkErr( ATSUDisposeTextLayout(layout) );
        }

        Rect textExtents() const
        {
            Rect rect;
            checkErr( ATSUSetTransientFontMatching(layout, TRUE) );
            checkErr( ATSUMeasureTextImage(layout, kATSUFromTextBeginning,
                                           kATSUToTextEnd, X2Fix(0), X2Fix(0), &rect) );
            return rect;
        }
        
        void drawToContext(Fixed x, Fixed y, CGContextRef context)
        {
            // Always draw in black - recoloring to white happens in drawText itself.
            // Reason: Text drawn using fallback fonts seems to always be black anyway :(
            
            RGBColor color = { 0, 0, 0 };
            setAttribute<RGBColor>(kATSUColorTag, color);
            setLayoutControl<CGContextRef>(kATSUCGContextTag, context);
            checkErr( ATSUSetTransientFontMatching(layout, TRUE) );
            checkErr( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, x, y) );
        }
    };
}

unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    // TODO: special case :///7
    if (text == L" ")
        return fontHeight / 3;
    
    ATSULayoutAndStyle atlas(text, fontName, fontHeight, fontFlags);
    Rect rect = atlas.textExtents();
    return rect.right + 1 - rect.left;
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    CachedFontInfo& font = getFont(fontName);
    
    ATSULayoutAndStyle atlas(text, fontName, fontHeight, fontFlags);
    Rect rect = atlas.textExtents();
    unsigned width = rect.right + 1 - rect.left;
    std::vector<boost::uint32_t> buf(width * fontHeight);
    {
        MacBitmap helper(&buf[0], width, fontHeight);
        atlas.drawToContext(X2Fix(-rect.left), X2Fix(fontHeight / font.heightAt1Pt * font.descentAt1Pt),
                            helper.context());
    }

    Bitmap wholeText;
    wholeText.resize(width, fontHeight);
    for (unsigned relY = 0; relY < fontHeight; ++relY)
        for (unsigned relX = 0; relX < width; ++relX)
        {
#ifdef __BIG_ENDIAN__
            Color::Channel alpha = buf[relY * width + relX];
#else
            Color::Channel alpha = Color(buf[relY * width + relX]).alpha();
#endif
            wholeText.setPixel(relX, relY, Color(alpha, 0xff, 0xff, 0xff));
        }
    bitmap.insert(wholeText, x, y);
}
