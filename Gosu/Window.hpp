//! \file Window.hpp
//! Interface of the Window class.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Platform.hpp>
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
    //! Returns the width (in pixels) of the user's primary screen.
    unsigned screen_width();
    
    //! Returns the height (in pixels) of the user's primary screen.
    unsigned screen_height();
    
    //! Returns the maximum width (in 'points') that is available for a non-fullscreen Window.
    //! All windows larger than this size will automatically be shrunk to fit.
    unsigned available_width();
    
    //! Returns the maximum height (in 'points') that is available for a non-fullscreen Window.
    //! All windows larger than this size will automatically be shrunk to fit.
    unsigned available_height();
    
    //! Convenient all-in-one class that serves as the foundation of a standard Gosu application.
    //! Manages initialization of all of Gosu's core components and provides timing functionality.
    //! Note that you should really only use one instance of this class at the same time.
    class Window
    {
        struct Impl;
        // Non-movable (const) to avoid dangling internal references.
        const std::unique_ptr<Impl> pimpl;

    public:
        //! Constructs a Window.
        //! \param width Width of the window in points; that is, pixels on a normal display, and
        //! 'points' on a high-resolution display.
        //! \param height See width.
        //! \param update_interval Interval in milliseconds between two calls to the update member
        //! function.
        Window(unsigned width, unsigned height, bool fullscreen = false,
            double update_interval = 16.666666);
        virtual ~Window();

        unsigned width() const;
        unsigned height() const;
        bool fullscreen() const;
        void resize(unsigned width, unsigned height, bool fullscreen);
        
        double update_interval() const;
        void set_update_interval(double update_interval);

        std::string caption() const;
        void set_caption(const std::string& caption);

        //! Enters a modal loop where the Window is visible on screen and
        //! receives calls to draw, update etc.
        virtual void show();
        
        //! EXPERIMENTAL - MAY DISAPPEAR WITHOUT WARNING.
        //! Performs a single mainloop step.
        //! This method is only useful if you want to integrate Gosu with another library that has
        //! its own main loop.
        //! This method implicitly shows the window if it was hidden before, and returns false when
        //! the window has been closed.
        //! If you discard the return value and keep calling tick(), the window will be shown again,
        //! or keep being shown.
        virtual bool tick();
        
        //! Closes the window if it is currently shown.
        //! If you do not want the window to close immediately, you should override this method and
        //! only call the base implementation (Window::close) when needed.
        virtual void close();

        //! Called every update_interval milliseconds while the window is being shown.
        //! Your application's main game logic goes here.
        virtual void update() {}
        
        //! Called after every update and when the OS wants the window to repaint itself.
        //! Your application's rendering code goes here.
        virtual void draw() {}
        
        //! Gives the game a chance to say no to being redrawn.
        //! This is not a definitive answer. The operating system can still force the window to
        //! redraw itself.
        //! By default, the window is redrawn all the time.
        virtual bool needs_redraw() const { return true; }

        //! If this function returns true, the system arrow cursor is drawn while over the window.
        virtual bool needs_cursor() const { return false; }
        
        //! This function is called when the window loses focus on some platforms.
        //! Most importantly, it is called on the iPhone or iPad when the user locks the screen.
        virtual void lose_focus() {}
        
        //! This function is called when the operating system's memory is low.
        //! So far, it is only called in iOS applications.
        virtual void release_memory() {}

        //! Called before update when the user presses a button while the window has the focus.
        //! By default, this will toggle fullscreen mode if the user presses Alt+Enter (Windows,
        //! Linux) or cmd+F (macOS).
        //! To support these shortcuts in your application, make sure to call Window::button_down
        //! in your implementation.
        virtual void button_down(Gosu::Button);
        
        //! Same as button_down. Called when the user releases a button.
        virtual void button_up(Gosu::Button) {}

        //! Called when a file is dropped onto the window.
        //! \param path The filename of the dropped file. When multiple files are dropped, this
        //! method will be called several times.
        virtual void drop(const std::string& filename) {}

        //! EXPERIMENTAL - MAY DISAPPEAR WITHOUT WARNING.
        //! Creates a Gosu::Bitmap from the current framebuffer (aka Screenshot)
        //! can then be used for various things most commonly save it to a file
        Gosu::Bitmap to_bitmap();
        
        // Ignore when SWIG is wrapping this class for Ruby/Gosu.
        #ifndef SWIG
        // Callbacks for touch events. So far these are only used on iOS.
        virtual void touch_began(Touch touch) {}
        virtual void touch_moved(Touch touch) {}
        virtual void touch_ended(Touch touch) {}
        virtual void touch_cancelled(Touch touch) {}
        
        const Graphics& graphics() const;
        Graphics& graphics();
        
        const Input& input() const;
        Input& input();
        #endif
        
        #ifdef GOSU_IS_IPHONE
        void* UIWindow() const;
        #endif
    };
}
