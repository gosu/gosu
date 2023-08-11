#include <Gosu/Directories.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>

#ifdef GOSU_IS_WIN
#include <windows.h>
#else
#include <unistd.h>
#endif

#ifndef GOSU_IS_IPHONE
#include <SDL.h>
#include <memory>
#endif

void Gosu::use_resource_directory()
{
#ifdef GOSU_IS_WIN
    SetCurrentDirectoryW(utf8_to_utf16(resource_path("")).c_str());
#else
    chdir(resource_path("").c_str());
#endif
}

#ifndef GOSU_IS_IPHONE
std::string Gosu::resource_path(const std::string& relative_filename)
{
    static const char* resource_prefix = SDL_GetBasePath(); // never freed, doesn't matter
    return relative_filename.empty() ? resource_prefix : resource_prefix + relative_filename;
}

std::string Gosu::user_settings_path(const std::string& organization,
                                     const std::string& application,
                                     const std::string& relative_filename)
{
    char* settings_prefix = SDL_GetPrefPath(organization.c_str(), application.c_str());
    if (!settings_prefix) {
        throw std::runtime_error("Could not create settings directory: "
                                 + std::string(SDL_GetError()));
    }
    const std::unique_ptr<char, decltype(&SDL_free)> guard(settings_prefix, &SDL_free);
    return relative_filename.empty() ? settings_prefix : settings_prefix + relative_filename;
}

#endif
