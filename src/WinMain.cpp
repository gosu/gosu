#include <windows.h>
#include <exception>
#include <string>
#include <vector>
using namespace std;

vector<string> splitCmdLine()
{
    vector<string> result;

    const char* cmdLine = ::GetCommandLineA();

    const char* argBegin = nullptr;
    bool isQuotedArg = false;

    while (*cmdLine)
    {
        if (*cmdLine == '"')
        {
            if (argBegin == nullptr)
            {
                argBegin = cmdLine + 1;
                isQuotedArg = true;
            }
            else if (isQuotedArg)
            {
                result.push_back(std::string(argBegin, cmdLine));
                argBegin = nullptr;
            }
        }
        else if (!isspace((unsigned char)*cmdLine) && argBegin == nullptr)
        {
            argBegin = cmdLine;
            isQuotedArg = false;
        }
        else if (isspace((unsigned char)*cmdLine) && argBegin != nullptr && !isQuotedArg)
        {
            result.push_back(std::string(argBegin, cmdLine + 1));
            argBegin = nullptr;
        }
        ++cmdLine;
    }

    if (argBegin != 0)
        result.push_back(argBegin);

    return result;
}

int main(int argc, char* argv[]);

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
