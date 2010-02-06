#import <Gosu/Platform.hpp>

#if defined(GOSU_IS_IPHONE) || defined(__LP64__)

#import <Gosu/Text.hpp>
#import <Gosu/Bitmap.hpp>
#import <Gosu/Utility.hpp>
#import <GosuImpl/MacUtility.hpp>
#import <map>
#import <cmath>
using namespace std;

#if defined(GOSU_IS_IPHONE)
#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>
typedef UIFont OSXFont;
#else
#import <AppKit/AppKit.h>
typedef NSFont OSXFont;
#endif

namespace
{
    // TODO: Merge with InputMac.mm
    template<typename CFTypeRef>
    class CFScope : boost::noncopyable
    {
        CFTypeRef ref;
    public:
        explicit CFScope(CFTypeRef ref) : ref(ref) {}
        ~CFScope() { CFRelease(ref); }
        CFTypeRef get() { return ref; }
    };

    map<pair<wstring, double>, OSXFont*> usedFonts;
    OSXFont* getFont(const wstring& fontName, double height)
    {
        OSXFont* result = usedFonts[make_pair(fontName, height)];
        if (!result)
        {
            if (fontName.find(L"/") == std::wstring::npos)
            {
                // System font
                Gosu::ObjRef<NSString> name([[NSString alloc] initWithUTF8String: Gosu::wstringToUTF8(fontName).c_str()]);
                result = [[OSXFont fontWithName: name.obj() size: height] retain];
            }
            else
            {
            #if 0
                // Filename to font
                CFScope<CFStringRef> urlString(
                    CFStringCreateWithBytes(NULL,
                        reinterpret_cast<const UInt8*>(fontName.c_str()),
                        fontName.length() * sizeof(wchar_t),
                        kCFStringEncodingUTF32LE, NO));

                CFScope<CFURLRef> url(
                    CFURLCreateWithFileSystemPath(NULL, urlString.get(),
                        kCFURLPOSIXPathStyle, YES));
                
                CFScope<CFArrayRef> array(
                    CTFontManagerCreateFontDescriptorsFromURL(url.get()));
                
                if (array.get() == NULL || CFArrayGetCount(array.get()) < 1)
            #endif
                    result = getFont(Gosu::defaultFontName(), height);
            #if 0
                else
                {
                    CTFontDescriptorRef ref =
                        CFArrayGetValueAtIndex(array.get(), 0);
                    CFScope<CTStringRef> fontName(
                        (CTStringRef)CTFontDescriptorCopyAttribute(ref.get(), kCTFontNameAttribute));
                    if (CTFontManagerRegisterFontsForURL(url.get(),
                        kCTFontManagerScopeProcess, NULL))
                    {
                        const char* utf8FontName =
                            CFStringGetCStringPtr(fontName.get(), kCFStringEncodingUTF8);
                        result = getFont(Gosu::utf8ToWstring(utf8FontName), height);
                    }
                    else
                        result = getFont(Gosu::defaultFontName(), height);
                }
            #endif
            }
        }
        return result;
    }
}

wstring Gosu::defaultFontName()
{
    // OF COURSE Helvetica is better - but the dots above my capital umlauts get
    // eaten when I use it with Gosu. Until this is fixed, keep Arial. (TODO)
    return L"Arial";
}

unsigned Gosu::textWidth(const wstring& text,
    const wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    OSXFont* font = getFont(fontName, fontHeight);
    
    // This will, of course, compute a too large size; fontHeight is in pixels, the method expects point.
    ObjRef<NSString> string([[NSString alloc] initWithUTF8String: wstringToUTF8(text).c_str()]);
    #ifndef GOSU_IS_IPHONE
    ObjRef<NSDictionary> attributes([[NSDictionary alloc] initWithObjectsAndKeys:
        font, NSFontAttributeName, nil]);
    NSSize size = [string.obj() sizeWithAttributes: attributes.get()];
    #else
    CGSize size = [string.obj() sizeWithFont: font];
    #endif
                           
    // Now adjust the scaling...
    return ceil(size.width / size.height * fontHeight);
}

void Gosu::drawText(Bitmap& bitmap, const wstring& text, int x, int y,
    Color c, const wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    OSXFont* font = getFont(fontName, fontHeight);
    ObjRef<NSString> string([[NSString alloc] initWithUTF8String: wstringToUTF8(text).c_str()]);

    // This will, of course, compute a too large size; fontHeight is in pixels, the method expects point.
    #ifndef GOSU_IS_IPHONE
    ObjRef<NSDictionary> attributes([[NSDictionary alloc] initWithObjectsAndKeys:
        font, NSFontAttributeName, nil]);
    NSSize size = [string.obj() sizeWithAttributes: attributes.get()];
    #else
    CGSize size = [string.obj() sizeWithFont: font];
    #endif
    
    unsigned width = ceil(size.width / size.height * fontHeight);

    // Get the width and height of the image
    Bitmap bmp;
    bmp.resize(width, fontHeight);
    
    // Use a temporary context to draw the CGImage to the buffer.
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context =
        CGBitmapContextCreate(bmp.data(),
                              bmp.width(), bmp.height(), 8, bmp.width() * 4,
                              colorSpace,
                              kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);
    #if defined(GOSU_IS_IPHONE)
    CGFloat color[] = { c.green() / 255.0, c.blue() / 255.0, c.red() / 255.0, 0 };
    CGContextSetStrokeColor(context, color);
    CGContextSetFillColor(context, color);
    #endif
    
    // Use new font with proper size this time.
    font = getFont(fontName, fontHeight * fontHeight / size.height);

    #ifdef GOSU_IS_IPHONE
    CGContextTranslateCTM(context, 0, fontHeight);
    CGContextScaleCTM(context, 1, -1);
    UIGraphicsPushContext(context);
        [string.obj() drawAtPoint: CGPointZero withFont: font];
    UIGraphicsPopContext();
    #else
    NSPoint NSPointZero = { 0, 0 };
    attributes.reset([[NSDictionary alloc] initWithObjectsAndKeys:
        font, NSFontAttributeName, [NSColor whiteColor], NSForegroundColorAttributeName, nil]);
    
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext: [NSGraphicsContext graphicsContextWithGraphicsPort:(void *)context flipped:false]];
        [string.obj() drawAtPoint: NSPointZero withAttributes: attributes.get()];
    [NSGraphicsContext restoreGraphicsState];
    #endif
    CGContextRelease(context);

    // Done!
    bitmap.insert(bmp, x, y);
}

#endif
