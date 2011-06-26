#include <Gosu/Input.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/TR1.hpp>
#include <Gosu/WinUtility.hpp>
#include <cwchar>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

namespace {
    std::tr1::array<bool, Gosu::numButtons> buttons;
}

struct Gosu::Input::Impl
{
    TextInput* textInput;
    Impl() : textInput(0) {}

    HWND window;
    std::tr1::shared_ptr<IDirectInput8> input;

    typedef std::tr1::shared_ptr<IDirectInputDevice8> Device;
    Device keyboard, mouse;
    std::vector<Device> gamepads;

    double mouseX, mouseY;
    double mouseFactorX, mouseFactorY;
    bool swapMouse;

    struct EventInfo
    {
        enum { buttonUp, buttonDown } action;
        unsigned id;
    };
    typedef std::vector<EventInfo> Events;
    Events events;

    static const unsigned inputBufferSize = 32;
    static const int stickRange = 500;
    static const int stickThreshold = 250;

    // For devices with buffered data.
    void forceButton(unsigned id, bool down, bool collectEvent)
    {
        buttons.at(id) = down;

        if (!collectEvent)
            return;

        EventInfo newEvent;
        if (down)
            newEvent.action = EventInfo::buttonDown;
        else
            newEvent.action = EventInfo::buttonUp;
        newEvent.id = id;
        events.push_back(newEvent);
    }

    // For polled devices, or when there's no data.
    void setButton(unsigned id, bool down, bool collectEvent)
    {
        if (buttons.at(id) != down)
            forceButton(id, down, collectEvent);
    }

    static void throwError(const char* action, HRESULT hr)
    {
        std::ostringstream stream;
        stream << "While " << action << ", the following DirectInput "
            << "error occured: " << std::hex << std::setw(8) << hr;
        throw std::runtime_error(stream.str());
        // IMPR: Error string!
    }

    static inline HRESULT check(const char* action, HRESULT hr)
    {
        if (FAILED(hr))
            throwError(action, hr);

        return hr;
    }

    // Callback that adjusts all found axes to [-stickRange, +stickRange].
    static BOOL CALLBACK axisCallback(LPCDIDEVICEOBJECTINSTANCE instance,
        LPVOID userData)
    {
        IDirectInputDevice8* dev = static_cast<IDirectInputDevice8*>(userData);

        DIPROPRANGE range;
        range.diph.dwSize = sizeof(DIPROPRANGE);
        range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        range.diph.dwHow = DIPH_BYID;
        range.diph.dwObj = instance->dwType;
        range.lMin = -stickRange;
        range.lMax = +stickRange;
        dev->SetProperty(DIPROP_RANGE, &range.diph);

        return DIENUM_CONTINUE;
    }

    // Callback that lists all gamepads.
    static BOOL CALLBACK gamepadCallback(LPCDIDEVICEINSTANCE device,
        LPVOID userData)
    {
        Impl* pimpl = static_cast<Impl*>(userData);

        IDirectInputDevice8* gamepad;
        if (FAILED(pimpl->input->CreateDevice(device->guidInstance,
            &gamepad, 0)))
        {
            return DIENUM_CONTINUE;
        }

        if (FAILED(gamepad->SetDataFormat(&c_dfDIJoystick)) ||
            FAILED(gamepad->SetCooperativeLevel(pimpl->window,
                DISCL_EXCLUSIVE | DISCL_FOREGROUND)) ||
            FAILED(gamepad->EnumObjects(axisCallback, gamepad, DIDFT_AXIS)))
        {
            gamepad->Release();
            return DIENUM_CONTINUE;
        }

        pimpl->gamepads.push_back(Win::shareComPtr(gamepad));

        return DIENUM_CONTINUE;
    }

    void updateMousePos()
    {
        POINT pos;
        if (!::GetCursorPos(&pos))
            return;

        Win::check(::ScreenToClient(window, &pos));

        mouseX = pos.x;
        mouseY = pos.y;
    }

