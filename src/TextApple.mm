#import <Gosu/Platform.hpp>

#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Math.hpp>
#include "AppleUtility.hpp"
#include <map>
#include <cmath>
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
    using Gosu::CFRef;

    // If a font is a filename, loads the font and returns its family name that can be used
    // like any system font. Otherwise, just returns the family name.
    std::wstring normalizeFont(const std::wstring& fontName)
    {
        // On iOS, we have no support for loading font files yet. However if you register your fonts
        // via your app's Info.plist, you should be able to reference them by their name.
        #ifdef GOSU_IS_IPHONE
        return fontName;
        #else
        static map<wstring, wstring> familyOfFiles;
        
        // Not a path name: It is already a family name
        if (fontName.find(L"/") == std::wstring::npos) {
            return fontName;
        }
        
        // Already activated font & extracted family name
        if (familyOfFiles.count(fontName) > 0) {
            return familyOfFiles[fontName];
        }
        
        CFRef<CFStringRef> urlString(
            CFStringCreateWithBytes(NULL,
                reinterpret_cast<const UInt8*>(fontName.c_str()),
                fontName.length() * sizeof(wchar_t),
                kCFStringEncodingUTF32LE, NO));
        CFRef<CFURLRef> url(
            CFURLCreateWithFileSystemPath(NULL, urlString.obj(), kCFURLPOSIXPathStyle, YES));
        if (!url.get()) {
            return familyOfFiles[fontName] = Gosu::defaultFontName();
        }
        
        CFRef<CFArrayRef> array(
            CTFontManagerCreateFontDescriptorsFromURL(url.obj()));

        if (array.get() == NULL || CFArrayGetCount(array.obj()) < 1 ||
                !CTFontManagerRegisterFontsForURL(url.obj(), kCTFontManagerScopeProcess, NULL)) {
            return familyOfFiles[fontName] = Gosu::defaultFontName();
        }

        CTFontDescriptorRef ref =
            (CTFontDescriptorRef)CFArrayGetValueAtIndex(array.get(), 0);
        CFRef<CFStringRef> fontNameStr(
            (CFStringRef)CTFontDescriptorCopyAttribute(ref, kCTFontFamilyNameAttribute));
        
        const char *utf8FontName = [(__bridge NSString *)fontNameStr.obj() UTF8String];
        return familyOfFiles[fontName] = Gosu::utf8ToWstring(utf8FontName);
        #endif
    }

    OSXFont* getFont(wstring fontName, unsigned fontFlags, double height)
    {
        fontName = normalizeFont(fontName);
    
        static map<pair<wstring, pair<unsigned, double> >, OSXFont*> usedFonts;
        
        OSXFont* result = usedFonts[make_pair(fontName, make_pair(fontFlags, height))];
        if (!result)
        {
            NSString *name = [NSString stringWithUTF8String:Gosu::wstringToUTF8(fontName).c_str()];
            #ifdef GOSU_IS_IPHONE
            result = [OSXFont fontWithName:name size:height];
            #else
            NSFontDescriptor* desc = [[NSFontDescriptor fontDescriptorWithFontAttributes:nil]
                                      fontDescriptorWithFamily:name];
            result = [NSFont fontWithDescriptor:desc size:height];
            if (result && (fontFlags & Gosu::ffBold)) {
                result = [[NSFontManager sharedFontManager] convertFont:result toHaveTrait:NSFontBoldTrait];
            }
            if (result && (fontFlags & Gosu::ffItalic)) {
                result = [[NSFontManager sharedFontManager] convertFont:result toHaveTrait:NSFontItalicTrait];
            }
            #endif
            if (!result && fontName != Gosu::defaultFontName()) {
                result = getFont(Gosu::defaultFontName(), 0, height);
            }
            assert(result);
            usedFonts[make_pair(fontName, make_pair(fontFlags, height))] = result;
        }
        return result;
    }
}

wstring Gosu::defaultFontName()
{
    return L"Arial";
}

