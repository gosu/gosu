//! \file Window.hpp
//! Interface of the Window class.

#ifndef GOSU_WINDOW_HPP
#define GOSU_WINDOW_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Input.hpp>
#include <boost/scoped_ptr.hpp>
#include <string>

#ifdef GOSU_IS_WIN
#include <windows.h>
#endif

namespace Gosu
{
    //! Convenience class that serves as the foundation of a standard Gosu
    //! application. Manages initialization of all of Gosu's core components
    //! and provides basic timing functionality.
    class Window
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

        // Called when the user tries to close the application with the
        // interface provided by the window manager (i.e. the [X] button).
        // Applications can override this member function to ask the user
        // whether she is sure etc.
        /*virtual void closeRequest()
        { 
            close(); 
        }*/
        
    public:
        //! Constructs a Window.
        //! \param updateInterval Interval in milliseconds between two calls
        //! to the update member function.
        Window(unsigned width, unsigned height, bool fullscreen,
            unsigned updateInterval);
        virtual ~Window();

        std::wstring caption() const;
        void setCaption(const std::wstring& caption);

        //! Starts the main event loop.
        void show();
        //! Closes the window if it is currently shown.
        void close();

        //! Called every updateInterval milliseconds while the window is being
        //! shown. Your application's main game logic goes here.
        virtual void update() {}
        //! Called after every update and when the OS wants the window to
        //! repaint itself. Your application's rendering code goes here.
        virtual void draw() {}

        //! Called before update when the user pressed a button while the
        //! window had the focus.
        virtual void buttonDown(Gosu::Button btn) {}
        //! Same as buttonDown. Called then the user released a button.
        virtual void buttonUp(Gosu::Button btn) {}
        
#ifndef SWIG
        const Graphics& graphics() const;
        Graphics& graphics();

        const Audio& audio() const;
        Audio& audio();

        const Input& input() const;
        Input& input();

        #ifdef GOSU_IS_WIN
        HWND handle() const;
        virtual LRESULT handleMessage(UINT message, WPARAM wparam,
            LPARAM lparam);
        #endif
#endif
    };
}

#endif
