#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <Gosu/Utility.hpp>
#include <SDL.h>
#include <cwctype>
#include <cstdlib>
#include <algorithm>
#include <array>

namespace
{
    void cleanup();

    void requireSDLVideo()
    {
        static bool initialized = false;
        if (!initialized)
        {
            SDL_InitSubSystem(SDL_INIT_VIDEO);
            initialized = true;
            std::atexit(cleanup);
        }
    }
    
    void cleanup()
    {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }

    std::array<bool, Gosu::numButtons> buttonStates = { { false } };
}

struct Gosu::Input::Impl
{
    Input& input;
    SDL_Window* window;
    TextInput* textInput;
    double mouseX, mouseY;
    double mouseFactorX, mouseFactorY;
    double mouseOffsetX, mouseOffsetY;
    
    Impl(Input& input, SDL_Window* window)
    : input(input), window(window), textInput(nullptr)
    {
        mouseFactorX = mouseFactorY = 1;
        mouseOffsetX = mouseOffsetY = 0;
    }
    
    void updateMousePosition()
    {
    // Do not use GetGlobalMouseState on Linux for now to prevent this bug:
    // https://github.com/gosu/gosu/issues/326
    // Once SDL 2.0.5 has been released, we can use this function as a workaround:
    // https://wiki.libsdl.org/SDL_GetWindowBordersSize
    #if SDL_VERSION_ATLEAST(2, 0, 4) && !defined(GOSU_IS_X)
        int x, y, windowX, windowY;
        SDL_GetWindowPosition(window, &windowX, &windowY);
        SDL_GetGlobalMouseState(&x, &y);
        mouseX = x - windowX, mouseY = y - windowY;
    #else
        int x, y;
        SDL_GetMouseState(&x, &y);
        mouseX = x, mouseY = y;
    #endif
    }
    
    void setMousePosition(double x, double y)
    {
        SDL_WarpMouseInWindow(window,
            (x - mouseOffsetX) / mouseFactorX, (y - mouseOffsetY) / mouseFactorY);

    #if SDL_VERSION_ATLEAST(2, 0, 4) && !defined(GOSU_IS_X)
        // On systems where we have a working GetGlobalMouseState, we can warp the mouse and
        // retrieve its position directly afterwards.
        updateMousePosition();
    #else
        // Otherwise, we have to assume that setting the position worked, because if we update the
        // mouse position now, we'll get the previous position.
        mouseX = x, mouseY = y;
    #endif
    }
    
    void enqueueEvent(int id, bool down)
    {
        eventQueue.push_back(down ? id : ~id);
    }
    
    void dispatchEnqueuedEvents()
    {
        for (int event : eventQueue) {
            bool down = (event >= 0);
            Button button(down ? event : ~event);
            
            buttonStates[button.id()] = down;
            if (down && input.onButtonDown) {
                input.onButtonDown(button);
            }
            else if (!down && input.onButtonUp) {
                input.onButtonUp(button);
            }
        }
        eventQueue.clear();
    }
    
    void initializeGamepads()
    {
        SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);

        int numGamepads = std::min<int>(Gosu::numGamepads, SDL_NumJoysticks());
        
        for (int i = 0; i < numGamepads; ++i) {
            // Prefer the SDL_GameController API...
            if (SDL_IsGameController(i)) {
                SDL_GameController *gameController = SDL_GameControllerOpen(i);
                if (gameController) {
                    gameControllers.push_back(gameController);
                    continue;
                }
            }
            // ...but fall back on the good, old SDL_Joystick API :)
            SDL_Joystick *joystick = SDL_JoystickOpen(i);
            if (joystick) {
                joysticks.push_back(joystick);
            }
        }
    }
    
    void releaseGamepads()
    {
        std::for_each(joysticks.begin(), joysticks.end(), &SDL_JoystickClose);
        joysticks.clear();
        std::for_each(gameControllers.begin(), gameControllers.end(), &SDL_GameControllerClose);
        gameControllers.clear();
        
        SDL_QuitSubSystem(SDL_INIT_GAMECONTROLLER);
    }
    
    typedef std::array<bool, gpNumPerGamepad> GamepadBuffer;
    
    void pollGamepads()
    {
        // This gamepad is an OR-ed version of all the other gamepads. If button
        // 3 is pressed on any attached gamepad, down(gpButton3) will return
        // true. This is handy for singleplayer games.
        GamepadBuffer anyGamepad = { false };
        
        std::size_t availableGamepads = gameControllers.size() + joysticks.size();
        
        for (int i = 0; i < availableGamepads; ++i) {
            GamepadBuffer currentGamepad = { false };
            
            // Poll data from SDL, using either of two API interfaces.
            if (i < gameControllers.size()) {
                SDL_GameController *gameController = gameControllers[i];
                pollGameController(gameController, currentGamepad);
            }
            else {
                SDL_Joystick *joystick = joysticks[i - gameControllers.size()];
                pollJoystick(joystick, currentGamepad);
            }
            
            // Now at the same time, enqueue all events for this particular
            // gamepad, and OR the keyboard state into anyGamepad.
            int offset = gpRangeBegin + gpNumPerGamepad * (i + 1);
            for (int j = 0; j < currentGamepad.size(); ++j) {
                anyGamepad[j] = anyGamepad[j] || currentGamepad[j];
                
                if (currentGamepad[j] && !buttonStates[j + offset]) {
                    buttonStates[j + offset] = true;
                    enqueueEvent(j + offset, true);
                }
                else if (!currentGamepad[j] && buttonStates[j + offset]) {
                    buttonStates[j + offset] = false;
                    enqueueEvent(j + offset, false);
                }
            }
        }
        
        // And lastly, enqueue events for the virtual "any" gamepad.
        for (int j = 0; j < anyGamepad.size(); ++j) {
            if (anyGamepad[j] && !buttonStates[j + gpRangeBegin]) {
                buttonStates[j + gpRangeBegin] = true;
                enqueueEvent(j + gpRangeBegin, true);
            }
            else if (!anyGamepad[j] && buttonStates[j + gpRangeBegin]) {
                buttonStates[j + gpRangeBegin] = false;
                enqueueEvent(j + gpRangeBegin, false);
            }
        }
    }
    
