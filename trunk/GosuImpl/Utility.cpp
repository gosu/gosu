#include <Gosu/Utility.hpp>
#include <cstddef>
#include <cstdlib>
#include <stdexcept>
#include <vector>
using namespace std;

wstring Gosu::widen(const string& s)
{
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
