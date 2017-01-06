#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <algorithm>
#include <stdexcept>
#include <vector>

#ifndef GOSU_IS_IPHONE

#ifndef GOSU_IS_WIN
#include "Iconv.hpp"
#endif
using namespace std;

#ifndef GOSU_IS_WIN
namespace
{
    extern const char UTF_8[] = "UTF-8";
#ifdef __BIG_ENDIAN__
    extern const char UCS_4_INTERNAL[] = "UCS-4BE";
#else
    extern const char UCS_4_INTERNAL[] = "UCS-4LE";
#endif
}

wstring Gosu::utf8_to_wstring(const string& s)
{
    return iconvert<wstring, UCS_4_INTERNAL, UTF_8>(s);
}
string Gosu::wstring_to_utf8(const std::wstring& ws)
{
    return iconvert<string, UTF_8, UCS_4_INTERNAL>(ws);
}

#else
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
wstring Gosu::utf8_to_wstring(const string& utf8)
{
    vector<wchar_t> buffer(utf8.size() + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size() + 1, &buffer[0], buffer.size());
    return &buffer[0];
}
string Gosu::wstring_to_utf8(const wstring& ws)
{
    unsigned size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), 0, 0, 0, 0);
    vector<char> buffer(size + 1);
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), &buffer[0], buffer.size(), 0, 0);
    return &buffer[0];
}
#endif
#endif

bool Gosu::has_extension(const std::string& filename, const char* extension)
{
    size_t ext_len = strlen(extension);
    if (ext_len > filename.length()) {
        return false;
    }

    const char* str = filename.c_str() + filename.length();
    const char* ext = extension + ext_len;
    while (ext_len--) {
        if (tolower((int) *--str) != *--ext) {
            return false;
        }
    }

    return true;
}

#if defined(GOSU_IS_UNIX) && !defined(GOSU_IS_MAC)
string Gosu::language()
{
    return getenv("LANG");
}
#endif
