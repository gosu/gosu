#include <Gosu/Utility.hpp>
#include <Gosu/Platform.hpp>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <vector>
using namespace std;

#ifdef __APPLE__
// We want Apple's iconv
#include </usr/include/iconv.h>
#include <errno.h>
namespace
{
    extern const char UTF_8[] = "UTF-8";
    extern const char UCS_2_INTERNAL[] = "UCS-2-INTERNAL";
    extern const char UCS_4_INTERNAL[] = "UCS-4-INTERNAL";
    extern const char CHAR[] = "char";

    template<typename Out, const char* to, const char* from, typename In>
    Out iconvert(const In& in)
    {
        if (in.empty())
            return Out();
    
        const size_t bufferLen = 128;
        typedef typename In::value_type InElem;
        typedef typename Out::value_type OutElem;

        static iconv_t cd = iconv_open(to, from);
        
        Out result;
        OutElem buffer[bufferLen];
        
        char* inbuf = const_cast<char*>(reinterpret_cast<const char*>(&in[0]));
        size_t inbytesleft = in.size() * sizeof(InElem);
        char* outbuf = reinterpret_cast<char*>(buffer);
        size_t outbytesleft = sizeof buffer;
        
        for (;;)
        {
            size_t ret = ::iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
            if (ret == static_cast<size_t>(-1) && errno == EILSEQ)
            {
                // Skip illegal sequence part, repeat loop.
                // TODO: Or retry w/ different encoding?
                ++inbuf;
                --inbytesleft;
            }
            else if (ret == static_cast<size_t>(-1) && errno == E2BIG)
            {
                // Append new characters, reset out buffer, then repeat loop.
                result.insert(result.end(), buffer, buffer + bufferLen);
                outbuf = reinterpret_cast<char*>(buffer);
                outbytesleft = sizeof buffer;
            }
            else
            {
                // Append what's new in the buffer, then LEAVE loop.
                result.insert(result.end(), buffer, buffer + bufferLen - outbytesleft / sizeof(OutElem));
                return result;
            }
        }        
    }
}
wstring Gosu::utf8ToWstring(const string& s)
{
    return iconvert<wstring, UCS_4_INTERNAL, UTF_8>(s);
}
string Gosu::wstringToUTF8(const std::wstring& ws)
{
    return iconvert<string, UTF_8, UCS_4_INTERNAL>(ws);
}
// This is only necessary on OS X (for text output)
namespace Gosu {
vector<unsigned short> wstringToUniChars(const wstring& ws)
{
    return iconvert<vector<unsigned short>, UCS_2_INTERNAL, UCS_4_INTERNAL>(ws);
}
}
#elif defined(GOSU_IS_WIN)
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
#else
// On Linux, everything ought to be UTF8 anyway -- forward to locale widening/narrowing
std::wstring Gosu::utf8ToWstring(const std::string& utf8)
{
    return widen(utf8);
}
std::string Gosu::wstringToUTF8(const std::wstring& ws)
{
    return narrow(ws);
}
#endif

wstring Gosu::widen(const string& s)
{
#ifdef GOSU_IS_X
    setlocale(LC_ALL, "");
#endif

    size_t wideLen = std::mbstowcs(0, s.c_str(), 0);
    if (wideLen == static_cast<size_t>(-1))
        throw std::runtime_error("Could not convert from string to wstring");

    vector<wchar_t> buf(wideLen + 1);
    mbstowcs(&buf.front(), s.c_str(), buf.size());

    return wstring(buf.begin(), buf.end() - 1);
}

string Gosu::narrow(const wstring& ws)
{
    size_t narrowLen = std::wcstombs(0, ws.c_str(), 0);
    if (narrowLen == static_cast<size_t>(-1))
        throw std::runtime_error("Could not convert from wstring to string");

    vector<char> buf(narrowLen + 1);
    wcstombs(&buf.front(), ws.c_str(), buf.size());

    return string(buf.begin(), buf.end() - 1);
}
