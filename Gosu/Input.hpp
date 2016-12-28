//! \file Input.hpp
//! Interface of the Input class.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Buttons.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace Gosu
{
    //! Very lightweight class that identifies a button (keyboard, mouse or other device).
    class Button
    {
        unsigned id_;
        
    public:
        //! For internal use.
        explicit Button(unsigned id) : id_(id) {}
        //! For internal use.
        unsigned id() const { return id_; }

        //! Default constructor; == no_button.
        Button() : id_(NO_BUTTON) {}

        //! Conversion from ButtonName constants.
        Button(ButtonName name) : id_(name) {}
    };
    
    //! Tests whether two Buttons identify the same physical button.
    inline bool operator==(Button lhs, Button rhs)
    {
        return lhs.id() == rhs.id();
    }
    inline bool operator!=(Button lhs, Button rhs)
    {
        return !(lhs == rhs);
    }
    inline bool operator<(Button lhs, Button rhs)
    {
        return lhs.id() < rhs.id();
    }
    
    //! Struct that saves information about a touch on the surface of a multi-
    //! touch device.
    //! Available even on non-iPhone platforms to make it easier to compile the
    //! same source for multiple platforms.
    struct Touch
    {
        //! Allows for identification of a touch across calls.
        void* id;
        //! Position of a touch on the touch screen.
        float x, y;
    };
    typedef std::vector<Touch> Touches;
    
    //! Manages initialization and shutdown of the input system. Only one Input
    //! instance can exist per application.
    class Input
    {
        struct Impl;
        // Non-movable (const) to avoid dangling internal references.
        const std::unique_ptr<Impl> pimpl;

    public:
    #ifdef GOSU_IS_IPHONE
        Input(void* view, float update_interval);
        void feed_touch_event(int type, void* touches);
    #else
        Input(void* window);
        bool feed_sdl_event(void* event);
    #endif
        
        ~Input();
        
        //! Returns the character a button usually produces, or 0.
        static wchar_t id_to_char(Button btn);
        //! Returns the button that has to be pressed to produce the
        //! given character, or no_button.
        static Button char_to_id(wchar_t ch);
        
        //! Returns true if a button is currently pressed.
        //! Updated every tick.
        static bool down(Button btn);
        
        //! Returns the horizontal position of the mouse relative to the top
        //! left corner of the window given to Input's constructor.
        double mouse_x() const;
        //! See mouse_x.
        double mouse_y() const;
        
        //! Immediately moves the mouse as far towards the desired position
        //! as possible. x and y are relative to the window, just as in mouse_x()
        //! and mouse_y(), so (0, 0) is the top left corner of the window..
        void set_mouse_position(double x, double y);

        // Undocumented for the moment. Also applies to current_touches().
        void set_mouse_factors(double factor_x, double factor_y, double offset_x = 0, double offset_y = 0);
        
        //! Currently known touches.
        const Touches& current_touches() const;
        
        //! Accelerometer positions in all three dimensions (smoothened).
        //! Note: Not really deprecated - but temporarily defunct.
        GOSU_DEPRECATED double accelerometer_x() const;
        GOSU_DEPRECATED double accelerometer_y() const;
        GOSU_DEPRECATED double accelerometer_z() const;
        
        //! Collects new information about which buttons are pressed, where the
        //! mouse is and calls on_button_up/on_button_down, if assigned.
        void update();
        
        //! Assignable events that are called by update. You can bind these to your own functions.
        //! If you use the Window class, it will assign these to its own methods.
        std::function<void (Button)> on_button_down, on_button_up;
        
        //! Assignable events that are called by update. You can bind these to your own functions.
        //! If you use the Window class, it will assign these to its own methods.
        std::function<void (Touch)> on_touch_began, on_touch_moved, on_touch_ended, on_touch_cancelled;
        
        //! Returns the currently active TextInput instance, or 0.
        TextInput* text_input() const;
        //! Sets the currently active TextInput, or clears it (input = 0).
        void set_text_input(TextInput* input);
    };
}
