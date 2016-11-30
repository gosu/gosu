#include <Gosu/Utility.hpp>
#include "WinUtility.hpp"
#include <windows.h>
using namespace std;

string Gosu::language()
{
#if defined(GOSU_IS_UWP)
	// this is ugly, since Gosu expects ansi and windows unicode
	wchar_t buffer[LOCALE_NAME_MAX_LENGTH];
	::GetUserDefaultLocaleName((LPWSTR)&buffer, LOCALE_NAME_MAX_LENGTH);
	std::wstring unicode = wstring(buffer);
	std::string ret(unicode.begin(), unicode.end());
	return ret;
#else
    LCID lcid = GetUserDefaultLCID();
    char buffer[9];
    Win::check(
        GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, buffer, sizeof buffer));
    return buffer;
#endif
}
