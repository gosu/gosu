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

#ifdef GOSU_IS_WIN
#include <windows.h>
#endif

namespace Gosu
{
    //! Convenient all-in-one class that serves as the foundation of a standard
	//! Gosu application. Manages initialization of all of Gosu's core components
    //! and provides timing functionality.
    //! Note that you should really only use on instance of this class at the same time.
    //! This may or may not change later.
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
        
        double updateInterval() const;

        //! Enters a modal loop where the Window is visible on screen and receives calls to draw, update etc.
        void show();
        //! Closes the window if it is currently shown.
        void close();

        //! Called every updateInterval milliseconds while the window is being
        //! shown. Your application's main game logic goes here.
        virtual void update() {}
        //! Called after every update and when the OS wants the window to
        //! repaint itself. Your application's rendering code goes here.
        virtual void draw() {}
        
        //! Gives the game a chance to say no to being redrawn.
        //! This is not a definitive answer. The operating system can still cause
        //! redraws for one reason or another.
        //! By default, the window is redrawn all the time.
        virtual bool needsRedraw() const { return true; }
        
        //! Called before update when the user pressed a button while the
        //! window had the focus.
        virtual void buttonDown(Gosu::Button) {}
        //! Same as buttonDown. Called then the user released a button.
        virtual void buttonUp(Gosu::Button) {}
        
        // Ignore when SWIG is wrapping this class for Ruby/Gosu.
        #ifndef SWIG
        
        const Graphics& graphics() const;
        Graphics& graphics();
        
        const Input& input() const;
        Input& input();
        
        #ifdef GOSU_IS_WIN
        // Only on Windows, used for integrating with GUI toolkits.
        HWND handle() const;
        virtual LRESULT handleMessage(UINT message, WPARAM wparam,
            LPARAM lparam);
        #endif
        
        #ifdef GOSU_IS_UNIX
        // Context for creating shared contexts.
        // Only on Unices (so far).
        typedef boost::shared_ptr<boost::function<void()> > SharedContext;
        SharedContext createSharedContext();
        #endif
        
        #ifdef GOSU_IS_IPHONE
        // iPhone-only callbacks for touch events.
        // Note that it does not hurt to override them even if you compile
        // for another platform; if you don't specify "virtual" the code
        // should even be stripped away cleanly.
        virtual void touchesBegan(const Touches& touches) {}
        virtual void touchesMoved(const Touches& touches) {}
        virtual void touchesEnded(const Touches& touches) {}
        // Currently known touches.
        const Touches& currentTouches() const;
        #endif        
        
        #endif
        
        // Deprecated.
        const Audio& audio() const;
        Audio& audio();
    };
}

#endif
