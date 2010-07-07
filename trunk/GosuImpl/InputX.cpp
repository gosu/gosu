#include <Gosu/Input.hpp>
#include <Gosu/TextInput.hpp>
#include <Gosu/Utility.hpp>
#include <vector>
#include <map>

#include <GosuImpl/Iconv.hpp>

struct Gosu::Input::Impl
{
    TextInput* textInput;
    std::vector< ::XEvent> eventList;
    std::map<unsigned int, bool> keyMap;
    double mouseX, mouseY, mouseFactorX, mouseFactorY;
    ::Display* display;
	::Window window;
    Impl() : textInput(0) {}
};

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
    // ASCII chars
    if (ch >= 32 && ch <= 255)
        return Button(ch);
    // Other chars are conceptually not findable :(
    return noButton;
}

namespace
{
    extern const char LATIN[] = "ISO8859-1";
    extern const char UCS_4_INTERNAL[] = "UCS-4LE";
}

wchar_t Gosu::Input::idToChar(Button btn)
{
    // ASCII chars
    if (btn.id() >= 32 && btn.id() <= 255)
        return btn.id();
    
    // Looking at SDL source suggests that this is to be interpreted depending on the third byte.
    // Should find solid literature on that if it exists.
    // Commented out: This is pretty pointless since LATIN-1 maps to Unicode directly...
    // BUT could serve as a basis for more?!
    //if ((btn.id() >> 8) == 0)
    //{
    //    unsigned char in[] = { btn.id() & 0xff, 0 };
    //    std::wstring converted = iconvert<std::wstring, UCS_4_INTERNAL, LATIN>(std::string(reinterpret_cast<char*>(in)));
    //    return converted.at(0);
    //}

    return 0;
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

            if (i < pimpl->eventList.size() - 1)
            {
                ::XEvent nextEvent = pimpl->eventList[i + 1];
                if (nextEvent.type == KeyPress && nextEvent.xkey.keycode == event.xkey.keycode)
                {
                    i += 1;
                    continue;
                }
            }

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
                onButtonDown(Button(id));
            if (onButtonUp and id == msWheelUp or id == msWheelDown)
                onButtonUp(Button(id));
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