    void updateButtons(bool collectEvents)
    {
        DIDEVICEOBJECTDATA data[inputBufferSize];
        DWORD inOut;
        HRESULT hr;
        
        RECT rect;
        ::GetClientRect(window, &rect);
        bool ignoreClicks = mouseX < 0 || mouseX > rect.right || mouseY < 0 || mouseY > rect.bottom;

        inOut = inputBufferSize;
        hr = mouse->GetDeviceData(sizeof data[0], data, &inOut, 0);
        switch(hr)
        {
            case DI_OK:
            case DI_BUFFEROVERFLOW:
            {
                // Everything's ok: Update buttons and fire events.
                for (unsigned i = 0; i < inOut; ++i)
                {
                    bool down = (data[i].dwData & 0x80) != 0 && !ignoreClicks;
                    
                    // No switch statement here because it breaks compilation with MinGW.
                    if (data[i].dwOfs == DIMOFS_BUTTON0)
                    {
                        unsigned id = swapMouse ? msRight : msLeft;
                        setButton(id, down, collectEvents);
                    }
                    else if (data[i].dwOfs == DIMOFS_BUTTON1)
                    {
                        unsigned id = swapMouse ? msLeft : msRight;
                        setButton(id, down, collectEvents);
                    }
                    else if (data[i].dwOfs == DIMOFS_BUTTON2)
                    {
                        setButton(msMiddle, down, collectEvents);
                    }
                    else if (data[i].dwOfs == DIMOFS_Z &&
                        collectEvents && data[i].dwData)
                    {
                        EventInfo event;
                        event.action = EventInfo::buttonDown;
                        if (int(data[i].dwData) < 0)
                            event.id = msWheelDown;
                        else
                            event.id = msWheelUp;
                        events.push_back(event);
                        event.action = EventInfo::buttonUp;
                        events.push_back(event);
                    }
                }
                break;
            }

            case DIERR_NOTACQUIRED:
            case DIERR_INPUTLOST:
            {
                // Cannot fetch new events: Release all buttons.
                for (unsigned id = msRangeBegin; id < msRangeEnd; ++id)
                    setButton(id, false, collectEvents);
                mouse->Acquire();
                break;
            }
        }
        
        keyboard:

        inOut = inputBufferSize;
        hr = keyboard->GetDeviceData(sizeof data[0], data, &inOut, 0);
        switch (hr)
        {
            case DI_OK:
            case DI_BUFFEROVERFLOW:
            {
                for (unsigned i = 0; i < inOut; ++i)
                    forceButton(data[i].dwOfs, (data[i].dwData & 0x80) != 0, collectEvents);
                break;
            }

            case DIERR_NOTACQUIRED:
            case DIERR_INPUTLOST:
            {
                for (unsigned id = kbRangeBegin; id < kbRangeEnd; ++id)
                    setButton(id, false, collectEvents);
                keyboard->Acquire();
                break;
            }
        }

        std::tr1::array<bool, gpNum> gpBuffer = { false };
        for (unsigned gp = 0; gp < gamepads.size(); ++gp)
        {
            gamepads[gp]->Poll();
            
            DIJOYSTATE joy;
            hr = gamepads[gp]->GetDeviceState(sizeof joy, &joy);
            switch (hr)
            {
                case DI_OK:
                {
                    if (joy.lX < -stickThreshold)
                        gpBuffer[gpLeft - gpRangeBegin] = true;
                    else if (joy.lX > stickThreshold)
                        gpBuffer[gpRight - gpRangeBegin] = true;

                    if (joy.lY < -stickThreshold)
                        gpBuffer[gpUp - gpRangeBegin] = true;
                    else if (joy.lY > stickThreshold)
                        gpBuffer[gpDown - gpRangeBegin] = true;

                    for (unsigned id = gpButton0; id < gpRangeEnd; ++id)
                        if (joy.rgbButtons[id - gpButton0])
                            gpBuffer[id - gpRangeBegin] = true;
                    
                    break;
                }

                case DIERR_NOTACQUIRED:
                case DIERR_INPUTLOST:
                {
                    gamepads[gp]->Acquire();

                    break;
                }
            }
        }
        for (unsigned id = gpRangeBegin; id < gpRangeEnd; ++id)
            setButton(id, gpBuffer[id - gpRangeBegin], collectEvents);
    }
};

