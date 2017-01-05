#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <exception>
#include <string>
#include <vector>
#include <windows.h>
using namespace std;

vector<string> split_cmd_line()
{
    vector<string> result;

    const char* cmd_line = ::GetCommandLineA();

    const char* arg_begin = nullptr;
    bool is_quoted_arg = false;

    while (*cmd_line) {
        if (*cmd_line == '"') {
            if (arg_begin == nullptr) {
                arg_begin = cmd_line + 1;
                is_quoted_arg = true;
            }
            else if (is_quoted_arg) {
                result.push_back(std::string(arg_begin, cmd_line));
                arg_begin = nullptr;
            }
        }
        else if (!isspace((unsigned char)*cmd_line) && arg_begin == nullptr) {
            arg_begin = cmd_line;
            is_quoted_arg = false;
        }
        else if (isspace((unsigned char)*cmd_line) && arg_begin != nullptr && !is_quoted_arg) {
            result.push_back(std::string(arg_begin, cmd_line + 1));
            arg_begin = nullptr;
        }
        ++cmd_line;
    }

    if (arg_begin != 0)
        result.push_back(arg_begin);

    return result;
}

int main(int argc, char* argv[]);

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    try {
        vector<string> arguments = split_cmd_line();
        vector<char*> argv(arguments.size());
        for (unsigned i = 0; i < argv.size(); ++i)
            argv[i] = const_cast<char*>(arguments[i].c_str());
        return main(argv.size(), &argv[0]);
    }
    catch (const std::exception& e) {
        ::MessageBoxA(0, e.what(), "Uncaught Exception", MB_OK | MB_ICONERROR);
        return EXIT_FAILURE;
    }
}

#endif
