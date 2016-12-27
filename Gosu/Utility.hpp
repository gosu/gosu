//! \file Utility.hpp
//! General purpose utility functions.

#pragma once

#include <string>

namespace Gosu
{
    //! Converts an UTF-8 to UCS-4 or UTF-16, depending on the platform's interpretation of wstring.
    std::wstring utf8_to_wstring(const std::string& utf8);
    //! Converts an UCS-4 or UTF-16 to UTF-8, depending on the platform's interpretation of wstring.
    std::string wstring_to_utf8(const std::wstring& ws);
    
    //! Returns true if the filename has the given extension.
    //! The comparison is case-insensitive, but you must pass the extension in lower case.
    bool has_extension(const std::string& filename, const char* extension);
    
    //! Returns the user's preferred language, at the moment of calling the function. Expect return
    //! values such as 'en_US', 'de_DE.UTF-8', 'ja', 'zh-Hans'.
    //! The first two letters will always be a language code.
    std::string language();
}
