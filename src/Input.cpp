#include <Gosu/Platform.hpp>
#if !defined(GOSU_IS_IPHONE)

#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <Gosu/Utility.hpp>

#include <SDL.h>
#include "utf8proc.h"

#include <cwctype>
#include <cstdlib>
#include <algorithm>
#include <array>
using namespace std;

// Workaround for broken SDL_GetGlobalMouseState, see below.
#ifdef GOSU_IS_MAC
#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>
#endif

static void require_sdl_video()
{
    static bool initialized = false;
    if (!initialized) {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
        initialized = true;
        atexit([] { SDL_QuitSubSystem(SDL_INIT_VIDEO); });
    }
}

static array<bool, Gosu::NUM_BUTTONS> button_states = { { false } };
static array<double, Gosu::GP_NUM_AXES> axis_states = { { 0 } };
static array<int, Gosu::NUM_GAMEPADS> gamepad_slots = { { -1, -1, -1, -1 } }; // Stores joystick instance id or -1 if empty
static vector<SDL_Joystick *> joysticks;
static vector<SDL_GameController *> game_controllers;
struct InputEvent
{
    int id;
    int type;
    double axis_value;
    bool gamepad_connected;
    int gamepad_instance_id;
};

enum InputEventType {
    ButtonEvent,
    AxisEvent,
    ControllerConnectionEvent
};

struct Gosu::Input::Impl
{
    Input& input;
    SDL_Window* window;

    TextInput* text_input = nullptr;
    double mouse_x, mouse_y;
    double mouse_scale_x = 1;
    double mouse_scale_y = 1;
    double mouse_offset_x = 0;
    double mouse_offset_y = 0;

    Impl(Input& input, SDL_Window* window)
    : input(input), window(window)
    {
        require_sdl_video();

        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

        int num_gamepads = min<int>(Gosu::NUM_GAMEPADS, SDL_NumJoysticks());
    }

