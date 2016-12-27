#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

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
    std::string normalize_font(const std::string& font_name)
    {
        // On iOS, we have no support for loading font files yet. However if you register your fonts
        // via your app's Info.plist, you should be able to reference them by their name.
        #ifdef GOSU_IS_IPHONE
        return font_name;
        #else
        static map<string, string> family_of_files;
        
        // Not a path name: It is already a family name
        if (font_name.find("/") == std::string::npos) {
            return font_name;
        }
        
        // Already activated font & extracted family name
        if (family_of_files.count(font_name) > 0) {
            return family_of_files[font_name];
        }
        
        CFRef<CFStringRef> url_string(
            CFStringCreateWithCString(NULL, font_name.c_str(), kCFStringEncodingUTF8));
        CFRef<CFURLRef> url(
            CFURLCreateWithFileSystemPath(NULL, url_string.obj(), kCFURLPOSIXPathStyle, YES));
        if (!url.get()) {
            return family_of_files[font_name] = Gosu::default_font_name();
        }
        
        CFRef<CFArrayRef> array(
            CTFontManagerCreateFontDescriptorsFromURL(url.obj()));

        if (array.get() == NULL || CFArrayGetCount(array.obj()) < 1 ||
                !CTFontManagerRegisterFontsForURL(url.obj(), kCTFontManagerScopeProcess, NULL)) {
            return family_of_files[font_name] = Gosu::default_font_name();
        }

        CTFontDescriptorRef ref =
            (CTFontDescriptorRef)CFArrayGetValueAtIndex(array.get(), 0);
        CFRef<CFStringRef> font_name_str(
            (CFStringRef)CTFontDescriptorCopyAttribute(ref, kCTFontFamilyNameAttribute));
        
        const char *utf8_font_name = [(__bridge NSString *)font_name_str.obj() UTF8String];
        return family_of_files[font_name] = std::string(utf8_font_name ?: "");
        #endif
    }

    OSXFont* get_font(string font_name, unsigned font_flags, double height)
    {
        font_name = normalize_font(font_name);
    
        static map<pair<string, pair<unsigned, double> >, OSXFont*> used_fonts;
        
        OSXFont* result = used_fonts[make_pair(font_name, make_pair(font_flags, height))];
        if (!result)
        {
            NSString *name = [NSString stringWithUTF8String:font_name.c_str()];
            #ifdef GOSU_IS_IPHONE
            result = [OSXFont fontWithName:name size:height];
            #else
            NSFontDescriptor* desc = [[NSFontDescriptor fontDescriptorWithFontAttributes:nil]
                                      fontDescriptorWithFamily:name];
            result = [NSFont fontWithDescriptor:desc size:height];
            if (result && (font_flags & Gosu::FF_BOLD)) {
                result = [[NSFontManager sharedFontManager] convertFont:result toHaveTrait:NSFontBoldTrait];
            }
            if (result && (font_flags & Gosu::FF_ITALIC)) {
                result = [[NSFontManager sharedFontManager] convertFont:result toHaveTrait:NSFontItalicTrait];
            }
            #endif
            if (!result && font_name != Gosu::default_font_name()) {
                result = get_font(Gosu::default_font_name(), 0, height);
            }
            assert(result);
            used_fonts[make_pair(font_name, make_pair(font_flags, height))] = result;
        }
        return result;
    }
}

string Gosu::default_font_name()
{
    return "Arial";
}

#ifndef GOSU_IS_IPHONE
namespace
{
    NSDictionary *attribute_dictionary(NSFont* font, unsigned font_flags)
    {
        NSDictionary* dict = @{
            NSFontAttributeName: font,
            NSForegroundColorAttributeName: [NSColor whiteColor]
        };
        if (font_flags & Gosu::FF_UNDERLINE) {
            NSMutableDictionary *mutable_dict = [dict mutableCopy];
            // Cannot use accessor syntax here without breaking compilation with OS X 10.7/Xcode 4.6.3.
            [mutable_dict setObject:@(NSUnderlineStyleSingle) forKey:NSUnderlineStyleAttributeName];
            dict = [mutable_dict copy];
        }
        return dict;
    }
}
#endif

