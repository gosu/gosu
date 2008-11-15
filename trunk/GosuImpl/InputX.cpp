#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <Gosu/Utility.hpp>
#include <vector>
#include <map>

struct Gosu::Input::Impl
{
    TextInput* textInput;
    std::vector< ::XEvent> eventList;
    std::map<unsigned int, bool> keyMap;
    double mouseX, mouseY, mouseFactorX, mouseFactorY;
    ::Display* display;
    Impl() : textInput(0) {}
};

Gosu::Input::Input(::Display* dpy)
    : pimpl(new Impl)
{
    // IMPR: Get current position?
    pimpl->mouseX = pimpl->mouseY = 0;
    pimpl->mouseFactorX = pimpl->mouseFactorY = 1.0;
    pimpl->display = dpy;

    XAutoRepeatOff(dpy);
}

Gosu::Input::~Input()
{
}

bool Gosu::Input::feedXEvent(::XEvent& event, Gosu::Window* window)
{
    if(event.type == VisibilityNotify ||
       event.type == CirculateRequest ||
       event.type == ConfigureRequest ||
       event.type == MapRequest ||
       event.type == ResizeRequest ||
       event.type == ClientMessage)
        return false;

    pimpl->eventList.push_back(event);
    return true;
}

bool Gosu::Input::down(Gosu::Button btn) const
{
    // Will default to false for unknown indices (good).
    return pimpl->keyMap[btn.getId()];
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    return Gosu::Button(ch);
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    return btn.getId();
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX * pimpl->mouseFactorX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY * pimpl->mouseFactorY;
}

void Gosu::Input::setMouseFactors(double factorX, double factorY)
{
    pimpl->mouseFactorX = factorX;
    pimpl->mouseFactorY = factorY;
}

void Gosu::Input::update()
{
    for (unsigned int i = 0; i < pimpl->eventList.size(); i++)
    {
        ::XEvent event = pimpl->eventList[i];

        if (textInput() && textInput()->feedXEvent(pimpl->display, &event))
            continue;

        if (event.type == KeyPress)
        {
            // char buf[8];
            // unsigned chars = XLookupString(&event.xkey, buf, sizeof buf, 0, 0);
            // unsigned keysym = XKeycodeToKeysym(pimpl->display, event.xkey.keycode, 0);
            // unsigned id = (chars == 0) ? keysym : widen(buf).at(0);

            unsigned id = XKeycodeToKeysym(pimpl->display, event.xkey.keycode, 0);
            pimpl->keyMap[id] = true;
            if (onButtonDown)
                onButtonDown(Button(id));
        }
        else if (event.type == KeyRelease)
        {
            // char buf[8];
            // unsigned chars = XLookupString(&event.xkey, buf, sizeof buf, 0, 0);
            // unsigned keysym = XKeycodeToKeysym(pimpl->display, event.xkey.keycode, 0);
            // unsigned id = (chars == 0) ? keysym : widen(buf).at(0);

            unsigned id = XKeycodeToKeysym(pimpl->display, event.xkey.keycode, 0);
            pimpl->keyMap[id] = false;
            if (onButtonUp) 
                onButtonUp(Button(id));
        }
        else if (event.type == ButtonPress)
        {
            unsigned id;
            switch (event.xbutton.button)
            {
            case Button1: id = msLeft; break;
            case Button2: id = msMiddle; break;
            case Button3: id = msRight; break;
            case Button4: id = msWheelUp; break;
            case Button5: id = msWheelDown; break;
            default: continue;
            }
            pimpl->keyMap[id] = true;
            // TODO: Here, above, below, who came up with that cast? Uh :)
            if (onButtonDown)
                onButtonDown(*reinterpret_cast<Button*>(&id));
        }
        else if (event.type == ButtonRelease)
        {
            unsigned id;
            switch (event.xbutton.button)
            {
            case Button1: id = msLeft; break;
            case Button2: id = msMiddle; break;
            case Button3: id = msRight; break;
            default: continue;
            }
            pimpl->keyMap[id] = false;
            if (onButtonUp)
                onButtonUp(*reinterpret_cast<Button*>(&id));
        }
        else if (event.type == MotionNotify)
        {
            pimpl->mouseX = event.xbutton.x;
            pimpl->mouseY = event.xbutton.y;
        }
    }
    pimpl->eventList.clear();
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return pimpl->textInput;
}

void Gosu::Input::setTextInput(TextInput* textInput)
{
    pimpl->textInput = textInput;
}

