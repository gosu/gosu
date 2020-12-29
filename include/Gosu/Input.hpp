#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Buttons.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Gosu
{
    /// Struct that saves information about a touch on the surface of a multi-touch device.
    /// (Right now this is only supported on iOS.)
    struct Touch
    {
        /// Allows for identification of a touch across calls.
        void* id;
        /// Position of a touch on the touch screen.
        double x, y;
    };
    typedef std::vector<Touch> Touches;

    /// Manages initialization and shutdown of the input system.
    /// Only one Input instance can exist per application.
    class Input : Noncopyable
    {
        struct Impl;
        std::unique_ptr<Impl> pimpl;

    public:
    #ifdef GOSU_IS_IPHONE
        Input(void* view, float update_interval);
        void feed_touch_event(std::function<void (Touch)>& callback, void* touches);
    #else
        explicit Input(void* window);
        bool feed_sdl_event(void* event);
    #endif

        ~Input();

        //! Returns the character (as a UTF-8 encoded string) that a button usually produces.
        //! Returns the empty string if nothing can be found.
        static std::string id_to_char(Button btn);
        //! Returns the button that has to be pressed to produce the given character (as a UTF-8
        //! encoded string), or NO_BUTTON.
        static Button char_to_id(std::string ch);

        //! Returns the name for the given button, provided it is a Gosu::KB_{constant}
        //! Returns the empty string if nothing can be found.
        static std::string button_name(Button btn);

        //! Returns the name for the given gamepad.
        //! Returns an empty string if there is no gamepad in the slot index.
        static std::string gamepad_name(int index);

        //! Returns true if a button is currently pressed.
        //! Updated every tick.
        static bool down(Button btn);

        //! Returns the value of a gamepad stick in the range -1.0 through +1.0, or a trigger in the
        //! range 0.0 through +1.0.
        //! Updated every tick.
        static double axis(Button btn);

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
        void set_mouse_factors(double factor_x, double factor_y,
            double offset_x = 0, double offset_y = 0);

        //! Currently known touches.
        const Touches& current_touches() const;

        //! Accelerometer positions in all three dimensions (smoothened).
        //! Note: Not really deprecated, but temporarily defunct.
        GOSU_DEPRECATED double accelerometer_x() const;
        GOSU_DEPRECATED double accelerometer_y() const;
        GOSU_DEPRECATED double accelerometer_z() const;

        //! Collects new information about which buttons are pressed, where the
        //! mouse is and calls on_button_up/on_button_down, if assigned.
        void update();

        //! Assignable events that are called by update. You can bind these to your own functions.
        //! If you use the Window class, it will assign these to its own methods.
        std::function<void (Button)> on_button_down, on_button_up;

        std::function<void (int)> on_gamepad_connected, on_gamepad_disconnected;

        //! Assignable events that are called by update. You can bind these to your own functions.
        //! If you use the Window class, it will assign these to its own methods.
        std::function<void (Touch)> on_touch_began, on_touch_moved, on_touch_ended,
            on_touch_cancelled;

        //! Returns the currently active TextInput instance, or nullptr.
        TextInput* text_input() const;
        //! Sets the currently active TextInput, or resets it to the nullptr.
        void set_text_input(TextInput* input);
    };
}
