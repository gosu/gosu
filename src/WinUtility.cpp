#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include "WinUtility.hpp"
#include <Gosu/Utility.hpp>
#include <stdexcept>
#include <windows.h>
using namespace std;

void Gosu::throw_last_winapi_error(const string& action)
{
    // Obtain error message from Windows.
    wchar_t* buffer;
                 
    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS, 0, GetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, 0)
            || buffer == nullptr) {
        throw runtime_error("Unknown error");
    }

    // Safely move the message into a string.
    string message;
    try {
        message = wstring_to_utf8(buffer);
    }
    catch (...) {
        LocalFree(buffer);
        throw;
    }
    LocalFree(buffer);
    
    // Optionally prepend the action.
    if (!action.empty()) {
        message = "While " + action + ", the following error occured: " + message;
    }

    throw runtime_error(message);
}

#endif
