#include <Gosu/Directories.hpp>
#include "WinUtility.hpp"
#include <cwchar>
#include <stdexcept>
#include <shlobj.h>

namespace
{
    std::wstring specialFolderPath(int csidl)
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

    std::wstring exeFilename()
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

void Gosu::useResourceDirectory()
{
    SetCurrentDirectory(resourcePrefix().c_str());
}

std::wstring Gosu::resourcePrefix()
{
    static std::wstring result;
    if (result.empty()) {
        result = exeFilename();
        std::wstring::size_type lastDelim = result.find_last_of(L"\\/");
        if (lastDelim != std::wstring::npos)
            result.resize(lastDelim + 1);
        else
            result = L"";
    }
    return result;
}

std::wstring Gosu::sharedResourcePrefix()
{
    return resourcePrefix();
}

std::wstring Gosu::userSettingsPrefix()
{
    return specialFolderPath(CSIDL_APPDATA);
}

std::wstring Gosu::userDocsPrefix()
{
    return specialFolderPath(CSIDL_PERSONAL);
}
