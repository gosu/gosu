#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include "TextImpl.hpp"

#include <Gosu/Text.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Math.hpp>
#include <Gosu/Utility.hpp>

#include <cmath>
#include <map>

#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>

using namespace std;

static string find_font_filename(string font_name, unsigned font_flags)
{
    if (font_name.find_first_of("./\\") != string::npos) return font_name;
    
    static map<pair<string, unsigned>, string> filenames;
    
    auto key = make_pair(font_name, font_flags);
    
    if (filenames[key].empty()) {
        unsigned symbolic_traits = 0;
        if (font_flags & Gosu::FF_BOLD)   symbolic_traits |= kCTFontBoldTrait;
        if (font_flags & Gosu::FF_ITALIC) symbolic_traits |= kCTFontItalicTrait;

        NSDictionary *attributes = @{
            ((__bridge id)kCTFontNameAttribute): [NSString stringWithUTF8String:font_name.c_str()],
            ((__bridge id)kCTFontTraitsAttribute): @{
                ((__bridge id)kCTFontSymbolicTrait): @(symbolic_traits)
            }
        };
        CTFontDescriptorRef descriptor =
            CTFontDescriptorCreateWithAttributes((__bridge CFDictionaryRef)attributes);

        if (descriptor) {
            CTFontRef font = CTFontCreateWithFontDescriptorAndOptions(descriptor, 20, nullptr, 0);
            if (font) {
                NSURL *url = CFBridgingRelease(CTFontCopyAttribute(font, kCTFontURLAttribute));
                if (url && url.fileSystemRepresentation) {
                    filenames[key] = url.fileSystemRepresentation;
                }
                CFRelease(font);
            }
            CFRelease(descriptor);
        }
        
        if (filenames[key].empty() && font_flags != 0) {
            filenames[key] = find_font_filename(font_name, 0);
        }
        if (filenames[key].empty()) {
            filenames[key] = find_font_filename(Gosu::default_font_name(), font_flags);
        }
        if (filenames[key].empty()) {
            filenames[key] = "/Library/Fonts/Arial.ttf";
        }
    }
    
    return filenames[key];
}

string Gosu::default_font_name()
{
    return "Arial";
}

int Gosu::text_width(const string& text, const string& font_name,
                     int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != string::npos) {
        throw invalid_argument("text_width cannot handle line breaks");
    }
    
    return text_width_ttf(text, find_font_filename(font_name, font_flags),
                          font_height, font_flags);
}

void Gosu::draw_text(Bitmap& bitmap, const string& text, int x, int y, Color c,
                     const string& font_name, int font_height, unsigned font_flags)
{
    if (text.find_first_of("\r\n") != text.npos) {
        throw invalid_argument("the argument to draw_text cannot contain line breaks");
    }
    
    return draw_text_ttf(bitmap, text, x, y, c, find_font_filename(font_name, font_flags),
                         font_height, font_flags);
}

#endif