Gosu::Input::Input(HWND window)
: pimpl(new Impl)
{
    pimpl->window = window;
    pimpl->mouseFactorX = pimpl->mouseFactorY = 1.0;

    // Create the main input object (only necessary for setup).

    IDirectInput8* inputRaw;
    Impl::check("creating the main DirectInput object",
        ::DirectInput8Create(Win::instance(), DIRECTINPUT_VERSION,
            IID_IDirectInput8, reinterpret_cast<void**>(&inputRaw), 0));
    pimpl->input = Win::shareComPtr(inputRaw);


    // Prepare property struct for setting the amount of data to buffer.

    DIPROPDWORD bufferSize;
    bufferSize.diph.dwSize = sizeof(DIPROPDWORD);
    bufferSize.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    bufferSize.diph.dwHow = DIPH_DEVICE;
    bufferSize.diph.dwObj = 0;
    bufferSize.dwData = Impl::inputBufferSize;


    // Set up the system keyboard.

    IDirectInputDevice8* kbRaw;
    Impl::check("creating the keyboard device object",
        pimpl->input->CreateDevice(GUID_SysKeyboard, &kbRaw, 0));
    pimpl->keyboard = Win::shareComPtr(kbRaw);

    Impl::check("setting the keyboard's data format",
        kbRaw->SetDataFormat(&c_dfDIKeyboard));
    Impl::check("setting the keyboard's cooperative level",
        kbRaw->SetCooperativeLevel(window,
            DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
    Impl::check("setting the keyboard's buffer size",
        kbRaw->SetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph));

    kbRaw->Acquire();

    
    // Set up the system mouse.

    IDirectInputDevice8* mouseRaw;
    Impl::check("creating the mouse device object",
        pimpl->input->CreateDevice(GUID_SysMouse, &mouseRaw, 0));
    pimpl->mouse = Win::shareComPtr(mouseRaw);

    Impl::check("setting the mouse's data format",
        mouseRaw->SetDataFormat(&c_dfDIMouse));
    Impl::check("setting the mouse's cooperative level",
        mouseRaw->SetCooperativeLevel(window,
            DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));
    Impl::check("setting the mouse's buffer size",
        mouseRaw->SetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph));

    mouseRaw->Acquire();

    pimpl->swapMouse = ::GetSystemMetrics(SM_SWAPBUTTON) != 0;


    // Set up all gamepads.

    pimpl->input->EnumDevices(DI8DEVCLASS_GAMECTRL, Impl::gamepadCallback,
        pimpl.get(), DIEDFL_ATTACHEDONLY);


    // Get into a usable default state.

    pimpl->mouseX = pimpl->mouseY = 0;
    pimpl->updateMousePos();
    buttons.assign(false);
}

Gosu::Input::~Input()
{
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    SHORT vkey = ::VkKeyScan(/*std::*/towlower(ch));

    // No key found?
    if (HIBYTE(vkey) == static_cast<unsigned char>(-1) &&
        LOBYTE(vkey) == static_cast<unsigned char>(-1))
    {
        return noButton;
    }

    // Key needs special modifier keys?
    if (HIBYTE(vkey) != 0)
        return noButton;

    // Now try to translate the virtual key code into a scan code.
    return Button(::MapVirtualKey(vkey, 0));
}

wchar_t Gosu::Input::idToChar(Gosu::Button btn)
{
    // Only translate keyboard ids.
    if (btn.id() > 255)
        return 0;

    // Special case...?
    if (btn.id() == kbSpace)
        return L' ';

    // Try to get the key name.
    // (Three elements so too-long names will make GKNT return 3 and we'll know.)
    wchar_t buf[3];
    if (::GetKeyNameText(btn.id() << 16, buf, 3) == 1)
        return /*std::*/towlower(buf[0]);

    return 0;
}

bool Gosu::Input::down(Button btn) const
{
    // The invalid button is never pressed (but can be passed to this function).
    if (btn == noButton || btn.id() >= numButtons)
        return false;
    
    return buttons.at(btn.id());
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX * pimpl->mouseFactorX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY * pimpl->mouseFactorY;
}

void Gosu::Input::setMousePosition(double x, double y)
{
    POINT pos = { x / pimpl->mouseFactorX, y / pimpl->mouseFactorY };
    ::ClientToScreen(pimpl->window, &pos);
    ::SetCursorPos(pos.x, pos.y);
    pimpl->updateMousePos();
}

void Gosu::Input::setMouseFactors(double factorX, double factorY)
{
    pimpl->mouseFactorX = factorX;
    pimpl->mouseFactorY = factorY;
}

const Gosu::Touches& Gosu::Input::currentTouches() const
{
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
    pimpl->updateMousePos();
    pimpl->updateButtons(true);
    Impl::Events events;
    events.swap(pimpl->events);
    for (unsigned i = 0; i < events.size(); ++i)
    {
        if (events[i].action == Impl::EventInfo::buttonDown)
        {
            if (onButtonDown)
                onButtonDown(Button(events[i].id));
        }
        else
        {
            if (onButtonUp)
                onButtonUp(Button(events[i].id));
        }
    }
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return pimpl->textInput;
}

void Gosu::Input::setTextInput(TextInput* textInput)
{
    pimpl->textInput = textInput;
}
