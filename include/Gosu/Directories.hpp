#pragma once

#include <string>

namespace Gosu
{
    /// Changes the current directory to the result of resource_prefix().
    void use_resource_directory();
    
    /// Prefix for a program's own resources.
    /// <ul>
    /// <li> Windows: The parent directory of the executable.
    /// <li> macOS, iOS: The 'Resources' directory inside the .app bundle.
    /// <li> Linux: The current directory (empty string).
    /// </ul>
    const std::string& resource_prefix();
    
    /// Prefix for shared resources of a group of programs, e.g. the game and its level editor.
    /// <ul>
    /// <li> Windows: The parent directory of the executable.
    /// <li> macOS: The parent directory of the .app bundle.
    /// <li> Linux: The current directory (empty string).
    /// </uil>
    const std::string& shared_resource_prefix();
    
    /// Prefix for user settings.
    /// <ul>
    /// <li> Windows: The same as %APPDATA%.
    /// <li> macOS: The user's Library/Preferences folder.
    /// <li> Linux: The home directory plus a trailing dot for hidden files.
    /// </ul>
    const std::string& user_settings_prefix();
    
    /// Prefix for user documents, e.g. saved games.
    /// <ul>
    /// <li> Windows: The "My Documents" folder.
    /// <li> macOS: The user's "Documents" folder.
    /// <li> Linux: The home directory.
    /// </ul>
    const std::string& user_documents_prefix();

    // TODO: Instead of re-inventing wheels here, we should wrap SDL_GetBasePath / SDL_GetPrefPath.
}
