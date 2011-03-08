#include <Gosu/WinUtility.hpp>
#include <Gosu/Utility.hpp>
#include <stdexcept>
#include <vector>

namespace
{
    typedef std::vector<boost::function<bool (MSG&)> > Hooks;
    Hooks hooks;

    bool handledByHook(MSG& message)
    {
        for (Hooks::iterator i = hooks.begin(); i != hooks.end(); ++i)
            if ((*i)(message))
                return true;

        return false;
    }
}

HINSTANCE Gosu::Win::instance()
{
    return check(::GetModuleHandle(0), "getting the module handle");
}

void Gosu::Win::handleMessage()
{
    MSG message;
    BOOL ret = ::GetMessage(&message, 0, 0, 0);

    switch (ret)
    {
        case -1:
        {
            // GetMessage() failed.
            throwLastError("trying to get the next message");
        }

        case 0:
        {
            // GetMessage() found a WM_QUIT message.
            // IMPR: Is there a better way to respond to this?
            break;
        }

        default:
        {
            // Normal behaviour, if the message does not get handled by
            // something else.
            if (!handledByHook(message))
            {
                ::TranslateMessage(&message);
                ::DispatchMessage(&message);
            }
        }
    }
}

void Gosu::Win::processMessages()
{
    MSG message;

    while (::PeekMessage(&message, 0, 0, 0, PM_REMOVE))
        if (!handledByHook(message))
        {
            ::TranslateMessage(&message);
            ::DispatchMessage(&message);
        }
}

void Gosu::Win::registerMessageHook(const boost::function<bool (MSG&)>& hook)
{
    hooks.push_back(hook);
}

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

std::wstring Gosu::Win::appFilename()
{
    static std::wstring result;
    if (!result.empty())
        return result;

    wchar_t buffer[MAX_PATH * 2];
    check(::GetModuleFileName(0, buffer, MAX_PATH * 2),
        "getting the module filename");
    result = buffer;
    return result;
}

std::wstring Gosu::Win::appDirectory()
{
    static std::wstring result;
    if (!result.empty())
        return result;

    result = appFilename();
    std::wstring::size_type lastDelim = result.find_last_of(L"\\/");
    if (lastDelim != std::wstring::npos)
        result.resize(lastDelim + 1);
    else
        result = L"";
    return result;
}
