//! \file Directories.hpp
//! Access to a small set of system paths.

#pragma once

#include <string>

namespace Gosu
{
    //! Changes the current directory to the result of resource_prefix().
    void use_resource_directory();
    
    //! Prefix for a program's own resources.
    //! On Windows, the executable's containing directory.
    //! On OS X, the application's Resources subdirectory.
    //! On Linux, the current directory (empty string).
    std::wstring resource_prefix();
    
    //! Prefix for resources of a group of programs.
    //! On Windows, the executable's containing directory.
    //! On OS X, the application's containing subdirectory.
    //! On Linux, the current directory (empty string).
    std::wstring shared_resource_prefix();
    
    //! Prefix for user settings.
    //! On Windows, the same as %APPDATA%.
    //! On OS X, the user's Library/Preferences folder.
    //! On Linux, the home directory plus a trailing dot for hidden files.
    std::wstring user_settings_prefix();
    
    //! Prefix for user documents, e.g. saved games.
    //! On Windows, the My Documents folder.
    //! On OS X, the user's Documents folder.
    //! On Linux, the home directory.
    std::wstring user_documents_prefix();
}
