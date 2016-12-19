#include <Gosu/Directories.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>

namespace 
{
    std::wstring home_dir()
    {   
        passwd* pwd = getpwuid(geteuid());
        assert(pwd);
        return Gosu::widen(pwd->pw_dir);
    }
}

void Gosu::use_resource_directory()
{
    // Do nothing - the current directory HAS to be correct on Linux.
}

std::wstring Gosu::resource_prefix()
{
    return std::wstring();
}

std::wstring Gosu::shared_resource_prefix()
{
    return std::wstring();
}

std::wstring Gosu::user_settings_prefix()
{
    return home_dir() + L"/.";
}

std::wstring Gosu::user_documents_prefix()
{
    return home_dir() + L"/";
}
