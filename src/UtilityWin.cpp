#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/Utility.hpp>
#include "WinUtility.hpp"
#include <windows.h>

std::string Gosu::language()
{
    LCID lcid = GetUserDefaultLCID();
    char buffer[9];
    winapi_check(GetLocaleInfoA(lcid, LOCALE_SISO639LANGNAME, buffer, sizeof buffer));
    return buffer;
}

#endif
