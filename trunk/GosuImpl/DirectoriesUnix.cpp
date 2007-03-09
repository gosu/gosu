#include <Gosu/Directories.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

namespace 
{
    std::wstring homeDir()
    {   
        // TODO: Would geteuid() be a better choice? I'm no UNIX guru.
        passwd* pwd = getpwuid(getuid());
        assert(pwd);
        return Gosu::widen(pwd->pw_dir);
    }
}

#ifndef GOSU_IS_MAC
std::wstring Gosu::resourcePrefix()
{
    return std::wstring();
}

std::wstring Gosu::sharedResourcePrefix()
{
    return std::wstring();
}
#endif

std::wstring Gosu::userSettingsPrefix()
{
#ifdef GOSU_IS_MAC
    return homeDir() + L"/Library/";
#else
    return homeDir() + L"/.";
#endif
}

std::wstring Gosu::userDocsPrefix()
{
#ifdef GOSU_IS_MAC
    return homeDir() + L"/Documents/";
#else
    return homeDir() + L"/";
#endif
}
