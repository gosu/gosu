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
    static ::Display* display;
	::Window window;
    Impl() : textInput(0) {}
};

// TODO - ugly, but necessary for charToId/idToChar, since they are static functions.
// TODO - should instead create a global, shared connection to X11
::Display* Gosu::Input::Impl::display = NULL;

Gosu::Input::Input(::Display* dpy, ::Window wnd)
    : pimpl(new Impl)
{
    // IMPR: Get current position?
    pimpl->mouseX = pimpl->mouseY = 0;
    pimpl->mouseFactorX = pimpl->mouseFactorY = 1.0;
    pimpl->display = dpy;
	pimpl->window = wnd;
}

Gosu::Input::~Input()
{
    Impl::display = NULL;
}

bool Gosu::Input::feedXEvent(::XEvent& event)
{
	// IMPR: Wouldn't it make more sense to filter the other way around?
	
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
    if (btn == noButton)
        return false;

    // Will default to false for unknown indices (good).
    return pimpl->keyMap[btn.id()];
}

Gosu::Button Gosu::Input::charToId(wchar_t ch)
{
    // TODO - Effectively, this is now a member function of Input again, not a
    // static member function.
    if (! Impl::display)
        return noButton;
    
    // TODO - Does this always work for ISO Latin 1 (which the lowest Unicode byte maps to)?
    if (ch >= 32 && ch <= 255)
        return Button(XKeysymToKeycode(Impl::display, ch) - 8);
        
    // TODO - no idea how other characters can be found. Maybe with the code above?
    return noButton;
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    // TODO - Effectively, this is now a member function of Input again, not a
    // static member function.
    if (! Impl::display)
        return 0;
    
    KeySym keySym = XKeycodeToKeysym(Impl::display, btn.id() + 8, 0);
    if (keySym == NoSymbol || keySym < 32 || keySym > 255)
        return 0;
    return keySym;
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
    for (unsigned int i = 0; i < pimpl->eventList.size(); i++)
    {
        ::XEvent event = pimpl->eventList[i];

        if (textInput() && textInput()->feedXEvent(pimpl->display, &event))
            continue;

        if (event.type == KeyPress)
        {
            unsigned id = event.xkey.keycode - 8;

            pimpl->keyMap[id] = true;
            if (onButtonDown)
                onButtonDown(Button(id));
        }
        else if (event.type == KeyRelease)
        {
            if (i < pimpl->eventList.size() - 1)
            {
                ::XEvent nextEvent = pimpl->eventList[i + 1];
                if (nextEvent.type == KeyPress && nextEvent.xkey.keycode == event.xkey.keycode)
                {
                    i += 1;
                    continue;
                }
            }

            unsigned id = event.xkey.keycode - 8;

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
            case 6: id = msOther0; break;
            case 7: id = msOther1; break;
            case 8: id = msOther2; break;
            case 9: id = msOther3; break;
            case 10: id = msOther4; break;
            case 11: id = msOther5; break;
            case 12: id = msOther6; break;
            case 13: id = msOther7; break;
            default: continue;
            }
            pimpl->keyMap[id] = true;
            if (onButtonDown)
                onButtonDown(Button(id));
        }
        else if (event.type == ButtonRelease)
        {
            unsigned id;
            switch (event.xbutton.button)
            {
            case Button1: id = msLeft; break;
            case Button2: id = msMiddle; break;
            case Button3: id = msRight; break;
            case Button4: id = msWheelUp; break;
            case Button5: id = msWheelDown; break;
            case 6: id = msOther0; break;
            case 7: id = msOther1; break;
            case 8: id = msOther2; break;
            case 9: id = msOther3; break;
            case 10: id = msOther4; break;
            case 11: id = msOther5; break;
            case 12: id = msOther6; break;
            case 13: id = msOther7; break;
            default: continue;
            }
            pimpl->keyMap[id] = false;
            if (onButtonUp)
                onButtonUp(Button(id));
        }
        else if (event.type == MotionNotify)
        {
            pimpl->mouseX = event.xbutton.x;
            pimpl->mouseY = event.xbutton.y;
        }
        else if (event.type == EnterNotify || event.type == LeaveNotify)
        {
            pimpl->mouseX = event.xcrossing.x;
            pimpl->mouseY = event.xcrossing.y;
        }
    }
    pimpl->eventList.clear();
}

void Gosu::Input::setMousePosition(double x, double y)
{
    ::XWarpPointer(pimpl->display, None, pimpl->window, 0, 0, 0, 0,
				   x / pimpl->mouseFactorX, y / pimpl->mouseFactorY);
    ::XSync(pimpl->display, False);
    // Couldn't find a way to fetch the current mouse position. These
    // values may not be correct if the cursor was grabbed, for example.
    pimpl->mouseX = x, pimpl->mouseY = y;
}

Gosu::TextInput* Gosu::Input::textInput() const
{
    return pimpl->textInput;
}

void Gosu::Input::setTextInput(TextInput* textInput)
{
    pimpl->textInput = textInput;
}

