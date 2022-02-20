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
    return pwd && pwd->pw_dir ? pwd->pw_dir : ".";
}

void Gosu::use_resource_directory()
{
    // Do nothing, we expect the user to have the correct cwd on Linux.
}

const std::string& Gosu::resource_prefix()
{
    static const std::string empty;
    return empty;
}

const std::string& Gosu::shared_resource_prefix()
{
    static const std::string empty;
    return empty;
}

const std::string& Gosu::user_settings_prefix()
{
    static const std::string user_settings_prefix = home_dir() + "/.";
    return user_settings_prefix;
}

const std::string& Gosu::user_documents_prefix()
{
    static const std::string user_documents_prefix = home_dir() + "/";
    return user_documents_prefix;
}

#endif
