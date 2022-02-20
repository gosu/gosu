#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/Directories.hpp>
#include <Gosu/Utility.hpp>
#include "WinUtility.hpp"
#include <cwchar>
#include <shlobj.h>
#include <stdexcept>

static std::string special_folder_path(int csidl)
{
    WCHAR buf[MAX_PATH + 2];
    if (FAILED(SHGetFolderPathW(nullptr, csidl | CSIDL_FLAG_CREATE, nullptr, 0, buf)))
        throw std::runtime_error{"Error getting special folder path"};
    size_t len = std::wcslen(buf);
    if (buf[len - 1] != L'\\') {
        buf[len] = L'\\';
        buf[len + 1] = 0;
    }
    return Gosu::utf16_to_utf8(buf);
}

std::string exe_filename()
{
    WCHAR buffer[MAX_PATH * 2];
    Gosu::winapi_check(GetModuleFileNameW(nullptr, buffer, MAX_PATH * 2),
                       "getting the module filename");
    return Gosu::utf16_to_utf8(buffer);
}

void Gosu::use_resource_directory()
{
    SetCurrentDirectory(utf8_to_utf16(resource_prefix()).c_str());
}

const std::string& Gosu::resource_prefix()
{
    static const std::string resource_prefix = [] {
        std::string result = exe_filename();
        const auto last_delim = result.find_last_of("\\/");
        result.resize(last_delim == std::string::npos ? 0 : last_delim + 1);
        return result;
    }();
    return resource_prefix;
}

const std::string& Gosu::shared_resource_prefix()
{
    return resource_prefix();
}

const std::string& Gosu::user_settings_prefix()
{
    static const std::string user_settings_prefix = special_folder_path(CSIDL_APPDATA);
    return user_settings_prefix;
}

const std::string& Gosu::user_documents_prefix()
{
    static const std::string user_documents_prefix = special_folder_path(CSIDL_PERSONAL);
    return user_documents_prefix;
}

#endif
