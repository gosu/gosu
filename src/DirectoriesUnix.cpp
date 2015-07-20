#include <Gosu/Directories.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

namespace 
{
    std::wstring homeDir()
    {   
        passwd* pwd = getpwuid(geteuid());
        assert(pwd);
        return Gosu::widen(pwd->pw_dir);
    }
}

void Gosu::useResourceDirectory()
{
    // Do nothing - the current directory HAS to be correct on Linux.
}

std::wstring Gosu::resourcePrefix()
{
    return std::wstring();
}

std::wstring Gosu::sharedResourcePrefix()
{
    return std::wstring();
}

std::wstring Gosu::userSettingsPrefix()
{
    return homeDir() + L"/.";
}

std::wstring Gosu::userDocsPrefix()
{
    return homeDir() + L"/";
}
