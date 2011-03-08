//! \file WinUtility.hpp
//! Contains some functions which are used to implement Gosu on Windows and
//! might be useful for advanced users who try to integrate Gosu in a Win32
//! application.

#ifndef GOSU_WINUTILITY_HPP
#define GOSU_WINUTILITY_HPP

#include <windows.h>
#include <Gosu/Platform.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

namespace Gosu
{
    //! Implementation helpers for the Windows platform.
    namespace Win
    {
        //! Returns the instance handle of the application.
        HINSTANCE instance();

        //! Blocking function which waits for the next message, processes it,
        //! then returns.
        void handleMessage();

        //! Non-blocking function which processes all waiting messages but does
        //! not wait for further incoming messages.
        void processMessages();

        //! Registers a function to be called by handleMessage and
        //! processMessages. Every message is passed to the hooks and not
        //! processed further if any hook function returns true.
        void registerMessageHook(const boost::function<bool (MSG&)>& hook);
        
        //! Throws an exception according to the error which GetLastError()
        //! returns, optionally prefixed with "While (action), the following
        //! error occured: ".
        GOSU_NORETURN void throwLastError(const std::string& action = "");

        //! Small helper function that throws an exception whenever the value
        //! passed through is false. Note that this doesn't make sense for all
        //! Windows API functions, but for most of them.
        template<typename T>
        inline T check(T valToCheck, const std::string& action = "")
        {
            if (!valToCheck)
                throwLastError(action);
            return valToCheck;
        }

        // IMPR: Why can't I use mem_fn for releasing objects even though it is
        // shown like that in the shared_ptr documentation?
        template<typename T>
        void releaseComPtr(T* ptr)
        {
            ptr->Release();
        }

        //! Small helper function that transfers ownership of a COM interface
        //! to a boost::shared_ptr.
        template<typename T>
        inline boost::shared_ptr<T> shareComPtr(T* ptr)
        {
            return boost::shared_ptr<T>(ptr, releaseComPtr<T>);
        }

        //! Returns the executable's filename.
        std::wstring appFilename();

        //! Returns the executable's containing directory.
        std::wstring appDirectory();
    }
}

#endif
