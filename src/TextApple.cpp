#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include "TextImpl.hpp"

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


static AppleFont* get_font(string font_name, unsigned font_flags, double height)
{
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
    
    if (font_name.find_first_of("./\\") != text.npos) {
        return text_width_ttf(text, font_name, font_height, font_flags);
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
    
    if (font_name.find_first_of("./\\") != text.npos) {
        return draw_text_ttf(bitmap, text, x, y, c, font_name, font_height, font_flags);
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
