#include <Gosu/Utility.hpp>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <vector>
using namespace std;

#ifdef __APPLE__
#include <iconv.h>
wstring Gosu::utf8ToWstring(const string& s)
{
    if (s.empty())
        return L"";

    static iconv_t cd = iconv_open("UCS-4-INTERNAL", "UTF-8");
    
    vector<wchar_t> result(s.length());
    
    const char* inbuf = reinterpret_cast<const char*>(s.data());
    size_t inbytesleft = s.size() * sizeof s[0];
    char* outbuf = reinterpret_cast<char*>(&result[0]);
    size_t outbytesleft = result.size() * sizeof result[0];
    size_t res = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    if (res == static_cast<size_t>(-1))
    switch (errno) {
        case EILSEQ: throw std::runtime_error("EILSEQ");
        case EINVAL: throw std::runtime_error("EINVAL");
        case E2BIG: throw std::runtime_error("E2BIG");
        default: throw std::runtime_error(":(");
    }
    
    result.resize(result.size() - outbytesleft / 4);
        
    return wstring(result.begin(), result.end());
}

string Gosu::wstringToUTF8(const std::wstring& ws)
{
    if (ws.empty())
        return string();

    static iconv_t cd = iconv_open("UTF-8", "UCS-4-INTERNAL");
    
    vector<char> result(ws.length() * 10); // TODO: arbitrary, correct!
    
    const char* inbuf = reinterpret_cast<const char*>(ws.data());
    size_t inbytesleft = ws.size() * sizeof ws[0];
    char* outbuf = reinterpret_cast<char*>(&result[0]);
    size_t outbytesleft = result.size() * sizeof result[0];
    size_t res = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    if (res == static_cast<size_t>(-1))
    switch (errno) {
        case EILSEQ: throw std::runtime_error("EILSEQ");
        case EINVAL: throw std::runtime_error("EINVAL");
        case E2BIG: throw std::runtime_error("E2BIG");
        default: throw std::runtime_error(":(");
    }
    
    result.resize(result.size() - outbytesleft);
    
    return string(result.begin(), result.end());
}

// This is only necessary on OS X
namespace Gosu {
vector<unsigned short> wstringToUniChars(const wstring& ws)
{
    if (ws.empty())
        return vector<unsigned short>();

    static iconv_t cd = iconv_open("UCS-2-INTERNAL", "UCS-4-INTERNAL");
    
    vector<unsigned short> result(ws.length());
    
    const char* inbuf = reinterpret_cast<const char*>(ws.data());
    size_t inbytesleft = ws.size() * sizeof ws[0];
    char* outbuf = reinterpret_cast<char*>(&result[0]);
    size_t outbytesleft = result.size() * sizeof result[0];
    size_t res = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
    
    if (res == static_cast<size_t>(-1))
    switch (errno) {
        case EILSEQ: throw std::runtime_error("EILSEQ");
        case EINVAL: throw std::runtime_error("EINVAL");
        case E2BIG: throw std::runtime_error("E2BIG");
        default: throw std::runtime_error(":(");
    }
    
    result.resize(result.size() - outbytesleft / 2);
    
    return result;
}
}
#else
std::wstring Gosu::utf8ToWString(const std::string& utf8)
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
    setlocale(LC_ALL, "utf8");
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