unsigned Gosu::text_width(const wstring& text,
    const string& font_name, unsigned font_height, unsigned font_flags)
{
    if (text.find_first_of(L"\r\n") != wstring::npos) {
        throw std::invalid_argument("the argument to text_width cannot contain line breaks");
    }
    
    OSXFont *font = get_font(font_name, font_flags, font_height);
    
    // This will, of course, compute a too large size; font_height is in pixels,
    // the method expects point.
    NSString *string = [NSString stringWithUTF8String:wstring_to_utf8(text).c_str()];
    #ifndef GOSU_IS_IPHONE
    NSDictionary *attributes = attribute_dictionary(font, font_flags);
    NSSize size = [string sizeWithAttributes:attributes];
    #else
    CGSize size = [string sizeWithFont:font];
    #endif
    
    // Now adjust the scaling...
    return ceil(size.width / size.height * font_height);
}

void Gosu::draw_text(Bitmap& bitmap, const wstring& text, int x, int y,
    Color c, const string& font_name, unsigned font_height,
    unsigned font_flags)
{
    if (text.find_first_of(L"\r\n") != wstring::npos) {
        throw std::invalid_argument("the argument to draw_text cannot contain line breaks");
    }
    
    OSXFont *font = get_font(font_name, font_flags, font_height);
    NSString *string = [NSString stringWithUTF8String:wstring_to_utf8(text).c_str()];

    // Note that font_height is in pixels, the method expects points, so we have to scale this down.
    #ifndef GOSU_IS_IPHONE
    NSDictionary *attributes = attribute_dictionary(font, font_flags);
    NSSize size = [string sizeWithAttributes:attributes];
    #else
    CGSize size = [string sizeWithFont:font];
    #endif
    
    unsigned width = static_cast<unsigned>(round(size.width / size.height * font_height));

    // Get the width and height of the image
    Bitmap bmp(width, font_height, 0x00ffffff);
    
    // Use a temporary context to draw the CGImage to the buffer.
    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context =
        CGBitmapContextCreate(bmp.data(),
                              bmp.width(), bmp.height(), 8, bmp.width() * 4,
                              color_space,
                              kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(color_space);
    #ifdef GOSU_IS_IPHONE
    CGFloat color[] = { 1.f, 1.f, 1.f, 0.f };
    CGContextSetStrokeColor(context, color);
    CGContextSetFillColor(context, color);
    #endif
    
    // Use new font with proper size this time.
    font = get_font(font_name, font_flags, font_height * font_height / size.height);

    #ifdef GOSU_IS_IPHONE
    CGContextTranslateCTM(context, 0, font_height);
    CGContextScaleCTM(context, 1, -1);
    UIGraphicsPushContext(context);
    [string drawAtPoint:CGPointZero withFont:font];
    UIGraphicsPopContext();
    #else
    NSPoint NSPointZero = { 0, 0 };
    attributes = attribute_dictionary(font, font_flags);
    
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:
        [NSGraphicsContext graphicsContextWithGraphicsPort:(void *)context flipped:false]];
    [string drawAtPoint:NSPointZero withAttributes:attributes];
    [NSGraphicsContext restoreGraphicsState];
    #endif
    CGContextRelease(context);

    int effective_width = Gosu::clamp<int>(width, 0, bitmap.width() - x);
    int effective_height = Gosu::clamp<int>(font_height, 0, bitmap.height() - y);
    
    // Now copy the set pixels back.
    for (int rel_y = 0; rel_y < effective_height; ++rel_y)
        for (int rel_x = 0; rel_x < effective_width; ++rel_x)
        {
            c.set_alpha(bmp.get_pixel(rel_x, rel_y).alpha());
            if (c.alpha()) {
                bitmap.set_pixel(x + rel_x, y + rel_y, c);
            }
        }
}

#endif
