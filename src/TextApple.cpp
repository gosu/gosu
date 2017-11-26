#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Utility.hpp>
#include <cmath>
#include <map>

#if defined(GOSU_IS_IPHONE)
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
typedef UIFont AppleFont;
#else
#import <AppKit/AppKit.h>
typedef NSFont AppleFont;
#endif

using namespace std;

// If a font is a filename, loads the font and returns its family name that can be used
// like any system font. Otherwise, just returns the family name.
static string normalize_font(const string& font_name)
{
#ifdef GOSU_IS_IPHONE
    // On iOS, we have no support for loading font files yet. However, if you register your fonts
    // via your app's Info.plist, you should be able to reference them by name.
    return font_name;
#else
    static map<string, string> family_of_files;
    
    // Not a path name: It is already a family name.
    if (font_name.find("/") == font_name.npos) {
        return font_name;
    }
    
    // Already activated font & extracted family name.
    if (family_of_files.count(font_name) > 0) {
        return family_of_files[font_name];
    }
    
    NSURL* url = [NSURL fileURLWithPath:[NSString stringWithUTF8String:font_name.c_str()]
                            isDirectory:NO];
    if (url == nullptr) {
        return family_of_files[font_name] = Gosu::default_font_name();
    }
    CFURLRef url_ref = (__bridge CFURLRef) url;
    
    NSArray* descriptors = CFBridgingRelease(CTFontManagerCreateFontDescriptorsFromURL(url_ref));
    if (descriptors.count < 1 ||
            !CTFontManagerRegisterFontsForURL(url_ref, kCTFontManagerScopeProcess, nullptr)) {
        return family_of_files[font_name] = Gosu::default_font_name();
    }

    CTFontDescriptorRef ref = (__bridge CTFontDescriptorRef) descriptors[0];
    CFTypeRef family_name_ref = CTFontDescriptorCopyAttribute(ref, kCTFontFamilyNameAttribute);
    NSString* family_name = CFBridgingRelease(family_name_ref);
    return family_of_files[font_name] = family_name.UTF8String ?: "";
#endif
}

static AppleFont* get_font(string font_name, unsigned font_flags, double height)
{
    font_name = normalize_font(font_name);

    static map<pair<string, pair<unsigned, double>>, AppleFont*> used_fonts;
    
    auto key = make_pair(font_name, make_pair(font_flags, height));
    
    AppleFont* result = used_fonts[key];
    if (!result) {
        NSString* name = [NSString stringWithUTF8String:font_name.c_str()];
    #ifdef GOSU_IS_IPHONE
        result = [AppleFont fontWithName:name size:height];
    #else
        NSFontDescriptor* desc =
            [[NSFontDescriptor fontDescriptorWithFontAttributes:nil] fontDescriptorWithFamily:name];
        result = [NSFont fontWithDescriptor:desc size:height];
        if (result && (font_flags & Gosu::FF_BOLD)) {
            result =
                [[NSFontManager sharedFontManager] convertFont:result toHaveTrait:NSFontBoldTrait];
        }
        if (result && (font_flags & Gosu::FF_ITALIC)) {
            result = [[NSFontManager sharedFontManager] convertFont:result
                                                        toHaveTrait:NSFontItalicTrait];
        }
    #endif
        if (result == nullptr) {
            if (font_name != Gosu::default_font_name()) {
                result = get_font(Gosu::default_font_name(), 0, height);
            }
            else {
                throw runtime_error("Cannot load default font");
            }
        }
        used_fonts[key] = result;
    }
    return result;
}

string Gosu::default_font_name()
{
    return "Arial";
}

#ifndef GOSU_IS_IPHONE
static NSDictionary* attribute_dictionary(NSFont* font, unsigned font_flags)
{
    auto underline_style =
        (font_flags & Gosu::FF_UNDERLINE) ? NSUnderlineStyleSingle : NSUnderlineStyleNone;
    return @{
        NSFontAttributeName: font,
        NSForegroundColorAttributeName: [NSColor whiteColor],
        NSUnderlineStyleAttributeName: @(underline_style)
    };
}
#endif

int Gosu::text_width(const string& text, const string& font_name,
                     int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != text.npos) {
        throw invalid_argument("text_width cannot handle line breaks");
    }
    
    AppleFont* font = get_font(font_name, font_flags, font_height);
    
    // This will, of course, compute a too large size; font_height is in pixels,
    // the method expects point.
    NSString* string = [NSString stringWithUTF8String:text.c_str()];
#ifndef GOSU_IS_IPHONE
    NSDictionary* attributes = attribute_dictionary(font, font_flags);
    NSSize size = [string sizeWithAttributes:attributes];
#else
    CGSize size = [string sizeWithFont:font];
#endif
    
    // Now adjust the scaling...
    return ceil(size.width / size.height * font_height);
}

void Gosu::draw_text(Bitmap& bitmap, const string& text, int x, int y, Color c,
                     const string& font_name, int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != text.npos) {
        throw invalid_argument("the argument to draw_text cannot contain line breaks");
    }
    
    AppleFont* font = get_font(font_name, font_flags, font_height);
    NSString* string = [NSString stringWithUTF8String:text.c_str()];

#ifndef GOSU_IS_IPHONE
    NSDictionary* attributes = attribute_dictionary(font, font_flags);
    NSSize size = [string sizeWithAttributes:attributes];
#else
    CGSize size = [string sizeWithFont:font];
#endif
    
    int width = static_cast<int>(round(size.width / size.height * font_height));

    // Get the width and height of the image
    Bitmap bmp(width, font_height, 0x00ffffff);
    
    // Use a temporary context to draw the CGImage to the buffer.
    CGColorSpaceRef color_space = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(bmp.data(), bmp.width(), bmp.height(), 8,
        bmp.width() * 4, color_space, kCGImageAlphaPremultipliedLast);
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
    [NSGraphicsContext
        setCurrentContext:[NSGraphicsContext graphicsContextWithGraphicsPort:(void*)context
                                                                     flipped:false]];
    [string drawAtPoint:NSPointZero withAttributes:attributes];
    [NSGraphicsContext restoreGraphicsState];
#endif
    CGContextRelease(context);

    int effective_width = Gosu::clamp<int>(width, 0, bitmap.width() - x);
    int effective_height = Gosu::clamp<int>(font_height, 0, bitmap.height() - y);
    
    // Now copy the set pixels back.
    for (int rel_y = 0; rel_y < effective_height; ++rel_y) {
        for (int rel_x = 0; rel_x < effective_width; ++rel_x) {
            c.set_alpha(bmp.get_pixel(rel_x, rel_y).alpha());
            if (c.alpha()) {
                bitmap.set_pixel(x + rel_x, y + rel_y, c);
            }
        }
    }
}

#endif
