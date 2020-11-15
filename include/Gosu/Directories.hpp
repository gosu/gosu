//! \file Directories.hpp
//! Access to a small set of system paths.

#pragma once

#include <string>

namespace Gosu
{
    //! Changes the current directory to the result of resource_prefix().
    void use_resource_directory();
    
    //! Prefix for a program's own resources.
    //! On Windows, the parent directory of the executable.
    //! On macOS, the 'Resources' directory inside the .app bundle.
    //! On Linux, the current directory (empty string).
    std::string resource_prefix();
    
    //! Prefix for shared resources of a group of programs.
    //! On Windows, the parent directory of the executable.
    //! On macOS, the parent directory of the .app bundle.
    //! On Linux, the current directory (empty string).
    std::string shared_resource_prefix();
    
    //! Prefix for user settings.
    //! On Windows, the same as %APPDATA%.
    //! On macOS, the user's Library/Preferences folder.
    //! On Linux, the home directory plus a trailing dot for hidden files.
    std::string user_settings_prefix();
    
    //! Prefix for user documents, e.g. saved games.
    //! On Windows, the My Documents folder.
    //! On macOS, the user's Documents folder.
    //! On Linux, the home directory.
    std::string user_documents_prefix();
}