private:
    // For button down event: Button name value (>= 0)
    // For button up event: ~Button name value (< 0)
    std::vector<int> eventQueue;
    
    std::vector<SDL_Joystick*> joysticks;
    std::vector<SDL_GameController*> gameControllers;
    
    // SDL returns axis values in the range -2^15 through 2^15-1, so we consider
    // -2^14 through 2^14 the dead zone.
    
    enum { DEAD_ZONE = (1 << 14) };
    
    void pollGameController(SDL_GameController *gameController, GamepadBuffer& gamepad)
    {
        gamepad[gpLeft - gpRangeBegin] =
            SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_LEFT) ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTX) < -DEAD_ZONE ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTX) < -DEAD_ZONE;
        
        gamepad[gpRight - gpRangeBegin] =
            SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTX) > +DEAD_ZONE ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTX) > +DEAD_ZONE;
        
        gamepad[gpUp - gpRangeBegin] =
            SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_UP) ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTY) < -DEAD_ZONE ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTY) < -DEAD_ZONE;
        
        gamepad[gpDown - gpRangeBegin] =
            SDL_GameControllerGetButton(gameController, SDL_CONTROLLER_BUTTON_DPAD_DOWN) ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_LEFTY) > +DEAD_ZONE ||
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_RIGHTY) > +DEAD_ZONE;
        
        int button = 0;
        for (button; button < SDL_CONTROLLER_BUTTON_DPAD_UP; ++button) {
            gamepad[gpButton0 + button - gpRangeBegin] =
                SDL_GameControllerGetButton(gameController, (SDL_GameControllerButton)button);
        }
        gamepad[gpButton0 + button++ - gpRangeBegin] =
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > +DEAD_ZONE;
        gamepad[gpButton0 + button++ - gpRangeBegin] =
            SDL_GameControllerGetAxis(gameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > +DEAD_ZONE;
    }
    
    void pollJoystick(SDL_Joystick *joystick, GamepadBuffer& gamepad)
    {
        int axes = SDL_JoystickNumAxes(joystick),
            hats = SDL_JoystickNumHats(joystick),
            buttons = std::min<int>(gpNumPerGamepad - 4, SDL_JoystickNumButtons(joystick));
        
        for (int axis = 0; axis < axes; ++axis) {
            Sint16 value = SDL_JoystickGetAxis(joystick, axis);
            
            if (value < -DEAD_ZONE) {
                if (axis % 2 == 0)
                    gamepad[gpLeft - gpRangeBegin] = true;
                else
                    gamepad[gpUp - gpRangeBegin] = true;
            }
            else if (value > +DEAD_ZONE) {
                if (axis % 2 == 0)
                    gamepad[gpRight - gpRangeBegin] = true;
                else
                    gamepad[gpDown - gpRangeBegin] = true;
            }
        }
        
        for (int hat = 0; hat < hats; ++hat) {
            Uint8 value = SDL_JoystickGetHat(joystick, hat);
            
            if (value & SDL_HAT_LEFT)
                gamepad[gpLeft - gpRangeBegin] = true;
            if (value & SDL_HAT_RIGHT)
                gamepad[gpRight - gpRangeBegin] = true;
            if (value & SDL_HAT_UP)
                gamepad[gpUp - gpRangeBegin] = true;
            if (value & SDL_HAT_DOWN)
                gamepad[gpDown - gpRangeBegin] = true;
        }
        
        for (int button = 0; button < buttons; ++button) {
            if (SDL_JoystickGetButton(joystick, button)) {
                gamepad[gpButton0 + button - gpRangeBegin] = true;
            }
        }
    }
};

