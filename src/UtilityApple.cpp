#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>
#include "AppleUtility.hpp"
#import <Foundation/Foundation.h>
#include <stdexcept>
#include <vector>
using namespace std;

#ifdef GOSU_IS_IPHONE // (but could also be used for OS X)
wstring Gosu::utf8_to_wstring(const string& s)
{
    if (s.empty()) {
        return wstring();
    }
    
    NSString *str = [NSString stringWithUTF8String:s.c_str()];
    vector<wchar_t> buffer(s.size());
    NSUInteger used_buffer_count;
    if (![str getBytes:&buffer[0]
             maxLength:buffer.size() * sizeof(wchar_t)
            usedLength:&used_buffer_count
              encoding:NSUTF32LittleEndianStringEncoding
               options:0
                 range:NSMakeRange(0, str.length)
            remainingRange:NULL]) {
        throw std::runtime_error("String " + s + " could not be converted to Unicode");
    }
    return wstring(&buffer[0], &buffer[0] + used_buffer_count / sizeof(wchar_t));
}

string Gosu::wstring_to_utf8(const std::wstring& ws)
{
    if (ws.empty()) {
        return string();
    }

    @autoreleasepool {
        NSString *str = [[NSString alloc] initWithBytes:ws.data()
                                                 length:ws.size() * sizeof(wchar_t)
                                               encoding:NSUTF32LittleEndianStringEncoding];
        return str.UTF8String ?: string();
    }
}
#endif

string Gosu::language()
{
    @autoreleasepool {
        // Cannot use accessor syntax here without breaking compilation with OS X 10.7/Xcode 4.6.3.
        NSString *language = [[NSLocale preferredLanguages] objectAtIndex:0];
        return language.UTF8String ?: "en";
    }
}

#endif