    ~Impl()
    {
        for_each(joysticks.begin(), joysticks.end(), &SDL_JoystickClose);
        joysticks.clear();
        for_each(game_controllers.begin(), game_controllers.end(), &SDL_GameControllerClose);
        game_controllers.clear();

        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    void update_mouse_position()
    {
    #if defined(GOSU_IS_MAC)
        // Avoid SDL_GetGlobalMouseState on macOS until this bug is fixed:
        // https://bugzilla.libsdl.org/show_bug.cgi?id=4255
        int window_x, window_y;
        SDL_GetWindowPosition(window, &window_x, &window_y);
        auto mouse_position = NSEvent.mouseLocation;
        mouse_x = mouse_position.x - window_x;
        mouse_y = (CGDisplayPixelsHigh(kCGDirectMainDisplay) - mouse_position.y) - window_y;
    #elif SDL_VERSION_ATLEAST(2, 0, 5)
        // SDL_GetGlobalMouseState was added in SDL 2.0.4, but it only started using the same
        // coordinate system as SDL_GetWindowPosition on X11 in 2.0.5.
        int x, y, window_x, window_y;
        SDL_GetWindowPosition(window, &window_x, &window_y);
        SDL_GetGlobalMouseState(&x, &y);
        mouse_x = x - window_x;
        mouse_y = y - window_y;
    #else
        int x, y;
        SDL_GetMouseState(&x, &y);
        mouse_x = x;
        mouse_y = y;
    #endif
    }

    void set_mouse_position(double x, double y)
    {
        SDL_WarpMouseInWindow(window,
                              static_cast<int>((x - mouse_offset_x) / mouse_scale_x),
                              static_cast<int>((y - mouse_offset_y) / mouse_scale_y));

    #if SDL_VERSION_ATLEAST(2, 0, 4) && !defined(GOSU_IS_X)
        // On systems where we have a working GetGlobalMouseState, we can warp the mouse and
        // retrieve its position directly afterwards.
        update_mouse_position();
    #else
        // Otherwise, we have to assume that setting the position worked, because if we update the
        // mouse position now, we'll get the previous position.
        mouse_x = x, mouse_y = y;
    #endif
    }

    bool feed_sdl_event(const SDL_Event* e)
    {
        switch (e->type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                if (e->key.repeat == 0 && e->key.keysym.scancode <= KB_RANGE_END) {
                    enqueue_event(e->key.keysym.scancode, e->type == SDL_KEYDOWN);
                    return true;
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                if (e->button.button >= 1 && e->button.button <= 3) {
                    enqueue_event(MS_LEFT + e->button.button - 1, e->type == SDL_MOUSEBUTTONDOWN);
                    return true;
                }
                break;
            }
            case SDL_MOUSEWHEEL: {
                if (e->wheel.y > 0) {
                    enqueue_event(MS_WHEEL_UP, true);
                    enqueue_event(MS_WHEEL_UP, false);
                    return true;
                }
                else if (e->wheel.y < 0) {
                    enqueue_event(MS_WHEEL_DOWN, true);
                    enqueue_event(MS_WHEEL_DOWN, false);
                    return true;
                }
                break;
            }
            case SDL_JOYAXISMOTION: {
                if (SDL_IsGameController(e->jaxis.which)) {
                    // Ignore this event as joystick triggers are reported in range -1..1 instead of 0..1
                    break;
                }
                else {
                    enqueue_event(ButtonName::GP_AXIS_LEFT_X + e->jaxis.axis, (double)e->jaxis.value);

                    if (int i = gamepad_slot_index(e->jaxis.which) && i >= 0) {
                        enqueue_event(ButtonName::GP_0_AXIS_LEFT_X + (6 * i), (double)e->jaxis.value);
                    }
                }
                break;
            }
            case SDL_CONTROLLERAXISMOTION: {
                enqueue_event(ButtonName::GP_AXIS_LEFT_X + e->caxis.axis, (double)e->caxis.value);

                if (int i = gamepad_slot_index(e->caxis.which) && i >= 0) {
                    enqueue_event(ButtonName::GP_0_AXIS_LEFT_X + (6 * i), (double)e->caxis.value);
                }
                break;
            }
            case SDL_JOYDEVICEADDED: {
                if (available_gamepad_slot_index() == -1) {
                    printf("Max Num of Gamepads added. Controllers: %i, Joysticks: %i (total: %i)\n", game_controllers.size(), joysticks.size(), game_controllers.size() + joysticks.size());
                    break;
                }
                int gamepad_slot = -1;
                int device_instance_id = -1;
                const char *gamepad_name;
                // Prefer the SDL_GameController API...
                if (SDL_IsGameController(e->jdevice.which)) {
                    if (SDL_GameController *game_controller = SDL_GameControllerOpen(e->jdevice.which)) {
                        printf("Added GameController Device with Instance ID: %i (%s)\n", SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(game_controller)), SDL_GameControllerNameForIndex(e->jdevice.which));
                        game_controllers.push_back(game_controller);
                        gamepad_slot = available_gamepad_slot_index();
                        gamepad_name = SDL_GameControllerNameForIndex(e->jdevice.which);
                        device_instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(game_controller));
                    }
                }
                // ...but fall back on the good, old SDL_Joystick API.
                else {
                    if (SDL_Joystick *joystick = SDL_JoystickOpen(e->jdevice.which)) {
                        printf("Added Joystick Device with Instance ID: %i (%s)\n", SDL_JoystickInstanceID(joystick), SDL_JoystickNameForIndex(e->jdevice.which));
                        joysticks.push_back(joystick);
                        gamepad_slot = available_gamepad_slot_index();
                        gamepad_name = SDL_JoystickNameForIndex(e->jdevice.which);
                        device_instance_id = SDL_JoystickInstanceID(joystick);
                    }
                }
                if (gamepad_slot >= 0 && device_instance_id >= 0) {
                    gamepad_slots[gamepad_slot] = device_instance_id;
                    enqueue_gamepad_connection_event(gamepad_slot, true, -1);
                }
                break;
            }
            case SDL_JOYDEVICEREMOVED: {
                int gamepad_slot = gamepad_slot_index(e->jdevice.which);

                if (gamepad_slot >= 0) {
                    enqueue_gamepad_connection_event(gamepad_slot, false, e->jdevice.which);
                }
                break;
            }
        }
        return false;
    }

    // returns the gamepad slot index (0..NUM_GAMEPADS - 1) for the joystick instance id or -1 if not found
    int gamepad_slot_index(int joystick_instance_id)
    {
        for (int i = 0; i < gamepad_slots.size(); i++) {
            if (gamepad_slots[i] == joystick_instance_id) {
                return i;
            }
        }

        return -1;
    }

    // returns first available gamepad slot or -1 if non are available
    int available_gamepad_slot_index()
    {
        for (int i = 0; i < gamepad_slots.size(); i++) {
            if (gamepad_slots[i] == -1) {
                return i;
            }
        }

        return -1;
    }

    // frees the gamepad slot associated with this joystick instance id
    // and frees the SDL_GameController/SDL_Joystick
    void free_gamepad_slot_index(int joystick_instance_id)
    {
        int index = gamepad_slot_index(joystick_instance_id);

        if (index >= 0) {
            for (int i = 0; i < game_controllers.size(); i++) {
                if (SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(game_controllers[i])) == joystick_instance_id) {
                    SDL_GameControllerClose(game_controllers[i]);
                    game_controllers.erase(game_controllers.begin() + i);
                    gamepad_slots[i] = -1;
                    return;
                }
            }
            for (int i = 0; i < joysticks.size(); i++) {
                if (SDL_JoystickInstanceID(joysticks[i]) == joystick_instance_id) {
                    SDL_JoystickClose(joysticks[i]);
                    joysticks.erase(joysticks.begin() + i);
                    gamepad_slots[i] = -1;
                    return;
                }
            }
        }
    }

    typedef array<bool, GP_NUM_PER_GAMEPAD> GamepadBuffer;

    void poll_gamepads()
    {
        // This gamepad is an OR-ed version of all the other gamepads. If button
        // 3 is pressed on any attached gamepad, down(GP_BUTTON_3) will return
        // true. This is handy for singleplayer games.
        GamepadBuffer any_gamepad = { false };

        for (int i = 0; i < gamepad_slots.size(); ++i) {
            SDL_GameController *game_controller = nullptr;
            SDL_Joystick *joystick = nullptr;
            GamepadBuffer current_gamepad = { false };

            if (gamepad_slots[i] == -1) {
                continue;
            }
            else {
                if (game_controller = SDL_GameControllerFromInstanceID(gamepad_slots[i])) {
                }
                else {
                    joystick = SDL_JoystickFromInstanceID(gamepad_slots[i]);
                }
            }

            // Poll data from SDL, using either of two API interfaces.
            if (game_controller) {
                poll_game_controller(game_controller, current_gamepad);
            }
            else {
                poll_joystick(joystick, current_gamepad);
            }

            // Now at the same time, enqueue all events for this particular
            // gamepad, and OR the keyboard state into any_gamepad.
            int offset = GP_RANGE_BEGIN + GP_NUM_PER_GAMEPAD * (i + 1);
            for (int j = 0; j < current_gamepad.size(); ++j) {
                any_gamepad[j] = any_gamepad[j] || current_gamepad[j];

                if (current_gamepad[j] && !button_states[j + offset]) {
                    button_states[j + offset] = true;
                    enqueue_event(j + offset, true);
                }
                else if (!current_gamepad[j] && button_states[j + offset]) {
                    button_states[j + offset] = false;
                    enqueue_event(j + offset, false);
                }
            }
        }

        // And lastly, enqueue events for the virtual "any" gamepad.
        for (int j = 0; j < any_gamepad.size(); ++j) {
            if (any_gamepad[j] && !button_states[j + GP_RANGE_BEGIN]) {
                button_states[j + GP_RANGE_BEGIN] = true;
                enqueue_event(j + GP_RANGE_BEGIN, true);
            }
            else if (!any_gamepad[j] && button_states[j + GP_RANGE_BEGIN]) {
                button_states[j + GP_RANGE_BEGIN] = false;
                enqueue_event(j + GP_RANGE_BEGIN, false);
            }
        }
    }

    void dispatch_enqueued_events()
    {
        for (InputEvent event : event_queue) {
            if (event.type == InputEventType::ButtonEvent) {
                bool down = (event.id >= 0);
                Button button(down ? event.id : ~event.id);

                button_states[button.id()] = down;
                if (down && input.on_button_down) {
                    input.on_button_down(button);
                }
                else if (!down && input.on_button_up) {
                    input.on_button_up(button);
                }
            }
            else if (event.type == InputEventType::AxisEvent) {
                axis_states[event.id - ButtonName::GP_AXIS_LEFT_X] = event.axis_value;
                if (input.on_axis_motion) {
                    input.on_axis_motion((Button)event.id, event.axis_value);
                }
            }
            else if (event.type == InputEventType::ControllerConnectionEvent) {
                if (event.gamepad_connected) {
                    if (input.on_gamepad_connected) {
                        input.on_gamepad_connected(event.id);
                    }
                }
                else {
                    if (input.on_gamepad_disconnected) {
                        input.on_gamepad_disconnected(event.id);
                        free_gamepad_slot_index(event.gamepad_instance_id);
                    }
                }
            }
        }
        event_queue.clear();
    }

