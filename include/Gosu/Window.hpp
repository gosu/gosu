#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Input.hpp>
#include <Gosu/Platform.hpp>
#include <memory>
#include <string>

namespace Gosu
{
    enum WindowFlags
    {
        WF_WINDOWED,
        WF_FULLSCREEN = 1,
        WF_RESIZABLE  = 2,
        WF_BORDERLESS = 4
    };

    //! Convenient all-in-one class that serves as the foundation of a standard Gosu application.
    //! Manages initialization of all of Gosu's core components and provides timing functionality.
    //! Note that you should really only use one instance of this class at the same time.
    class Window
    {
        struct Impl;
        const std::unique_ptr<Impl> pimpl;

    public:
        //! Constructs a Window.
        //! \param width Width of the window in points; that is, pixels on a normal display, and
        //! 'points' on a high-resolution display.
        //! \param height See width.
        //! \param window_flags A bitmask of values from Gosu::WindowFlags.
        //! \param update_interval Interval in milliseconds between two calls to the update member
        //! function.
        Window(int width, int height, unsigned window_flags = WF_WINDOWED,
               double update_interval = 16.666666);
        virtual ~Window();

        int width() const;
        int height() const;
        bool fullscreen() const;
        void resize(int width, int height, bool fullscreen);

        bool resizable() const;
        void set_resizable(bool resizable);

        bool borderless() const;
        void set_borderless(bool borderless);

        double update_interval() const;
        void set_update_interval(double update_interval);

        std::string caption() const;
        void set_caption(const std::string& caption);

        //! Enters a modal loop where the Window is visible on screen and
        //! receives calls to draw, update etc.
        virtual void show();

        //! EXPERIMENTAL - MAY DISAPPEAR WITHOUT WARNING.
        //! Performs a single main loop step.
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

        //! If this function returns true, the system cursor will be visible while over the window.
        virtual bool needs_cursor() const { return true; }

        //! This function is called when the window loses focus on some platforms.
        //! Most importantly, it is called on the iPhone or iPad when the user locks the screen.
        virtual void lose_focus() {}

        //! This function is called when the operating system's memory is low.
        //! So far, it is only called in iOS applications.
        virtual void release_memory() {}

        //! Called before update when the user presses a button while the window has the focus.
        //! By default, this will toggle fullscreen mode if the user presses Alt+Enter (Windows,
        //! Linux), cmd+F (macOS), or F11 (on all operating systems).
        //! To support these shortcuts in your application, make sure to call Window::button_down
        //! in your implementation.
        virtual void button_down(Gosu::Button);

        //! Same as button_down. Called when the user releases a button.
        virtual void button_up(Gosu::Button) {}

        //! Called when a gamepad is connected.
        //! \param index the index of the gamepad slot that the gamepad is in.
        virtual void gamepad_connected(int index) {}

        //! Called when a gamepad is disconnected.
        //! \param index the index of the gamepad slot that the gamepad is in. The slot will be freed immediately after this callback.
        virtual void gamepad_disconnected(int index) {}

        //! Called when a file is dropped onto the window.
        //! \param filename The filename of the dropped file. When multiple files are dropped, this
        //! method will be called several times.
        virtual void drop(const std::string& filename) {}

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
        void* uikit_window() const;
        #endif
    };

    //! Returns the width (in pixels) of a screen.
    //! \param window The result describes the screen on which the window is shown, or the
    //!               primary screen if no window is given.
    int screen_width(Window* window = nullptr);

    //! Returns the height (in pixels) of the user's primary screen.
    //! \param window The result describes the screen on which the window is shown, or the
    //!               primary screen if no window is given.
    int screen_height(Window* window = nullptr);

    //! Returns the maximum width (in 'points') that is available for a non-fullscreen Window.
    //! All windows larger than this size will automatically be shrunk to fit.
    //! \param window The result describes the screen on which the window is shown, or the
    //!               primary screen if no window is given.
    int available_width(Window* window = nullptr);

    //! Returns the maximum height (in 'points') that is available for a non-fullscreen Window.
    //! All windows larger than this size will automatically be shrunk to fit.
    //! \param window The result describes the screen on which the window is shown, or the
    //!               primary screen if no window is given.
    int available_height(Window* window = nullptr);
}
