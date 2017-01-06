#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#import <Gosu/Utility.hpp>
#import <Foundation/Foundation.h>
#import <stdexcept>
#import <vector>

#ifdef GOSU_IS_IPHONE
std::wstring Gosu::utf8_to_wstring(const std::string& s)
{
    if (s.empty()) {
        return std::wstring();
    }
    
    NSString* string = [NSString stringWithUTF8String:s.c_str()];
    std::vector<wchar_t> buffer(s.size());
    NSUInteger buffer_size;
    if (![string getBytes:&buffer[0]
                maxLength:buffer.size() * sizeof(wchar_t)
               usedLength:&buffer_size
                 encoding:NSUTF32LittleEndianStringEncoding
                  options:0
                    range:NSMakeRange(0, string.length)
           remainingRange:nullptr]) {
        throw std::runtime_error("String " + s + " could not be converted to UTF-32");
    }
    return std::wstring(&buffer[0], &buffer[0] + buffer_size / sizeof(wchar_t));
}

std::string Gosu::wstring_to_utf8(const std::wstring& ws)
{
    if (ws.empty()) return std::string();

    @autoreleasepool {
        NSString* string = [[NSString alloc] initWithBytes:ws.data()
                                                    length:ws.size() * sizeof(wchar_t)
                                                  encoding:NSUTF32LittleEndianStringEncoding];
        return string.UTF8String ?: std::string();
    }
}
#endif

std::string Gosu::language()
{
    @autoreleasepool {
        NSString* language = [NSLocale preferredLanguages][0];
        return language.UTF8String ?: "en";
    }
}

#endif
