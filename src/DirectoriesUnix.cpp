#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_X)

#include <Gosu/Directories.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

static std::string home_dir()
{
    passwd* pwd = getpwuid(geteuid());
    assert (pwd && pwd->pw_dir);
    return pwd->pw_dir;
}

void Gosu::use_resource_directory()
{
    // Do nothing, we expect the user to have the correct cwd on Linux.
}

std::string Gosu::resource_prefix()
{
    return std::string();
}

std::string Gosu::shared_resource_prefix()
{
    return std::string();
}

std::string Gosu::user_settings_prefix()
{
    return home_dir() + "/.";
}

std::string Gosu::user_documents_prefix()
{
    return home_dir() + "/";
}

#endif
