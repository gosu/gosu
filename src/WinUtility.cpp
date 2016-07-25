#include <Gosu/Utility.hpp>
#include "WinUtility.hpp"
#include <stdexcept>
#include <windows.h>

void Gosu::Win::throwLastError(const std::string& action)
{
    // Obtain error message from Windows.
    char* buffer;
    if (!::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0,
        ::GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPSTR>(&buffer), 0, 0) || buffer == 0)
    {
        // IMPR: Can we do better than this?
        throw std::runtime_error("Unknown error");
    }

    // Move the message out of the ugly char* buffer.
    std::string message;
    try
    {
        message = buffer;
    }
    catch (...)
    {
        ::LocalFree(buffer);
        throw;
    }
    ::LocalFree(buffer);
    
    // Optionally prepend the action.
    if (!action.empty())
        message = "While " + action + ", the following error occured: " +
            message;

    // Now throw it.
    throw std::runtime_error(message);
 }
