//! \file Window.hpp
//! Interface of the Window class.

#ifndef GOSU_WINDOW_HPP
#define GOSU_WINDOW_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/TR1.hpp>
#include <memory>
#include <string>

#ifdef GOSU_IS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace Gosu
{
    //! Convenient all-in-one class that serves as the foundation of a standard
    //! Gosu application. Manages initialization of all of Gosu's core components
    //! and provides timing functionality.
    //! Note that you should really only use one instance of this class at the same time.
    //! This may or may not change later.
    class Window
    {
        struct Impl;
        const std::auto_ptr<Impl> pimpl;
	
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

        //! Enters a modal loop where the Window is visible on screen and
        //! receives calls to draw, update etc.
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

        //! If this function returns true, the system arrow cursor is drawn while
        //! over the window.
        virtual bool needsCursor() const { return false; }
        
        //! This function is called when the window loses focus on some platforms.
        //! Most importantly, it is called on the iPhone or iPad when the user
        //! locks the screen.
        virtual void loseFocus() {}
        
        //! This function is called when the operating system's memory is low.
        //! So far, it is only called in iOS applications.
        virtual void releaseMemory() {}
        
				//! This function may be called on some platforms when some window callbacks
				//! encounter an uncaught exception and cannot handle it themselves, i.e.
				//! the exception wouldn't reach show().
				virtual void panic(std::exception& e);

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
        typedef std::tr1::shared_ptr<std::tr1::function<void()> > SharedContext;
        SharedContext createSharedContext();
        #endif
        
        #ifdef GOSU_IS_IPHONE
        void* rootViewController() const;
        // iPhone-only callbacks for touch events.
        // Note that it does not hurt to override them even if you compile
        // for another platform; if you don't specify "virtual" the code
        // should even be stripped away cleanly.
        virtual void touchBegan(Touch touch) {}
        virtual void touchMoved(Touch touch) {}
        virtual void touchEnded(Touch touch) {}
        #endif        
        
        const Audio& audio() const;
        Audio& audio();

        #endif
    };
}

#ifdef GOSU_IS_IPHONE
Gosu::Window& windowInstance();
#endif

#endif
