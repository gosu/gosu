#include <Gosu/Utility.hpp>
#include <Gosu/WinUtility.hpp>
#include <windows.h>
using namespace std;

string Gosu::language()
{
    LCID lcid = GetUserDefaultLCID();
    char buffer[9];
    Win::check(
        GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, buffer, sizeof buffer));
    return buffer;
}
