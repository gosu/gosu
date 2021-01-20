#include <Gosu/Platform.hpp>
#if !defined(GOSU_IS_IPHONE)

#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <Gosu/Utility.hpp>

#include <SDL.h>
#include <utf8proc.h>

#include <cwctype>
#include <cstdlib>
#include <algorithm>
#include <array>
#include <mutex>
using namespace std;

static void require_sdl_video()
{
    static std::once_flag initialized;

    std::call_once(initialized, [] {
        SDL_InitSubSystem(SDL_INIT_VIDEO);
    });
}

static const unsigned NUM_BUTTONS_PER_GAMEPAD =
    (Gosu::GP_RANGE_END - Gosu::GP_RANGE_BEGIN + 1 - 4) / (Gosu::NUM_GAMEPADS + 1) - 3;
static const unsigned NUM_AXES_PER_GAMEPAD = Gosu::NUM_AXES / (Gosu::NUM_GAMEPADS + 1);
static const unsigned NUM_BUTTONS = Gosu::GP_RANGE_END + 1;

static array<bool, NUM_BUTTONS> button_states = {false};
static array<double, NUM_AXES_PER_GAMEPAD * (Gosu::NUM_GAMEPADS + 1)> axis_states = {0.0};
static vector<shared_ptr<SDL_Joystick>> open_joysticks;
static vector<shared_ptr<SDL_GameController>> open_game_controllers;
// Stores joystick instance id or -1 if empty
static array<int, Gosu::NUM_GAMEPADS> gamepad_slots = {-1, -1, -1, -1};

