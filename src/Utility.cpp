#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>

#include "utf8proc.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <algorithm>
#include <stdexcept>
#include <vector>
using namespace std;

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
string Gosu::wstring_to_utf8(const wstring& ws)
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

u32string Gosu::utf8_to_composed_utc4(const string& utf8)
{
    u32string utc4;
    utc4.reserve(utf8.size());
    
    auto* current_byte = reinterpret_cast<const utf8proc_uint8_t*>(utf8.data());
    auto remaining_length = utf8.length();
    
    // First convert from UTF-8 to UTC-4.
    utf8proc_int32_t codepoint;
    while (remaining_length) {
        auto bytes_read = utf8proc_iterate(current_byte, remaining_length, &codepoint);
        if (bytes_read == -1) {
            // Not looking good, skip this byte and retry.
            current_byte += 1;
            remaining_length -= 1;
        } else {
            utc4.push_back(codepoint);
            current_byte += bytes_read;
            remaining_length -= bytes_read;
        }
    }
    
    // Now compose characters in-place.
    auto utc4_data = reinterpret_cast<const utf8proc_int32_t*>(utc4.data());
    // Note that this const_cast will not be necessary anymore in C++17.
    auto writable_utc4_data = const_cast<utf8proc_int32_t*>(utc4_data);
    auto options = static_cast<utf8proc_option_t>(UTF8PROC_NLF2LF | UTF8PROC_COMPOSE);
    auto new_length = utf8proc_normalize_utf32(writable_utc4_data, utc4.length(), options);
    if (new_length < 0) {
        throw runtime_error("Could not normalize '" + utf8 + "': " + utf8proc_errmsg(new_length));
    }
    utc4.resize(new_length);
    
    return utc4;
}

bool Gosu::has_extension(const string& filename, const char* extension)
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
