#include <Gosu/Utility.hpp>
#include <Gosu/WinUtility.hpp>
#include <windows.h>
using namespace std;

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

string Gosu::language()
{
    LCID lcid = GetUserDefaultLCID();
    char buffer[9];
    Win::check(
        GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, buffer, sizeof buffer));
    return buffer;
}