private:
    // For button down event: Button name value (>= 0)
    // For button up event: ~Button name value (< 0)
    vector<InputEvent> event_queue;

    void enqueue_event(int id, bool down)
    {
        InputEvent s = { down ? id : ~id, InputEventType::ButtonEvent };
        event_queue.push_back( s );
    }

    void enqueue_event(int id, double value)
    {
        double axis_value = value >= 0 ? (double)value / SDL_JOYSTICK_AXIS_MAX : -(double)value / SDL_JOYSTICK_AXIS_MIN;
        InputEvent s = { id, InputEventType::AxisEvent, axis_value };
        event_queue.push_back( s );
    }

    void enqueue_gamepad_connection_event(int gamepad_index_id, bool connected, int instance_id)
    {
        InputEvent s = { gamepad_index_id, InputEventType::ControllerConnectionEvent, 0, connected, instance_id };
        event_queue.push_back( s );
    }

    // SDL returns axis values in the range -2^15 through 2^15-1, so we consider -2^14 through
    // 2^14 (half of that range) the dead zone.
    enum { DEAD_ZONE = (1 << 14) };

    void poll_game_controller(SDL_GameController* controller, GamepadBuffer& gamepad)
    {
        gamepad[GP_LEFT - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) < -DEAD_ZONE ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) < -DEAD_ZONE;

        gamepad[GP_RIGHT - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) > +DEAD_ZONE ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) > +DEAD_ZONE;

        gamepad[GP_UP - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) < -DEAD_ZONE ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) < -DEAD_ZONE;

        gamepad[GP_DOWN - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY) > +DEAD_ZONE ||
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY) > +DEAD_ZONE;

        int button = 0;
        for (; button < SDL_CONTROLLER_BUTTON_DPAD_UP; ++button) {
            gamepad[GP_BUTTON_0 + button - GP_RANGE_BEGIN] =
                SDL_GameControllerGetButton(controller, (SDL_GameControllerButton) button);
        }
        gamepad[GP_BUTTON_0 + button++ - GP_RANGE_BEGIN] =
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > +DEAD_ZONE;
        gamepad[GP_BUTTON_0 + button++ - GP_RANGE_BEGIN] =
            SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > +DEAD_ZONE;
    }

    void poll_joystick(SDL_Joystick* joystick, GamepadBuffer& gamepad)
    {
        int axes = SDL_JoystickNumAxes(joystick);
        for (int axis = 0; axis < axes; ++axis) {
            Sint16 value = SDL_JoystickGetAxis(joystick, axis);

            if (value < -DEAD_ZONE) {
                gamepad[(axis % 2 ? GP_UP   : GP_LEFT)  - GP_RANGE_BEGIN] = true;
            }
            else if (value > +DEAD_ZONE) {
                gamepad[(axis % 2 ? GP_DOWN : GP_RIGHT) - GP_RANGE_BEGIN] = true;
            }
        }

        int hats = SDL_JoystickNumHats(joystick);
        for (int hat = 0; hat < hats; ++hat) {
            Uint8 value = SDL_JoystickGetHat(joystick, hat);
            if (value & SDL_HAT_LEFT)  gamepad[GP_LEFT  - GP_RANGE_BEGIN] = true;
            if (value & SDL_HAT_RIGHT) gamepad[GP_RIGHT - GP_RANGE_BEGIN] = true;
            if (value & SDL_HAT_UP)    gamepad[GP_UP    - GP_RANGE_BEGIN] = true;
            if (value & SDL_HAT_DOWN)  gamepad[GP_DOWN  - GP_RANGE_BEGIN] = true;
        }

        int buttons = min<int>(GP_NUM_PER_GAMEPAD - 4, SDL_JoystickNumButtons(joystick));
        for (int button = 0; button < buttons; ++button) {
            if (SDL_JoystickGetButton(joystick, button)) {
                gamepad[GP_BUTTON_0 + button - GP_RANGE_BEGIN] = true;
            }
        }
    }
};

