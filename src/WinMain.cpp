#include <windows.h>
#include <exception>
#include <string>
#include <vector>
#include <Gosu/platform.hpp>

#define SDL_MAIN_HANDLED

#if defined(GOSU_IS_UWP)
# include "SDL_main.h"
# include <wrl.h>

# ifndef SDL_WINRT_METADATA_FILE_AVAILABLE
#  ifndef __cplusplus_winrt
#   error SDL_winrt_main_NonXAML.cpp must be compiled with /ZW, otherwise build errors due to missing .winmd files can occur.
#  endif
#endif

# ifdef _MSC_VER
#  pragma warning(disable:4447)
# endif

/* Make sure the function to initialize the Windows Runtime gets linked in. */
# ifdef _MSC_VER
#  pragma comment(lib, "runtimeobject.lib")
# endif
#endif

using namespace std;

vector<string> splitCmdLine()
{
    vector<string> result;

    const char* cmdLine = ::GetCommandLineA();

    const char* argBegin = 0;
    bool isQuotedArg = false;

    while (*cmdLine)
    {
        if (*cmdLine == '"')
        {
            if (argBegin == 0)
            {
                argBegin = cmdLine + 1;
                isQuotedArg = true;
            }
            else if (isQuotedArg)
            {
                result.push_back(std::string(argBegin, cmdLine));
                argBegin = 0;
            }
        }
        else if (!isspace((unsigned char)*cmdLine) && argBegin == 0)
        {
            argBegin = cmdLine;
            isQuotedArg = false;
        }
        else if (isspace((unsigned char)*cmdLine) && argBegin != 0 && !isQuotedArg)
        {
            result.push_back(std::string(argBegin, cmdLine + 1));
            argBegin = 0;
        }
        ++cmdLine;
    }

    if (argBegin != 0)
        result.push_back(argBegin);

    return result;
}

int main(int argc, char* argv[]);

// if its Win32, use the normal WinMain
// if its UWP then use SDL_WinRTRunApp
#if !defined(GOSU_IS_UWP)

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try
    {
        vector<string> arguments = splitCmdLine();
        vector<char*> argv(arguments.size());
        for (unsigned i = 0; i < argv.size(); ++i)
            argv[i] = const_cast<char*>(arguments[i].c_str());
        return main(argv.size(), &argv[0]);
    }
    catch (const std::exception& e)
    {
        ::MessageBoxA(0, e.what(), "Uncaught Exception", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }
}

#else

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	if (FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED))) {
		return 1;
	}

	SDL_WinRTRunApp(main, NULL);
	return 0;
}

#endif
