#include <Gosu/Directories.hpp>
#include "WinUtility.hpp"
#include <cwchar>

#if defined(GOSU_IS_UWP)
#include <SDL.h>
#else
#include <stdexcept>
#include <shlobj.h>
#endif

namespace
{
    std::wstring specialFolderPath(int csidl)
    {
#if defined(GOSU_IS_UWP)
		std::wstring result = L".";
		return result;
#else
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
#endif
    }

    std::wstring exeFilename()
    {
#if defined(GOSU_IS_UWP)
		std::wstring result = L"";
		return result;
#else
        static std::wstring result;
        if (!result.empty())
            return result;

        wchar_t buffer[MAX_PATH * 2];
        Gosu::Win::check(::GetModuleFileName(0, buffer, MAX_PATH * 2),
            "getting the module filename");
        result = buffer;
        return result;
#endif
    }
}

void Gosu::useResourceDirectory()
{
#if !defined(GOSU_IS_UWP)
    SetCurrentDirectory(resourcePrefix().c_str());
#endif
}

std::wstring Gosu::resourcePrefix()
{
    static std::wstring result;
#if defined(GOSU_IS_UWP)
	const wchar_t* basePath = SDL_WinRTGetFSPathUNICODE(SDL_WINRT_PATH_INSTALLED_LOCATION);
	result = std::wstring(basePath);
#else
    if (result.empty()) {
        result = exeFilename();
        std::wstring::size_type lastDelim = result.find_last_of(L"\\/");
        if (lastDelim != std::wstring::npos)
            result.resize(lastDelim + 1);
        else
            result = L"";
    }
#endif
    return result;
}

std::wstring Gosu::sharedResourcePrefix()
{
    return resourcePrefix();
}

std::wstring Gosu::userSettingsPrefix()
{
#if defined(GOSU_IS_UWP)
	std::wstring result;
	const wchar_t* localFolder = SDL_WinRTGetFSPathUNICODE(SDL_WINRT_PATH_LOCAL_FOLDER); // roaming folder maybe?
	result = std::wstring(localFolder);
	return result;
#else
    return specialFolderPath(CSIDL_APPDATA);
#endif
}

std::wstring Gosu::userDocsPrefix()
{
#if defined(GOSU_IS_UWP)
	std::wstring result;
	const wchar_t* localFolder = SDL_WinRTGetFSPathUNICODE(SDL_WINRT_PATH_LOCAL_FOLDER);
	result = std::wstring(localFolder);
	return result;
#else
    return specialFolderPath(CSIDL_PERSONAL);
#endif
}
