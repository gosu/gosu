#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include "WinUtility.hpp"
#include <Gosu/Utility.hpp>
#include <stdexcept>
#include <windows.h>
using namespace std;

wstring Gosu::utf8_to_utf16(const string& utf8)
{
    wstring utf16(utf8.size(), '\0');
    auto len = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), utf8.size(),
                                   const_cast<wchar_t*>(utf16.data()), utf16.size());
    utf16.resize(len);
    return utf16;
}

string Gosu::utf16_to_utf8(const wstring& utf16)
{
    auto len = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), utf16.size(),
                                   nullptr, 0, nullptr, nullptr);
    string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), utf16.size(),
                        const_cast<char*>(utf8.data()), utf8.size(), nullptr, nullptr);
    return utf8;
}

void Gosu::throw_last_winapi_error(const string& action)
{
    // Obtain error message from Windows.
    wchar_t* buffer;

    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, GetLastError(),
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buffer, 0, nullptr)
            || buffer == nullptr) {
        throw runtime_error("Unknown error");
    }

    // Safely move the message into a string.
    string message;
    try {
        message = utf16_to_utf8(buffer);
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
