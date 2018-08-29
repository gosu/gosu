#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include "TrueTypeFont.hpp"

#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>

#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>

#include <map>
using namespace std;

const unsigned char* Gosu::ttf_data_by_name(const string& font_name, unsigned font_flags)
{
    // TODO: Make this cache thread-safe.
    static map<pair<string, unsigned>, const unsigned char*> ttf_file_cache;
    
    auto& ttf_ptr = ttf_file_cache[make_pair(font_name, font_flags)];
    if (ttf_ptr) return ttf_ptr;

    unsigned symbolic_traits = 0;
    if (font_flags & Gosu::FF_BOLD)   symbolic_traits |= kCTFontBoldTrait;
    if (font_flags & Gosu::FF_ITALIC) symbolic_traits |= kCTFontItalicTrait;

    NSDictionary *attributes = @{
        ((__bridge id) kCTFontNameAttribute): [NSString stringWithUTF8String:font_name.c_str()],
        ((__bridge id) kCTFontTraitsAttribute): @{
            ((__bridge id) kCTFontSymbolicTrait): @(symbolic_traits)
        }
    };
    CTFontDescriptorRef descriptor =
        CTFontDescriptorCreateWithAttributes((__bridge CFDictionaryRef) attributes);

    if (descriptor) {
        CTFontRef font = CTFontCreateWithFontDescriptorAndOptions(descriptor, 20, nullptr, 0);
        if (font) {
            NSURL *url = CFBridgingRelease(CTFontCopyAttribute(font, kCTFontURLAttribute));
            if (url && url.fileSystemRepresentation) {
                ttf_ptr = ttf_data_from_file(url.fileSystemRepresentation);
            }
            CFRelease(font);
        }
        CFRelease(descriptor);
    }
    
    return ttf_ptr;
}

const unsigned char* Gosu::ttf_fallback_data()
{
    // Prefer Arial Unicode MS as a fallback because it covers a lot of Unicode.
    static const unsigned char* arial_unicode = ttf_data_by_name("Arial Unicode MS", 0);
    if (arial_unicode) return arial_unicode;
    
    return ttf_data_from_file("/Library/Fonts/Arial.ttf");
}

string Gosu::default_font_name()
{
    return "Arial";
}

#endif