struct Gosu::Input::Impl
{
    struct InputEvent
    {
        enum {
            ButtonUp,
            ButtonDown,
            GamepadConnected,
            GamepadDisconnected
        } type;
        int id = -1;
        int gamepad_instance_id = -1;
    };

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
    }

    ~Impl()
    {
        open_joysticks.clear();
        open_game_controllers.clear();

        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }

    void update_mouse_position()
    {
        int x, y, window_x, window_y;
        SDL_GetWindowPosition(window, &window_x, &window_y);
        SDL_GetGlobalMouseState(&x, &y);
        mouse_x = x - window_x;
        mouse_y = y - window_y;
    }

    void set_mouse_position(double x, double y)
    {
        SDL_WarpMouseInWindow(window,
                              static_cast<int>((x - mouse_offset_x) / mouse_scale_x),
                              static_cast<int>((y - mouse_offset_y) / mouse_scale_y));

    #if !defined(GOSU_IS_X)
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
                if (e->key.repeat == 0 && e->key.keysym.scancode <= static_cast<int>(KB_RANGE_END)) {
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
            case SDL_JOYDEVICEADDED: {
                if (available_gamepad_slot_index() == -1) {
                    break;
                }
                int gamepad_slot = -1;
                int joystick_instance_id = -1;

                // Loop through attached gamepads as e->jdevice.which cannot be trusted (always 0)
                for (int i = 0; i < SDL_NumJoysticks(); i++) {
                    // Prefer the SDL_GameController API...
                    if (SDL_IsGameController(i)) {
                        if (SDL_GameController *game_controller = SDL_GameControllerOpen(i)) {
                            gamepad_slot = available_gamepad_slot_index();
                            joystick_instance_id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(game_controller));
                            if (gamepad_instance_id_is_known(joystick_instance_id)) {
                                continue;
                            }
                            open_game_controllers.emplace_back(
                                shared_ptr<SDL_GameController>(game_controller, SDL_GameControllerClose)
                            );
                        }
                    }
                    // ...but fall back on the good, old SDL_Joystick API.
                    else if (SDL_Joystick *joystick = SDL_JoystickOpen(i)) {
                        gamepad_slot = available_gamepad_slot_index();
                        joystick_instance_id = SDL_JoystickInstanceID(joystick);
                        if (gamepad_instance_id_is_known(joystick_instance_id)) {
                            continue;
                        }
                        open_joysticks.emplace_back(
                            shared_ptr<SDL_Joystick>(joystick, SDL_JoystickClose)
                        );
                    }

                    // Reserve gamepad slot and issue gamepad connection event
                    if (gamepad_slot >= 0 && joystick_instance_id >= 0) {
                        gamepad_slots[gamepad_slot] = joystick_instance_id;
                        enqueue_gamepad_connection_event(gamepad_slot, true, -1);
                    }
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

    double scale_axis(double value)
    {
        return value >= 0 ? value / SDL_JOYSTICK_AXIS_MAX : -value / SDL_JOYSTICK_AXIS_MIN;
    }

    // returns the gamepad slot index (0..NUM_GAMEPADS - 1) for the joystick instance id or -1 if not found
    int gamepad_slot_index(int joystick_instance_id) const
    {
        for (int i = 0; i < gamepad_slots.size(); i++) {
            if (gamepad_slots[i] == joystick_instance_id) {
                return i;
            }
        }

        return -1;
    }

    // returns first available gamepad slot or -1 if non are available
    int available_gamepad_slot_index() const
    {
        for (int i = 0; i < gamepad_slots.size(); i++) {
            if (gamepad_slots[i] == -1) {
                return i;
            }
        }

        return -1;
    }

    // returns whether the gamepad using the joystick instance id is known
    bool gamepad_instance_id_is_known(int id)
    {
        for (int j = 0; j < gamepad_slots.size(); j++) {
            if (gamepad_slots[j] == id) {
                return true;
            }
        }

        return false;
    }

    // frees the gamepad slot associated with this joystick instance id
    // and frees the SDL_GameController/SDL_Joystick
    void free_gamepad_slot(int instance_id)
    {
        int index = gamepad_slot_index(instance_id);

        for (int i = 0; i < open_game_controllers.size(); i++) {
            SDL_Joystick* joystick = SDL_GameControllerGetJoystick(open_game_controllers[i].get());
            if (SDL_JoystickInstanceID(joystick) == instance_id) {
                open_game_controllers.erase(open_game_controllers.begin() + i);
                gamepad_slots[index] = -1;
                return;
            }
        }

        for (int i = 0; i < open_joysticks.size(); i++) {
            if (SDL_JoystickInstanceID(open_joysticks[i].get()) == instance_id) {
                open_joysticks.erase(open_joysticks.begin() + i);
                gamepad_slots[index] = -1;
                return;
            }
        }
    }

    struct GamepadBuffer
    {
        array<bool, NUM_BUTTONS_PER_GAMEPAD> buttons = { false };
        array<bool, 4> directions = { false };
        array<double, NUM_AXES_PER_GAMEPAD> axes = { 0.0 };
    };

    void poll_gamepads()
    {
        // This gamepad is an OR-ed version of all the other gamepads.
        // If button3 is pressed on any attached gamepad, down(GP_BUTTON_3) will return true.
        // This is handy for singleplayer games where you don't care which gamepad that player uses.
        GamepadBuffer any_gamepad;

        // Reset all axes values, they will be recalculated below.
        axis_states.fill(0.0);

        for (int i = 0; i < gamepad_slots.size(); ++i) {
            if (gamepad_slots[i] == -1) {
                continue;
            }

            // Poll data from SDL, using either of two API interfaces.
            GamepadBuffer current_gamepad;
            if (SDL_GameController* game_controller = SDL_GameControllerFromInstanceID(gamepad_slots[i])) {
                current_gamepad = poll_game_controller(game_controller);
            }
            else {
                current_gamepad = poll_joystick(SDL_JoystickFromInstanceID(gamepad_slots[i]));
            }

            int axis_offset = NUM_AXES_PER_GAMEPAD * (i + 1);
            for (int a = 0; a < NUM_AXES_PER_GAMEPAD; ++a) {
                // Transfer the axes values into the global axis_state array..
                axis_states[a + axis_offset] = current_gamepad.axes[a];
                // Also transfer the gamepad-specific axes values into the "any gamepad" slots.
                // (Values with a higher amplitude override smaller ones.)
                if (abs(current_gamepad.axes[a]) > abs(axis_states[a])) {
                    axis_states[a] = current_gamepad.axes[a];
                }
            }

            // Now at the same time, enqueue all events for this particular
            // gamepad, and OR the keyboard state into any_gamepad.
            int offset = GP_RANGE_BEGIN + NUM_BUTTONS_PER_GAMEPAD * (i + 1);
            for (int j = 0; j < current_gamepad.buttons.size(); ++j) {
                any_gamepad.buttons[j] = any_gamepad.buttons[j] || current_gamepad.buttons[j];

                if (current_gamepad.buttons[j] && !button_states[j + offset]) {
                    button_states[j + offset] = true;
                    enqueue_event(j + offset, true);
                }
                else if (!current_gamepad.buttons[j] && button_states[j + offset]) {
                    button_states[j + offset] = false;
                    enqueue_event(j + offset, false);
                }
            }
            int direction_offset = GP_LEFT + 4 * (i + 1);
            for (int d = 0; d < 4; ++d) {
                any_gamepad.directions[d] = any_gamepad.directions[d] || current_gamepad.directions[d];

                if (current_gamepad.directions[d] && !button_states[d + direction_offset]) {
                    button_states[d + direction_offset] = true;
                    enqueue_event(d + direction_offset, true);
                }
                else if (!current_gamepad.directions[d] && button_states[d + direction_offset]) {
                    button_states[d + direction_offset] = false;
                    enqueue_event(d + direction_offset, false);
                }
            }
        }

        // And lastly, enqueue events for the virtual "any" gamepad.
        for (int j = 0; j < any_gamepad.buttons.size(); ++j) {
            if (any_gamepad.buttons[j] && !button_states[j + GP_RANGE_BEGIN]) {
                button_states[j + GP_RANGE_BEGIN] = true;
                enqueue_event(j + GP_RANGE_BEGIN, true);
            }
            else if (!any_gamepad.buttons[j] && button_states[j + GP_RANGE_BEGIN]) {
                button_states[j + GP_RANGE_BEGIN] = false;
                enqueue_event(j + GP_RANGE_BEGIN, false);
            }
        }
        for (int d = 0; d < 4; ++d) {
            if (any_gamepad.directions[d] && !button_states[d + GP_LEFT]) {
                button_states[d + GP_LEFT] = true;
                enqueue_event(d + GP_LEFT, true);
            }
            else if (!any_gamepad.directions[d] && button_states[d + GP_LEFT]) {
                button_states[d + GP_LEFT] = false;
                enqueue_event(d + GP_LEFT, false);
            }
        }
    }

    void dispatch_enqueued_events()
    {
        for (const InputEvent& event : event_queue) {
            switch (event.type) {
                case InputEvent::ButtonDown:
                    button_states[event.id] = true;
                    if (input.on_button_down) {
                        input.on_button_down(Button(event.id));
                    }
                    break;
                case InputEvent::ButtonUp:
                    button_states[event.id] = false;
                    if (input.on_button_up) {
                        input.on_button_up(Button(event.id));
                    }
                    break;
                case InputEvent::GamepadConnected:
                    if (input.on_gamepad_connected) {
                        input.on_gamepad_connected(event.id);
                    }
                    break;
                case InputEvent::GamepadDisconnected:
                    if (input.on_gamepad_disconnected) {
                        input.on_gamepad_disconnected(event.id);
                    }
                    free_gamepad_slot(event.gamepad_instance_id);
                    break;
            }
        }
        event_queue.clear();
    }

private:
    // For button down event: Button name value (>= 0)
    // For button up event: ~Button name value (< 0)
    vector<InputEvent> event_queue;

    void enqueue_event(unsigned id, bool down)
    {
        InputEvent event;
        event.type = down ? InputEvent::ButtonDown : InputEvent::ButtonUp;
        event.id = id;
        event_queue.push_back(event);
    }

    void enqueue_gamepad_connection_event(int gamepad_index_id, bool connected, int instance_id)
    {
        InputEvent event;
        event.type = connected ? InputEvent::GamepadConnected : InputEvent::GamepadDisconnected;
        event.id = gamepad_index_id;
        event.gamepad_instance_id = instance_id;
        event_queue.push_back(event);
    }

    GamepadBuffer poll_game_controller(SDL_GameController* controller)
    {
        GamepadBuffer gamepad;

        // Poll axes first.
        gamepad.axes[GP_LEFT_STICK_X_AXIS] =
            scale_axis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX));
        gamepad.axes[GP_LEFT_STICK_Y_AXIS] =
            scale_axis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY));
        gamepad.axes[GP_RIGHT_STICK_X_AXIS] =
            scale_axis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX));
        gamepad.axes[GP_RIGHT_STICK_Y_AXIS] =
            scale_axis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY));
        gamepad.axes[GP_LEFT_TRIGGER_AXIS] =
            scale_axis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
        gamepad.axes[GP_RIGHT_TRIGGER_AXIS] =
            scale_axis(SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));

        gamepad.buttons[GP_DPAD_LEFT - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        gamepad.buttons[GP_DPAD_RIGHT - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        gamepad.buttons[GP_DPAD_UP - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
        gamepad.buttons[GP_DPAD_DOWN - GP_RANGE_BEGIN] =
            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);

        int button = 0;
        for (; button < SDL_CONTROLLER_BUTTON_DPAD_UP; ++button) {
            gamepad.buttons[GP_BUTTON_0 + button - GP_RANGE_BEGIN] =
                SDL_GameControllerGetButton(controller, (SDL_GameControllerButton) button);
        }
        // Represent the triggers as buttons in addition to them being axes.
        gamepad.buttons[GP_BUTTON_0 + button++ - GP_RANGE_BEGIN] =
            gamepad.axes[GP_LEFT_TRIGGER_AXIS] >= 0.5;
        gamepad.buttons[GP_BUTTON_0 + button++ - GP_RANGE_BEGIN] =
            gamepad.axes[GP_RIGHT_TRIGGER_AXIS] >= 0.5;

        merge_directions(gamepad);
        return gamepad;
    }

    GamepadBuffer poll_joystick(SDL_Joystick* joystick)
    {
        GamepadBuffer gamepad;

        // Just guess that the first four axes are equivalent to two analog sticks.
        int axes = SDL_JoystickNumAxes(joystick);
        if (axes > 0) {
            gamepad.axes[GP_LEFT_STICK_X_AXIS] =
                scale_axis(SDL_JoystickGetAxis(joystick, 0));
        }
        if (axes > 1) {
            gamepad.axes[GP_LEFT_STICK_Y_AXIS] =
                scale_axis(SDL_JoystickGetAxis(joystick, 1));
        }
        if (axes > 2) {
            gamepad.axes[GP_RIGHT_STICK_X_AXIS] =
                scale_axis(SDL_JoystickGetAxis(joystick, 2));
        }
        if (axes > 3) {
            gamepad.axes[GP_RIGHT_STICK_Y_AXIS] =
                scale_axis(SDL_JoystickGetAxis(joystick, 3));
        }

        // All hats are merged into the DPad.
        int hats = SDL_JoystickNumHats(joystick);
        for (int hat = 0; hat < hats; ++hat) {
            Uint8 value = SDL_JoystickGetHat(joystick, hat);
            if (value & SDL_HAT_LEFT)  gamepad.buttons[GP_DPAD_LEFT  - GP_RANGE_BEGIN] = true;
            if (value & SDL_HAT_RIGHT) gamepad.buttons[GP_DPAD_RIGHT - GP_RANGE_BEGIN] = true;
            if (value & SDL_HAT_UP)    gamepad.buttons[GP_DPAD_UP    - GP_RANGE_BEGIN] = true;
            if (value & SDL_HAT_DOWN)  gamepad.buttons[GP_DPAD_DOWN  - GP_RANGE_BEGIN] = true;
        }

        int buttons = min<int>(NUM_BUTTONS_PER_GAMEPAD - 4, SDL_JoystickNumButtons(joystick));
        for (int button = 0; button < buttons; ++button) {
            if (SDL_JoystickGetButton(joystick, button)) {
                gamepad.buttons[GP_BUTTON_0 + button - GP_RANGE_BEGIN] = true;
            }
        }

        merge_directions(gamepad);
        return gamepad;
    }

    void merge_directions(GamepadBuffer& gamepad)
    {
        gamepad.directions[0] =
            gamepad.buttons[GP_DPAD_LEFT - GP_RANGE_BEGIN] ||
            gamepad.axes[GP_LEFT_STICK_X_AXIS] <= -0.5 ||
            gamepad.axes[GP_RIGHT_STICK_X_AXIS] <= -0.5;
        gamepad.directions[1] =
            gamepad.buttons[GP_DPAD_RIGHT - GP_RANGE_BEGIN] ||
            gamepad.axes[GP_LEFT_STICK_X_AXIS] >= +0.5 ||
            gamepad.axes[GP_RIGHT_STICK_X_AXIS] >= +0.5;
        gamepad.directions[2] =
            gamepad.buttons[GP_DPAD_UP - GP_RANGE_BEGIN] ||
            gamepad.axes[GP_LEFT_STICK_Y_AXIS] <= -0.5 ||
            gamepad.axes[GP_RIGHT_STICK_Y_AXIS] <= -0.5;
        gamepad.directions[3] =
            gamepad.buttons[GP_DPAD_DOWN - GP_RANGE_BEGIN] ||
            gamepad.axes[GP_LEFT_STICK_Y_AXIS] >= +0.5 ||
            gamepad.axes[GP_RIGHT_STICK_Y_AXIS] >= +0.5;
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

    if (btn > KB_RANGE_END) return "";

    // SDL_GetKeyName returns "Space" for this value, but we want the character value.
    if (btn == KB_SPACE) return " ";

    SDL_Keycode keycode = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(btn));
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

std::string Gosu::Input::button_name(Button btn)
{
    require_sdl_video();

    SDL_Keycode keycode = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(btn));
    return SDL_GetKeyName(keycode);
}

std::string Gosu::Input::gamepad_name(int index)
{
    if (index < 0 || index > gamepad_slots.size() - 1) {
        return "";
    }

    int instance_id = gamepad_slots[index];

    if (instance_id == -1) {
        return "";
    }

    if (SDL_GameController* game_controller = SDL_GameControllerFromInstanceID(instance_id)) {
        return SDL_GameControllerName(game_controller);
    }
    else if (SDL_Joystick* joystick = SDL_JoystickFromInstanceID(instance_id)) {
        return SDL_JoystickName(joystick);
    }

    return "";
}

bool Gosu::Input::down(Gosu::Button btn)
{
    if (btn == NO_BUTTON || btn >= NUM_BUTTONS) return false;

    return button_states[btn];
}

double Gosu::Input::axis(Gosu::Button btn)
{
    unsigned axis_id = btn - GP_LEFT_STICK_X_AXIS;

    if (axis_id >= axis_states.size()) {
        throw std::out_of_range("Invalid axis ID: " + std::to_string(btn));
    }

    return axis_states[axis_id];
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
