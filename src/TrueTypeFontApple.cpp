#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include <Gosu/Text.hpp>
#include "TrueTypeFont.hpp"
#include <CoreText/CoreText.h>
#include <Foundation/Foundation.h>
#include <map>

const std::uint8_t* Gosu::ttf_data_by_name(const std::string& font_name, unsigned font_flags)
{
    static std::map<std::pair<std::string, unsigned>, const std::uint8_t*> ttf_file_cache;
    static std::recursive_mutex mutex;
    std::scoped_lock lock(mutex);

    auto& ttf_ptr = ttf_file_cache[make_pair(font_name, font_flags)];
    if (ttf_ptr) {
        return ttf_ptr;
    }

    unsigned symbolic_traits = 0;
    if (font_flags & Gosu::FF_BOLD) {
        symbolic_traits |= kCTFontTraitBold;
    }
    if (font_flags & Gosu::FF_ITALIC) {
        symbolic_traits |= kCTFontTraitItalic;
    }

    NSDictionary* attributes = @{
        ((__bridge id)kCTFontNameAttribute) : [NSString stringWithUTF8String:font_name.c_str()],
        ((__bridge id)kCTFontTraitsAttribute) :
            @ { ((__bridge id)kCTFontSymbolicTrait) : @(symbolic_traits) }
    };
    CTFontDescriptorRef descriptor =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
        CTFontDescriptorCreateWithAttributes((__bridge CFDictionaryRef)attributes);

    if (descriptor) {
        CTFontRef font = CTFontCreateWithFontDescriptorAndOptions(descriptor, 20, nullptr, 0);
        if (font) {
            // GCOV_EXCL_START: This does not seem to happen anymore on recent macOS versions.
            if ((CTFontGetSymbolicTraits(font) & symbolic_traits) != symbolic_traits) {
                // For some reason the initial call to CTFontCreateWithFontDescriptorAndOptions does
                // not respect the requested font traits. Explicitly requesting the traits again
                // seems to help. Tested on macOS 11.2.2.
                CTFontRef correct_font = CTFontCreateCopyWithSymbolicTraits(
                    font, 0.0, nullptr, symbolic_traits, symbolic_traits);
                CFRelease(font);
                font = correct_font;
            }
            // GCOV_EXCL_END
            NSURL* url = CFBridgingRelease(CTFontCopyAttribute(font, kCTFontURLAttribute));
            if (url && url.fileSystemRepresentation) {
                ttf_ptr = ttf_data_from_file(url.fileSystemRepresentation);
            }
            CFRelease(font);
        }
        CFRelease(descriptor);
    }

    return ttf_ptr;
}

const std::uint8_t* Gosu::ttf_fallback_data()
{
    // Prefer Arial Unicode MS as a fallback because it covers a lot of Unicode.
    static const std::uint8_t* arial_unicode = ttf_data_by_name("Arial Unicode MS", 0);
    if (arial_unicode) {
        return arial_unicode;
    }

    // GCOV_EXCL_START: We do not want to delete system fonts in order to test this line.
    return ttf_data_from_file("/Library/Fonts/Arial.ttf");
    // GCOV_EXCL_END
}

std::string Gosu::default_font_name()
{
    return "Arial";
}

#endif
