//! \file Window.hpp
//! Interface of the Window class.

#ifndef GOSU_WINDOW_HPP
#define GOSU_WINDOW_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Input.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <string>
#include <vector>

#ifdef GOSU_IS_WIN
#include <windows.h>
#endif

namespace Gosu
{
    #ifdef GOSU_IS_IPHONE
    struct Touch { double x, y; };
    typedef std::vector<Touch> Touches;
    #endif

    //! Convenient all-in-one class that serves as the foundation of a standard
	//! Gosu application. Manages initialization of all of Gosu's core components
    //! and provides timing functionality.
    class Window
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        //! Constructs a Window.
        //! \param updateInterval Interval in milliseconds between two calls
        //! to the update member function.
        Window(unsigned width, unsigned height, bool fullscreen,
            double updateInterval = 16.666666);
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
        virtual void buttonDown(Gosu::Button) {}
        //! Same as buttonDown. Called then the user released a button.
        virtual void buttonUp(Gosu::Button) {}
        
        // Ignore when SWIG is wrapping this class for Ruby/Gosu.
        #ifndef SWIG
        const Graphics& graphics() const;
        Graphics& graphics();

        const Audio& audio() const;
        Audio& audio();

        const Input& input() const;
        Input& input();

        #ifdef GOSU_IS_WIN
        // Only on Windows.
        HWND handle() const;
        virtual LRESULT handleMessage(UINT message, WPARAM wparam,
            LPARAM lparam);
        #else
        // Only on Unices (so far).
        typedef boost::shared_ptr<boost::function<void()> > SharedContext;
        SharedContext createSharedContext();
        #endif
        #ifdef GOSU_IS_IPHONE
        virtual void touchesBegan(const Touches& touches) {}
        virtual void touchesMoved(const Touches& touches) {}
        virtual void touchesEnded(const Touches& touches) {}
        #endif        

        #endif
    };
}

#endif
