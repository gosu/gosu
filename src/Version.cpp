#include <Gosu/Version.hpp>
#include <Gosu/Platform.hpp>
#include <string>

const std::string Gosu::VERSION = std::to_string(GOSU_MAJOR_VERSION) + '.' +
                                  std::to_string(GOSU_MINOR_VERSION) + '.' +
                                  std::to_string(GOSU_POINT_VERSION);

const std::string Gosu::LICENSES =
    "This software may utilize code from the following third-party libraries:\n"
    "\n"
    "Gosu, https://www.libgosu.org, MIT License, http://opensource.org/licenses/MIT\n"
    "SDL 2, http://www.libsdl.org, MIT License, http://opensource.org/licenses/MIT\n"
#if defined(GOSU_IS_WIN) || defined(GOSU_IS_X)
    "libsndfile, http://www.mega-nerd.com/libsndfile, GNU LGPL 3, "
        "http://www.gnu.org/copyleft/lesser.html\n"
#endif
#if defined(GOSU_IS_WIN)
    "OpenAL Soft, http://kcat.strangesoft.net/openal.html, GNU LGPL 2, "
        "http://www.gnu.org/licenses/old-licenses/lgpl-2.0.html\n"
#endif
    ;
