#pragma once

#include <Gosu/Platform.hpp>
#include <string>

namespace Gosu
{
    //! Implementation helpers for the Windows platform.
    namespace Win
    {
        //! Throws an exception according to the error which GetLastError()
        //! returns, optionally prefixed with "While (action), the following
        //! error occured: ".
        GOSU_NORETURN void throwLastError(const std::string& action = "");

        //! Small helper function that throws an exception whenever the value
        //! passed through is false. Note that this doesn't make sense for all
        //! Windows API functions, but for most of them.
        template<typename T>
        inline T check(T valToCheck, const std::string& action = "")
        {
            if (!valToCheck)
                throwLastError(action);
            return valToCheck;
        }
    }
}