#ifndef GOSU_IS_IPHONE
namespace
{
    NSDictionary *attributeDictionary(NSFont* font, unsigned fontFlags)
    {
        NSDictionary* dict = @{
            NSFontAttributeName: font,
            NSForegroundColorAttributeName: [NSColor whiteColor]
        };
        if (fontFlags & Gosu::ffUnderline) {
            NSMutableDictionary *mutableDict = [dict mutableCopy];
            // Cannot use accessor syntax here without breaking compilation with OS X 10.7/Xcode 4.6.3.
            [mutableDict setObject:@(NSUnderlineStyleSingle) forKey:NSUnderlineStyleAttributeName];
            dict = [mutableDict copy];
        }
        return dict;
    }
}
#endif

unsigned Gosu::textWidth(const wstring& text,
    const wstring& fontName, unsigned fontHeight, unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != wstring::npos) {
        throw std::invalid_argument("the argument to textWidth cannot contain line breaks");
    }
    
    OSXFont *font = getFont(fontName, fontFlags, fontHeight);
    
    // This will, of course, compute a too large size; fontHeight is in pixels,
    // the method expects point.
    NSString *string = [NSString stringWithUTF8String:wstringToUTF8(text).c_str()];
    #ifndef GOSU_IS_IPHONE
    NSDictionary *attributes = attributeDictionary(font, fontFlags);
    NSSize size = [string sizeWithAttributes:attributes];
    #else
    CGSize size = [string sizeWithFont:font];
    #endif
    
    // Now adjust the scaling...
    return ceil(size.width / size.height * fontHeight);
}

void Gosu::drawText(Bitmap& bitmap, const wstring& text, int x, int y,
    Color c, const wstring& fontName, unsigned fontHeight,
    unsigned fontFlags)
{
    if (text.find_first_of(L"\r\n") != wstring::npos) {
        throw std::invalid_argument("the argument to drawText cannot contain line breaks");
    }
    
    OSXFont *font = getFont(fontName, fontFlags, fontHeight);
    NSString *string = [NSString stringWithUTF8String:wstringToUTF8(text).c_str()];

    // Note that fontHeight is in pixels, the method expects points, so we have to scale this down.
    #ifndef GOSU_IS_IPHONE
    NSDictionary *attributes = attributeDictionary(font, fontFlags);
    NSSize size = [string sizeWithAttributes:attributes];
    #else
    CGSize size = [string sizeWithFont:font];
    #endif
    
    unsigned width = static_cast<unsigned>(round(size.width / size.height * fontHeight));

    // Get the width and height of the image
    Bitmap bmp(width, fontHeight, 0x00ffffff);
    
    // Use a temporary context to draw the CGImage to the buffer.
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context =
        CGBitmapContextCreate(bmp.data(),
                              bmp.width(), bmp.height(), 8, bmp.width() * 4,
                              colorSpace,
                              kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);
    #ifdef GOSU_IS_IPHONE
    CGFloat color[] = { 1.f, 1.f, 1.f, 0.f };
    CGContextSetStrokeColor(context, color);
    CGContextSetFillColor(context, color);
    #endif
    
    // Use new font with proper size this time.
    font = getFont(fontName, fontFlags, fontHeight * fontHeight / size.height);

    #ifdef GOSU_IS_IPHONE
    CGContextTranslateCTM(context, 0, fontHeight);
    CGContextScaleCTM(context, 1, -1);
    UIGraphicsPushContext(context);
    [string drawAtPoint:CGPointZero withFont:font];
    UIGraphicsPopContext();
    #else
    NSPoint NSPointZero = { 0, 0 };
    attributes = attributeDictionary(font, fontFlags);
    
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:
        [NSGraphicsContext graphicsContextWithGraphicsPort:(void *)context flipped:false]];
    [string drawAtPoint:NSPointZero withAttributes:attributes];
    [NSGraphicsContext restoreGraphicsState];
    #endif
    CGContextRelease(context);

    int effectiveWidth = Gosu::clamp<int>(width, 0, bitmap.width() - x);
    int effectiveHeight = Gosu::clamp<int>(fontHeight, 0, bitmap.height() - y);
    
    // Now copy the set pixels back.
    for (int relY = 0; relY < effectiveHeight; ++relY)
        for (int relX = 0; relX < effectiveWidth; ++relX)
        {
            c.setAlpha(bmp.getPixel(relX, relY).alpha());
            if (c.alpha()) {
                bitmap.setPixel(x + relX, y + relY, c);
            }
        }
}
