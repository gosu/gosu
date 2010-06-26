#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <vector>

#ifndef GOSU_IS_WIN
#include <GosuImpl/Iconv.hpp>
#endif
using namespace std;

#ifndef GOSU_IS_WIN
namespace {
    extern const char UTF_8[] = "UTF-8";
#ifdef __BIG_ENDIAN__
    extern const char UCS_4_INTERNAL[] = "UCS-4BE";
#else
    extern const char UCS_4_INTERNAL[] = "UCS-4LE";
#endif
}

wstring Gosu::utf8ToWstring(const string& s)
{
    return iconvert<wstring, UCS_4_INTERNAL, UTF_8>(s);
}
string Gosu::wstringToUTF8(const std::wstring& ws)
{
    return iconvert<string, UTF_8, UCS_4_INTERNAL>(ws);
}

#ifdef GOSU_IS_MAC
// This is only necessary on OS X (for text output)
// TODO: Move to respective files now that iconvert<> has been extracted
// from this file.

namespace {
    extern const char MACROMAN[] = "MacRoman";
    extern const char UCS_2_INTERNAL[] = "UCS-2-INTERNAL";
}

namespace Gosu {
    wstring macRomanToWstring(const string& s)
    {
        return iconvert<wstring, UCS_4_INTERNAL, MACROMAN>(s);
    }
    vector<unsigned short> wstringToUniChars(const wstring& ws)
    {
        return iconvert<vector<unsigned short>, UCS_2_INTERNAL, UCS_4_INTERNAL>(ws);
    }
}

#endif
#else
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
wstring Gosu::utf8ToWstring(const string& utf8)
{
	vector<wchar_t> buffer(utf8.size() + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size() + 1, &buffer[0], buffer.size());
	return &buffer[0];
}	
string Gosu::wstringToUTF8(const wstring& ws)
{
	unsigned size = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), 0, 0, 0, 0);
	vector<char> buffer(size + 1);
	WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), ws.size(), &buffer[0], buffer.size(), 0, 0);
	return &buffer[0];
}
#endif

wstring Gosu::widen(const string& s)
{
#ifdef GOSU_IS_X
    setlocale(LC_ALL, "");
#endif

    size_t wideLen = std::mbstowcs(0, s.c_str(), 0);
    if (wideLen == static_cast<size_t>(-1))
        throw std::runtime_error("Could not convert from string to wstring: " + s);

    vector<wchar_t> buf(wideLen + 1);
    mbstowcs(&buf.front(), s.c_str(), buf.size());

    return wstring(buf.begin(), buf.end() - 1);
}

string Gosu::narrow(const wstring& ws)
{
    size_t narrowLen = std::wcstombs(0, ws.c_str(), 0);
    if (narrowLen == static_cast<size_t>(-1))
        throw std::runtime_error("Could not convert from wstring to string: " + string(ws.begin(), ws.end()));

    vector<char> buf(narrowLen + 1);
    wcstombs(&buf.front(), ws.c_str(), buf.size());

    return string(buf.begin(), buf.end() - 1);
}
