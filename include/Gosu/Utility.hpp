//! \file Utility.hpp
//! General purpose utility functions.

#pragma once

#include <string>

namespace Gosu
{
    std::u32string utf8_to_composed_utc4(const std::string& utf8);
    
    //! Returns true if the filename has the given extension.
    //! The comparison is case-insensitive, but you must pass the extension in lower case.
    bool has_extension(const std::string& filename, const char* extension);
    
    //! Returns the user's preferred language, at the moment of calling the function. Expect return
    //! values such as 'en_US', 'de_DE.UTF-8', 'ja', 'zh-Hans'.
    //! The first two letters will always be a language code.
    std::string language();
}
