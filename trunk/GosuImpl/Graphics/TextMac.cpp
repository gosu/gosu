#include <Gosu/Bitmap.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>
#include <boost/utility.hpp>
#include <boost/cstdint.hpp>
#include <cmath>
#include <stdexcept>
#include <vector>
#include <ApplicationServices/ApplicationServices.h>

std::wstring Gosu::defaultFontName()
{
    return L"Arial";
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
    str << "Error on line " << line << " (" << status << ")";
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
        ATSULayoutAndStyle(const std::wstring& text, const std::wstring& fontName,
                           double fontHeightPt, unsigned fontFlags)
        {
            utf16 = Gosu::wstringToUniChars(text);
        
            checkErr( ATSUCreateStyle(&style) );
            
            ATSUFontID font;
            std::string narrowFontName = Gosu::narrow(fontName);
            checkErr( ATSUFindFontFromName(narrowFontName.data(), narrowFontName.length(),
                                               kFontFullName, kFontNoPlatformCode, kFontNoScriptCode,
                                               kFontNoLanguageCode, &font) );

            setAttribute<ATSUFontID>(kATSUFontTag, font);
            
            setAttribute<Fixed>(kATSUSizeTag, X2Fix(fontHeightPt));
            if (fontFlags & Gosu::ffBold)
                setAttribute<Boolean>(kATSUQDBoldfaceTag, TRUE);
            if (fontFlags & Gosu::ffItalic)
                setAttribute<Boolean>(kATSUQDItalicTag, TRUE);
            if (fontFlags & Gosu::ffUnderline)
                setAttribute<Boolean>(kATSUQDUnderlineTag, TRUE);

            UniCharCount runLength = utf16.size();
            checkErr( ATSUCreateTextLayoutWithTextPtr(&utf16[0], kATSUFromTextBeginning,
                            kATSUToTextEnd, utf16.size(), 1, &runLength, &style, &layout) );
            
            checkErr( ATSUSetTransientFontMatching(layout, TRUE) ); // TODO: WHY DOESN'T IT WORK!??
        }
        
        ~ATSULayoutAndStyle()
        {
            checkErr( ATSUDisposeStyle(style) );
            checkErr( ATSUDisposeTextLayout(layout) );
        }

        Rect textExtents() const
        {
            Rect rect;
            checkErr( ATSUMeasureTextImage(layout, kATSUFromTextBeginning,
                                           kATSUToTextEnd, X2Fix(0), X2Fix(0), &rect) );
            return rect;
        }
        
        void drawToContext(Fixed x, Fixed y, CGContextRef context)
        {
            RGBColor color = { 0xffff, 0xffff, 0xffff };
            setAttribute<RGBColor>(kATSUColorTag, color);
            setLayoutControl<CGContextRef>(kATSUCGContextTag, context);
            checkErr( ATSUDrawText(layout, kATSUFromTextBeginning, kATSUToTextEnd, x, y) );
        }
    };
    
    void getMetricsAt1Pt(const std::wstring& fontName, double& height, double& descent)
    {
        // IMPR: Caching
        
        CFStringRef cfName = CFStringCreateWithCString(NULL, Gosu::narrow(fontName).c_str(), kCFStringEncodingASCII);
        ATSFontRef font;
        font = ATSFontFindFromName(cfName, kATSOptionFlagsDefault);
        CFRelease(cfName);
        
        ATSFontMetrics metrics;
        checkErr(ATSFontGetHorizontalMetrics(font, kATSOptionFlagsDefault, &metrics));
        height = metrics.ascent - metrics.descent;
        descent = -metrics.descent;
    }
}

unsigned Gosu::textWidth(const std::wstring& text,
    const std::wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    // TODO: special case :///7
    if (text == L" ")
        return fontHeight / 3;
    
    double heightAt1Pt, dummy;
    getMetricsAt1Pt(fontName, heightAt1Pt, dummy);
    
    ATSULayoutAndStyle atlas(text, fontName, fontHeight / heightAt1Pt, fontFlags);
    Rect rect = atlas.textExtents();
    return rect.right + 1 - rect.left;
}

void Gosu::drawText(Bitmap& bitmap, const std::wstring& text, int x, int y,
    Color c, const std::wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    double heightAt1Pt, descentAt1Pt;
    getMetricsAt1Pt(fontName, heightAt1Pt, descentAt1Pt);
    
    ATSULayoutAndStyle atlas(text, fontName, fontHeight / heightAt1Pt, fontFlags);
    Rect rect = atlas.textExtents();
    unsigned width = rect.right + 1 - rect.left;
    std::vector<boost::uint32_t> buf(width * fontHeight);
    {
        MacBitmap helper(&buf[0], width, fontHeight);
        atlas.drawToContext(X2Fix(-rect.left), X2Fix(fontHeight / heightAt1Pt * descentAt1Pt),
                            helper.context());
    }

    Bitmap wholeText;
    wholeText.resize(width, fontHeight);
    for (unsigned relY = 0; relY < fontHeight; ++relY)
        for (unsigned relX = 0; relX < width; ++relX)
            wholeText.setPixel(relX, relY, buf[relY * width + relX]);
    bitmap.insert(wholeText, x, y);
}
