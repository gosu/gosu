#include <Gosu/Platform.hpp>

#if !defined(GOSU_IS_IPHONE) && !defined(__LP64__)

#include <Gosu/Bitmap.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/IO.hpp>
#include <GosuImpl/MacUtility.hpp>
#include <cmath>
#include <stdexcept>
#include <map>
#include <vector>
#include <ApplicationServices/ApplicationServices.h>

std::wstring Gosu::defaultFontName()
{
    // OF COURSE Helvetica is better - but the dots above my capital umlauts get
    // eaten when I use it with Gosu. Until this is fixed, keep Arial. (TODO)
    return L"Arial";
}

namespace Gosu
{
    std::vector<unsigned short> wstringToUniChars(const std::wstring& ws);
}

namespace
{
    class MacBitmap
    {
        std::tr1::uint32_t* buf;
        unsigned width, height;
        CGContextRef ctx;
        
        MacBitmap(const MacBitmap&);
        MacBitmap& operator=(const MacBitmap&);

    public:
        MacBitmap(std::tr1::uint32_t* buf, unsigned width, unsigned height)
        : buf(buf), width(width), height(height)
        {
            CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB); 
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
            CHECK_OS( ATSFontActivateFromMemory(buf.data(), buf.size(),
                          kATSFontContextLocal, kATSFontFormatUnspecified, 
                          NULL, kATSOptionFlagsDefault, &container) );
            
            ATSFontRef fontRefs[1024]; 
            ItemCount  fontCount; 
            CHECK_OS( ATSFontFindFromContainer(container, kATSOptionFlagsDefault, 
                          1024, fontRefs, &fontCount) );
            if (fontCount == 0)
                throw std::runtime_error("No font found in " + Gosu::narrow(fontName));

            atsRef = fontRefs[0];
        }
        
        // Calculate metrics (for space allocations) and create CachedFontInfo entry.
        CachedFontInfo newFont;
        newFont.fontId = FMGetFontFromATSFontRef(atsRef);

        ATSFontMetrics metrics;
        CHECK_OS(ATSFontGetHorizontalMetrics(newFont.fontId, kATSOptionFlagsDefault, &metrics));
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
            CHECK_OS( ATSUSetAttributes(style, 1, &tag, &size, &ptr) );
        }
        
        template<typename T>
        void setLayoutControl(ATSUAttributeTag tag, T value)
        {
            ByteCount size = sizeof value;
            ATSUAttributeValuePtr ptr = &value;
            CHECK_OS( ATSUSetLayoutControls(layout, 1, &tag, &size, &ptr) );
        }
        
    public:
        ATSULayoutAndStyle(const std::wstring& text, const std::wstring& fontName, unsigned fontHeightPx, unsigned fontFlags)
        {
            utf16 = Gosu::wstringToUniChars(text);
        
            CHECK_OS( ATSUCreateStyle(&style) );
            
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
            CHECK_OS( ATSUCreateTextLayoutWithTextPtr(&utf16[0], kATSUFromTextBeginning,
                            kATSUToTextEnd, utf16.size(), 1, &runLength, &style, &layout) );
        }
        
        ~ATSULayoutAndStyle()
        {
            CHECK_OS( ATSUDisposeStyle(style) );
            CHECK_OS( ATSUDisposeTextLayout(layout) );
        }

        Rect textExtents() const
        {
            Rect rect;
            CHECK_OS( ATSUSetTransientFontMatching(layout, TRUE) );
            CHECK_OS( ATSUMeasureTextImage(layout, kATSUFromTextBeginning,
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
            CHECK_OS( ATSUSetTransientFontMatching(layout, TRUE) );
            CHECK_OS( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, x, y) );
        }
    };
}

unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to textWidth cannot contain line breaks");
    
    // TODO: Why 1?
    if (text.empty())
        return 1;
    
    // TODO: special case :(
    // TODO: Why? I guess it was empty otherwise?
    if (text == L" ")
        return fontHeight / 3;
    
    ATSULayoutAndStyle atlas(text, fontName, fontHeight, fontFlags);
    Rect rect = atlas.textExtents();
    return rect.right + 1 - rect.left + 1; // add one pixel on OS X
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != std::wstring::npos)
        throw std::invalid_argument("the argument to drawText cannot contain line breaks");
    
    if (text.empty())
        return;

    CachedFontInfo& font = getFont(fontName);
    
    ATSULayoutAndStyle atlas(text, fontName, fontHeight, fontFlags);
    Rect rect = atlas.textExtents();
    unsigned width = rect.right + 1 - rect.left + 1; // add one pixel on OS X
    std::vector<std::tr1::uint32_t> buf(width * fontHeight);
    {
        MacBitmap helper(&buf[0], width, fontHeight);
        atlas.drawToContext(X2Fix(-rect.left), X2Fix(fontHeight / font.heightAt1Pt * font.descentAt1Pt),
                            helper.context());
    }

    for (unsigned relY = 0; relY < fontHeight; ++relY)
        for (unsigned relX = 0; relX < width; ++relX)
        {
#ifdef __BIG_ENDIAN__
            Color::Channel alpha = buf[relY * width + relX];
#else
            Color::Channel alpha = Color(buf[relY * width + relX]).alpha();
#endif
            if (alpha != 0)
                bitmap.setPixel(x + relX, y + relY, multiply(c, Color(alpha, 0xff, 0xff, 0xff)));
        }
}

#endif
