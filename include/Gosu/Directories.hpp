#pragma once

#include <string>

namespace Gosu
{
    /// Changes the current directory to the result of resource_prefix().
    void use_resource_directory();

    /// Builds a filename for reading (not writing) game resources.
    /// An image file can be loaded as e.g. Gosu::Image(Gosu::resource_path("gfx/image.png")).
    ///
    /// This function will use the following directory:
    /// <ul>
    /// <li> Windows: The parent directory of the executable.
    /// <li> macOS, iOS: The 'Resources' directory inside the .app bundle, or the current directory
    ///      if this code is not being run from an .app bundle.
    /// <li> Linux: The current directory (empty string).
    /// </ul>
    ///
    /// Note: This uses SDL_GetBasePath internally. https://wiki.libsdl.org/SDL2/SDL_GetBasePath
    std::string resource_path(const std::string& relative_filename);

    /// Builds a filename for reading or writing game settings. This method will automatically
    /// create all intermediate directories needed to save files at the returned path.
    ///
    /// Preferences might be saved using e.g.:
    /// Gosu::save_file(buffer, Gosu::user_settings_path("MyStudio", "MyGame", "settings.ini")).
    ///
    /// This function will use the following directory:
    /// <ul>
    /// <li> Windows: The "organization/application" folder in %APPDATA%.
    /// <li> macOS: "~/Library/Application Support/organization/application".
    /// <li> Linux: "~.local/share/application".
    /// </ul>
    ///
    /// Note: This uses SDL_GetPrefPath internally. https://wiki.libsdl.org/SDL2/SDL_GetPrefPath
    std::string user_settings_path(const std::string& organization, const std::string& application,
                                   const std::string& relative_filename);
}
