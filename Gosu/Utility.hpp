//! \file Utility.hpp
//! General purpose utility functions.

// REDESIGN: Rename to StringConversion.hpp?

#ifndef GOSU_UTILITY_HPP
#define GOSU_UTILITY_HPP

#include <string>
#include <vector>

namespace Gosu
{
    //! Converts an std::string into an std::wstring.
    std::wstring utf8ToWstring(const std::string& utf8);
    //! Converts an std::wstring into an std::string.
    std::string wstringToUTF8(const std::wstring& ws);

    //! Converts an std::string into an std::wstring using local encoding.
    std::wstring widen(const std::string& s);
    //! Converts an std::wstring into an std::string using local encoding.
    std::string narrow(const std::wstring& ws);
}

#endif
