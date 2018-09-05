#pragma once

#include <Gosu/Platform.hpp>
#include <string>

namespace Gosu
{
    //! Converts UTF-8 to UTF-16, as it is used in Windows APIs.
    std::wstring utf8_to_utf16(const std::string& utf8);
    //! Converts UTF-16 to UTF-8.
    std::string utf16_to_utf8(const std::wstring& utf16);

    //! Throws an exception according to the error returned by  GetLastError(), optionally prefixed
    //! with "While (action), the following error occured: ".
    GOSU_NORETURN void throw_last_winapi_error(const std::string& action = "");

    //! Small helper function that throws the last Windows error when val_to_check is false.
    template<typename T>
    inline T winapi_check(T val_to_check, const std::string& action = "")
    {
        if (!val_to_check) {
            throw_last_winapi_error(action);
        }
        return val_to_check;
    }
}
