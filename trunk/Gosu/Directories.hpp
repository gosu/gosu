//! \file Directories.hpp
//! Access to a small set of system paths.

#ifndef GOSU_DIRECTORIES_HPP
#define GOSU_DIRECTORIES_HPP

#include <string>

namespace Gosu
{
    //! Prefix for a program's own resources.
    //! On Windows, the executable's containing directory.
    //! On OS X, the application's Resources subdirectory.
    //! On Linux, the current directory.
    std::wstring resourcePrefix();
    
    //! Prefix for resources of a group of programs.
    //! On Windows, the executable's containing directory.
    //! On OS X, the application's containing subdirectory.
    //! On Linux, the current directory.
    std::wstring sharedResourcePrefix();
    
    //! Prefix for user settings.
    //! On Windows, the same as %APPDATA%.
    //! On OS X, the user's Library/Preferences folder.
    //! On Linux, the home directory plus a trailing dot for hidden files.
    std::wstring userSettingsPrefix();
    
    //! Prefix for user documents, e.g. save games.
    //! On Windows, the My Documents folder.
    //! On OS X, the user's Documents folder.
    //! On Linux, the home directory.
    std::wstring userDocsPrefix();
}

#endif
