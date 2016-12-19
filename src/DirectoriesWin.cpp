#include <Gosu/Directories.hpp>
#include "WinUtility.hpp"
#include <cwchar>
#include <stdexcept>
#include <shlobj.h>

namespace
{
    std::wstring special_folder_path(int csidl)
    {
        WCHAR buf[MAX_PATH + 2];
        if (FAILED(SHGetFolderPath(NULL, csidl | CSIDL_FLAG_CREATE, NULL, 0, buf)))
            throw std::runtime_error("Error getting special folder path");
        std::size_t len = std::wcslen(buf);
        if (buf[len - 1] != L'\\')
        {
            buf[len] = L'\\';
            buf[len + 1] = 0;
        }
        return buf;
    }

    std::wstring exe_filename()
    {
        static std::wstring result;
        if (!result.empty())
            return result;

        wchar_t buffer[MAX_PATH * 2];
        Gosu::Win::check(::GetModuleFileName(0, buffer, MAX_PATH * 2),
            "getting the module filename");
        result = buffer;
        return result;
    }
}

void Gosu::use_resource_directory()
{
    SetCurrentDirectory(resource_prefix().c_str());
}

std::wstring Gosu::resource_prefix()
{
    static std::wstring result;
    if (result.empty()) {
        result = exe_filename();
        std::wstring::size_type last_delim = result.find_last_of(L"\\/");
        if (last_delim != std::wstring::npos)
            result.resize(last_delim + 1);
        else
            result = L"";
    }
    return result;
}

std::wstring Gosu::shared_resource_prefix()
{
    return resource_prefix();
}

std::wstring Gosu::user_settings_prefix()
{
    return special_folder_path(CSIDL_APPDATA);
}

std::wstring Gosu::user_documents_prefix()
{
    return special_folder_path(CSIDL_PERSONAL);
}
