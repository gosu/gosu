#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include "WinUtility.hpp"
#include <Gosu/Directories.hpp>
#include <Gosu/Utility.hpp>
#include <cwchar>
#include <stdexcept>
#include <shlobj.h>

static std::string special_folder_path(int csidl)
{
    WCHAR buf[MAX_PATH + 2];
    if (FAILED(SHGetFolderPathW(nullptr, csidl | CSIDL_FLAG_CREATE, nullptr, 0, buf))) {
        throw std::runtime_error("Error getting special folder path");
    }
    std::size_t len = std::wcslen(buf);
    if (buf[len - 1] != L'\\') {
        buf[len] = L'\\';
        buf[len + 1] = 0;
    }
    return Gosu::wstring_to_utf8(buf);
}

static std::string exe_filename()
{
    static std::string result;
    if (result.empty()) {
        WCHAR buffer[MAX_PATH * 2];
        Gosu::winapi_check(GetModuleFileNameW(nullptr, buffer, MAX_PATH * 2),
                           "getting the module filename");
        result = Gosu::wstring_to_utf8(buffer);
    }
    return result;
}

void Gosu::use_resource_directory()
{
    SetCurrentDirectory(utf8_to_wstring(resource_prefix()).c_str());
}

std::string Gosu::resource_prefix()
{
    static std::string result;
    if (result.empty()) {
        result = exe_filename();
        auto last_delim = result.find_last_of("\\/");
        result.resize(last_delim == result.npos ? 0 : last_delim + 1);
    }
    return result;
}

std::string Gosu::shared_resource_prefix()
{
    return resource_prefix();
}

std::string Gosu::user_settings_prefix()
{
    return special_folder_path(CSIDL_APPDATA);
}

std::string Gosu::user_documents_prefix()
{
    return special_folder_path(CSIDL_PERSONAL);
}

#endif
