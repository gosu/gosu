#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_MAC)

#import <Gosu/Utility.hpp>
#import <Foundation/Foundation.h>
#import <stdexcept>
#import <vector>
using namespace std;

#ifdef GOSU_IS_IPHONE
wstring Gosu::utf8_to_wstring(const string& s)
{
    if (s.empty()) return wstring();
    
    NSString* string = [NSString stringWithUTF8String:s.c_str()];
    vector<wchar_t> buffer(s.size());
    NSUInteger buffer_size;
    if (![string getBytes:&buffer[0]
                maxLength:buffer.size() * sizeof(wchar_t)
               usedLength:&buffer_size
                 encoding:NSUTF32LittleEndianStringEncoding
                  options:0
                    range:NSMakeRange(0, string.length)
           remainingRange:nullptr]) {
        throw runtime_error("String " + s + " could not be converted to UTF-32");
    }
    return wstring(&buffer[0], &buffer[0] + buffer_size / sizeof(wchar_t));
}

string Gosu::wstring_to_utf8(const wstring& ws)
{
    if (ws.empty()) return string();

    @autoreleasepool {
        NSString* string = [[NSString alloc] initWithBytes:ws.data()
                                                    length:ws.size() * sizeof(wchar_t)
                                                  encoding:NSUTF32LittleEndianStringEncoding];
        return string.UTF8String ?: string();
    }
}
#endif

string Gosu::language()
{
    @autoreleasepool {
        NSString* language = [NSLocale preferredLanguages][0];
        return language.UTF8String ?: "en";
    }
}

#endif
