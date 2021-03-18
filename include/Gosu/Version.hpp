#pragma once

#include <string>

#define GOSU_MAJOR_VERSION 1
#define GOSU_MINOR_VERSION 2
#define GOSU_POINT_VERSION 0

namespace Gosu
{
    //! A string that contains the full version of the Gosu library, like "0.7.50" or "1.0.0.pre5".
    extern const std::string VERSION;
    
    //! A block of legal copy that your game is obliged to display somewhere.
    extern const std::string LICENSES;
}