Gosu::Input::Input(void* window)
: pimpl(new Impl(*this, (SDL_Window*)window))
{
}

Gosu::Input::~Input()
{
}

bool Gosu::Input::feed_sdl_event(void* event)
{
    return (pimpl->text_input && pimpl->text_input->feed_sdl_event(event)) ||
        pimpl->feed_sdl_event(static_cast<SDL_Event*>(event));
}

string Gosu::Input::id_to_char(Button btn)
{
    require_sdl_video();

    if (btn.id() > KB_RANGE_END) return "";

    // SDL_GetKeyName returns "Space" for this value, but we want the character value.
    if (btn.id() == KB_SPACE) return " ";

    SDL_Keycode keycode = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(btn.id()));
    if (keycode == SDLK_UNKNOWN) return "";

    const char* name = SDL_GetKeyName(keycode);
    if (name == nullptr) return "";

    u32string codepoints = utf8_to_composed_utc4(name);

    // Filter out names that are more than one logical character.
    if (codepoints.length() != 1) return "";

    // Always return lower case to be consistent with previous versions of Gosu.
    codepoints[0] = utf8proc_tolower(codepoints[0]);
    // Convert back to UTF-8.
    utf8proc_uint8_t utf8_buffer[4];
    auto len = utf8proc_encode_char(codepoints[0], utf8_buffer);
    return string(reinterpret_cast<char*>(utf8_buffer), len);
}