Gosu::Input::Input(void* window)
: pimpl(new Impl(*this, (SDL_Window*)window))
{
    requireSDLVideo();
    
    pimpl->initializeGamepads();
}

Gosu::Input::~Input()
{
    pimpl->releaseGamepads();
}

bool Gosu::Input::feedSDLEvent(void* event)
{
    const SDL_Event* e = static_cast<SDL_Event*>(event);
    
    if (pimpl->textInput && pimpl->textInput->feedSDLEvent(event)) {
        return true;
    }
    
    switch (e->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            if (e->key.repeat == 0 && e->key.keysym.scancode <= kbRangeEnd) {
                pimpl->enqueueEvent(e->key.keysym.scancode, e->type == SDL_KEYDOWN);
                return true;
            }
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            if (e->button.button >= 1 && e->button.button <= 3) {
                pimpl->enqueueEvent(msLeft + e->button.button - 1, e->type == SDL_MOUSEBUTTONDOWN);
                return true;
            }
            break;
        }
        case SDL_MOUSEWHEEL: {
            if (e->wheel.y > 0) {
                pimpl->enqueueEvent(msWheelUp, true);
                pimpl->enqueueEvent(msWheelUp, false);
                return true;
            }
            else if (e->wheel.y < 0) {
                pimpl->enqueueEvent(msWheelDown, true);
                pimpl->enqueueEvent(msWheelDown, false);
                return true;
            }
            break;
        }
    }
    return false;
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    requireSDLVideo();
    
    if (btn.id() > kbRangeEnd)
        return 0;
    
    // SDL_GetKeyName would return "Space" for this value.
    if (btn.id() == kbSpace)
        return L' ';
    
    SDL_Keycode keycode = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(btn.id()));
    if (keycode == SDLK_UNKNOWN)
        return 0;
    
    const char* name = SDL_GetKeyName(keycode);
    if (name == nullptr)
        return 0;
    
    std::wstring wname = utf8ToWstring(name);
    if (wname.length() != 1)
        return 0;
    
    // Convert to lower case to be consistent with previous versions of Gosu.
    // Also, German umlauts are already reported in lower-case by the SDL, so
    // this makes everything a little more consistent.
    //
    // This should handle Turkish i/I just fine because it uses the current
    // locale, but if we ever receive bug reports from Turkish users, they are
    // likely caused by a combination of this line and an invalid locale :)
    return std::towlower(wname[0]);
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    requireSDLVideo();
    
    std::wstring string(1, ch);
    SDL_Keycode keycode = SDL_GetKeyFromName(wstringToUTF8(string).c_str());
    
    if (keycode == SDLK_UNKNOWN) {
        return noButton;
    }
    else {
        return Button(SDL_GetScancodeFromKey(keycode));
    }
}

bool Gosu::Input::down(Gosu::Button btn)
{
    if (btn == noButton || btn.id() >= numButtons)
        return false;
    
    return buttonStates[btn.id()];
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX * pimpl->mouseFactorX + pimpl->mouseOffsetX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY * pimpl->mouseFactorY + pimpl->mouseOffsetY;
}

void Gosu::Input::setMousePosition(double x, double y)
{
    pimpl->setMousePosition(x, y);
}

void Gosu::Input::setMouseFactors(double factorX, double factorY,
    double blackBarWidth, double blackBarHeight)
{
    pimpl->mouseFactorX = factorX;
    pimpl->mouseFactorY = factorY;
    pimpl->mouseOffsetX = -blackBarWidth;
    pimpl->mouseOffsetY = -blackBarHeight;
}

const Gosu::Touches& Gosu::Input::currentTouches() const
{
    // Note: We can actually use the SDL's touch API to implement this, even on OS X! Neat.
    
    static Gosu::Touches none;
    return none;
}

double Gosu::Input::accelerometerX() const
{
    return 0.0;
}

double Gosu::Input::accelerometerY() const
{
    return 0.0;
}

double Gosu::Input::accelerometerZ() const
{
    return 0.0;
}

void Gosu::Input::update()
{
    pimpl->updateMousePosition();
    pimpl->dispatchEnqueuedEvents();
    pimpl->pollGamepads();
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return pimpl->textInput;
}

void Gosu::Input::setTextInput(TextInput* textInput)
{
    if (pimpl->textInput && textInput == nullptr) {
        SDL_StopTextInput();
    }
    else if (pimpl->textInput == nullptr && textInput) {
        SDL_StartTextInput();
    }
    
    pimpl->textInput = textInput;
}
