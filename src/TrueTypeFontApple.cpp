#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include <Gosu/Text.hpp>
#include "Log.hpp"
#include "TrueTypeFont.hpp"
#include <CoreText/CoreText.h>
#include <Foundation/Foundation.h>
#include <map>

const unsigned char* Gosu::ttf_data_by_name(const std::string& font_name, unsigned font_flags)
{
    // TODO: Make this cache thread-safe.
    static std::map<std::pair<std::string, unsigned>, const unsigned char*> ttf_file_cache;

    auto& ttf_ptr = ttf_file_cache[make_pair(font_name, font_flags)];
    if (ttf_ptr) return ttf_ptr;

    log("Trying to find a font named '%s', flags=%x", font_name.c_str(), font_flags);

    unsigned symbolic_traits = 0;
    if (font_flags & Gosu::FF_BOLD) symbolic_traits |= kCTFontTraitBold;
    if (font_flags & Gosu::FF_ITALIC) symbolic_traits |= kCTFontTraitItalic;

    NSDictionary* attributes = @{
        ((__bridge id) kCTFontNameAttribute): [NSString stringWithUTF8String:font_name.c_str()],
        ((__bridge id) kCTFontTraitsAttribute):
                @{((__bridge id) kCTFontSymbolicTrait): @(symbolic_traits)}
    };
    CTFontDescriptorRef descriptor =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
            CTFontDescriptorCreateWithAttributes((__bridge CFDictionaryRef) attributes);

    if (descriptor) {
        CTFontRef font = CTFontCreateWithFontDescriptorAndOptions(descriptor, 20, nullptr, 0);
        if (font) {
            if ((CTFontGetSymbolicTraits(font) & symbolic_traits) != symbolic_traits) {
                // For some reason the initial call to CTFontCreateWithFontDescriptorAndOptions does
                // not respect the requested font traits. Explicitly requesting the traits again
                // seems to help. Tested on macOS 11.2.2.
                CTFontRef correct_font = CTFontCreateCopyWithSymbolicTraits(
                        font, 0.0, nullptr, symbolic_traits, symbolic_traits);
                CFRelease(font);
                font = correct_font;
            }
            NSURL* url = CFBridgingRelease(CTFontCopyAttribute(font, kCTFontURLAttribute));
            if (url && url.fileSystemRepresentation) {
                log("Loading file '%s'", url.fileSystemRepresentation);
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

std::string Gosu::default_font_name()
{
    return "Arial";
}

#endif