Gosu::Button Gosu::Input::char_to_id(string ch)
{
    require_sdl_video();

    SDL_Keycode keycode = SDL_GetKeyFromName(ch.c_str());
    return keycode == SDLK_UNKNOWN ? NO_BUTTON : Button(SDL_GetScancodeFromKey(keycode));
}

bool Gosu::Input::down(Gosu::Button btn)
{
    if (btn == NO_BUTTON || btn.id() >= NUM_BUTTONS) return false;

    return button_states[btn.id()];
}

double Gosu::Input::axis(Gosu::Button btn)
{
    return axis_states[btn.id() - Gosu::ButtonName::GP_AXIS_LEFT_X];
}

double Gosu::Input::mouse_x() const
{
    return pimpl->mouse_x * pimpl->mouse_scale_x + pimpl->mouse_offset_x;
}

double Gosu::Input::mouse_y() const
{
    return pimpl->mouse_y * pimpl->mouse_scale_y + pimpl->mouse_offset_y;
}

void Gosu::Input::set_mouse_position(double x, double y)
{
    pimpl->set_mouse_position(x, y);
}

void Gosu::Input::set_mouse_factors(double scale_x, double scale_y,
    double black_bar_width, double black_bar_height)
{
    pimpl->mouse_scale_x = scale_x;
    pimpl->mouse_scale_y = scale_y;
    pimpl->mouse_offset_x = -black_bar_width;
    pimpl->mouse_offset_y = -black_bar_height;
}

const Gosu::Touches& Gosu::Input::current_touches() const
{
    // We could use the SDL 2 touch API to implement this.

    static Gosu::Touches none;
    return none;
}

double Gosu::Input::accelerometer_x() const
{
    return 0.0;
}

double Gosu::Input::accelerometer_y() const
{
    return 0.0;
}

double Gosu::Input::accelerometer_z() const
{
    return 0.0;
}

void Gosu::Input::update()
{
    pimpl->update_mouse_position();
    pimpl->poll_gamepads();
    pimpl->dispatch_enqueued_events();
}

Gosu::TextInput* Gosu::Input::text_input() const
{
    return pimpl->text_input;
}

void Gosu::Input::set_text_input(TextInput* text_input)
{
    if (pimpl->text_input && text_input == nullptr) {
        SDL_StopTextInput();
    }
    else if (pimpl->text_input == nullptr && text_input) {
        SDL_StartTextInput();
    }

    pimpl->text_input = text_input;
}

#endif
