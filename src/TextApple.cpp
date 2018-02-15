#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include "TextImpl.hpp"

#include <Gosu/IO.hpp>
#include <Gosu/Text.hpp>
#include <Gosu/Utility.hpp>

#include <map>
#include <memory>
#include <stdexcept>

#import <CoreText/CoreText.h>
#import <Foundation/Foundation.h>

using namespace std;

static shared_ptr<Gosu::Buffer> find_ttf_data(string font_name, unsigned font_flags)
{
    static map<pair<string, unsigned>, shared_ptr<Gosu::Buffer>> ttf_buffers;

    auto key = make_pair(font_name, font_flags);
    
    auto iterator = ttf_buffers.find(key);
    if (iterator != ttf_buffers.end()) {
        return iterator->second;
    }
    
    // A filename? Just load it.
    if (font_name.find_first_of("./\\") != string::npos) {
        auto buffer = make_shared<Gosu::Buffer>();
        Gosu::load_file(*buffer, font_name);
        // Assign this buffer to all combinations of the given filename, since Gosu::FontFlags are
        // not yet supported for custom TTF files - this avoids loading the same file twice.
        for (unsigned flags = 0; flags < Gosu::FF_COMBINATIONS; ++flags) {
            ttf_buffers[make_pair(font_name, flags)] = buffer;
        }
        return buffer;
    }
    
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
                ttf_buffers[key] = find_ttf_data(url.fileSystemRepresentation, font_flags);
            }
            CFRelease(font);
        }
        CFRelease(descriptor);
    }
    
    if (!ttf_buffers[key] && font_flags != 0) {
        ttf_buffers[key] = find_ttf_data(font_name, 0);
    }
    if (!ttf_buffers[key] && font_name != Gosu::default_font_name()) {
        ttf_buffers[key] = find_ttf_data(Gosu::default_font_name(), font_flags);
    }
    if (!ttf_buffers[key]) {
        ttf_buffers[key] = find_ttf_data("/Library/Fonts/Arial.ttf", 0);
    }
    
    return ttf_buffers[key];
}

string Gosu::default_font_name()
{
    return "Arial";
}

int Gosu::text_width(const string& text, const string& font_name,
                     int font_height, unsigned font_flags)
{
    if (font_flags >= FF_COMBINATIONS) {
        throw invalid_argument("Invalid font_flags: " + to_string(font_flags));
    }
    
    if (text.find_first_of("\r\n") != string::npos) {
        throw invalid_argument("text_width cannot handle line breaks");
    }

    Gosu::Buffer& ttf_data = *find_ttf_data(font_name, font_flags);
    return text_width_ttf(static_cast<unsigned char*>(ttf_data.data()), font_height,
                          text);
}

int Gosu::draw_text(Bitmap& bitmap, const string& text, int x, int y, Color c,
                    const string& font_name, int font_height, unsigned font_flags)
{
    if (font_flags >= FF_COMBINATIONS) {
        throw invalid_argument("Invalid font_flags: " + to_string(font_flags));
    }
    
    if (text.find_first_of("\r\n") != text.npos) {
        throw invalid_argument("the argument to draw_text cannot contain line breaks");
    }
    
    Gosu::Buffer& ttf_data = *find_ttf_data(font_name, font_flags);
    return draw_text_ttf(static_cast<unsigned char*>(ttf_data.data()), font_height,
                         text, bitmap, x, y, c);
}

#endif
