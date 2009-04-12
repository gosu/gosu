#include <Gosu/Utility.hpp>
#include <GosuImpl/MacUtility.hpp>
#import <Foundation/Foundation.h>
#include <stdexcept>
#include <vector>
using namespace std;

wstring Gosu::utf8ToWstring(const string& s)
{
    if (s.empty())
        return wstring();
    
    ObjRef<NSString> str([[NSString alloc] initWithUTF8String:s.c_str()]);
    vector<wchar_t> buffer(s.size());
    NSUInteger usedBufferCount;
    if (![str.obj() getBytes:&buffer[0]
                    maxLength:buffer.size() * sizeof(wchar_t)
                    usedLength:&usedBufferCount
                    encoding:NSUTF32LittleEndianStringEncoding
                    options:0
                    range:NSMakeRange(0, [str.obj() length])
                    remainingRange:NULL])
        throw std::runtime_error("String " + s + " could not be converted to Unicode");
    return wstring(&buffer[0], &buffer[0] + usedBufferCount / sizeof(wchar_t));
}
string Gosu::wstringToUTF8(const std::wstring& ws)
{
    if (ws.empty())
        return string();

    ObjRef<NSString> str([[NSString alloc] initWithBytes: ws.data()
                                           length: ws.size() * sizeof(wchar_t)
                                           encoding:NSUTF32LittleEndianStringEncoding]);
    ObjRef<NSAutoreleasePool> pool([[NSAutoreleasePool alloc] init]);
    return [str.obj() UTF8String];
}
wstring Gosu::widen(const string& s)
{
    return utf8ToWstring(s);
}
string Gosu::narrow(const std::wstring& ws)
{
    return wstringToUTF8(ws);
}