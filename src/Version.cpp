#include <Gosu/Version.hpp>
#include <Gosu/Platform.hpp>
#include <string>
using namespace std;

const string Gosu::VERSION = to_string(GOSU_MAJOR_VERSION) + '.' +
                             to_string(GOSU_MINOR_VERSION) + '.' +
                             to_string(GOSU_POINT_VERSION);

const string Gosu::LICENSES =
    "This software may utilize code from the following third-party libraries:\n"
    "\n"
    "Gosu, https://www.libgosu.org, MIT License, https://opensource.org/licenses/MIT\n"
    "SDL 2, https://www.libsdl.org, MIT License, https://opensource.org/licenses/MIT\n"
#if defined(GOSU_IS_WIN) || defined(GOSU_IS_X)
    "libsndfile, http://www.mega-nerd.com/libsndfile, GNU LGPL 3, "
        "https://www.gnu.org/copyleft/lesser.html\n"
    "mpg123, https://mpg123.de, GNU LGPL 3, "
        "https://www.gnu.org/copyleft/lesser.html\n"
#endif
#if defined(GOSU_IS_WIN)
    "OpenAL Soft, http://kcat.strangesoft.net/openal.html, GNU LGPL 2, "
        "https://www.gnu.org/licenses/old-licenses/lgpl-2.0.html\n"
#endif
    ;
