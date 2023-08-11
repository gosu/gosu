#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/Utility.hpp>
#include <stdexcept>
#include <windows.h>

std::wstring Gosu::utf8_to_utf16(const std::string& utf8)
{
    if (utf8.empty()) {
        return std::wstring();
    }

    // The UTF-16 result can never consist of more codepoints than we have bytes in UTF-8.
    // -> Start with a size that we know is sufficient, then truncate later.
    std::wstring utf16(utf8.size(), '\0');
    int result = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8.data(), utf8.size(),
                                     utf16.data(), utf16.size());
    if (result == 0) {
        throw std::runtime_error("Could not convert UTF-8 to UTF-16, GetLastError() = "
                                 + std::to_string(GetLastError()));
    }
    utf16.resize(result);
    return utf16;
}

std::string Gosu::utf16_to_utf8(const std::wstring& utf16)
{
    if (utf16.empty()) {
        return std::string();
    }

    int result = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), utf16.size(), nullptr, 0, nullptr,
                                     nullptr);
    if (result == 0) {
        throw std::runtime_error("Could not convert UTF-16 to UTF-8, GetLastError() = "
                                 + std::to_string(GetLastError()));
    }
    std::string utf8(result, '\0');
    WideCharToMultiByte(CP_UTF8, MB_ERR_INVALID_CHARS, utf16.c_str(), utf16.size(), utf8.data(),
                        utf8.size(), nullptr, nullptr);
    return utf8;
}

#endif
