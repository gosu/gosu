#include <Gosu/Input.hpp>
#include <vector>
#include <map>

struct Gosu::Input::Impl
{
    std::vector< ::XEvent> eventList;
    std::map<unsigned int, bool> keyMap;
    double mouseX, mouseY;
    ::Display* display;
};

Gosu::Input::Input(::Display* dpy)
    : pimpl(new Impl)
{
    // IMPR: Get current position?
    pimpl->mouseX = 0;
    pimpl->mouseY = 0;
    pimpl->display = dpy;
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

bool Gosu::Input::down(unsigned id) const
{
    // Will default to false for unknown indices (good).
    return pimpl->keyMap[id];
}

Gosu::Button Gosu::Input::charToId(wchar_t ch) const
{
    return Gosu::Button(ch);
}

wchar_t Gosu::Input::idToChar(Button btn) const
{
    return btn.getId();
}

double Gosu::Input::mouseX() const
{
    return pimpl->mouseX;
}

double Gosu::Input::mouseY() const
{
    return pimpl->mouseY;
}

void Gosu::Input::update()
{
    for(unsigned int i = 0; i < pimpl->eventList.size(); i++)
    {
        ::XEvent event = pimpl->eventList[i];
        if(event.type == KeyPress)
        {
            unsigned keysym = XKeycodeToKeysym(pimpl->display,
                event.xkey.keycode, 0);
            pimpl->keyMap[keysym] = true;
            if(onButtonDown)
                onButtonDown(*reinterpret_cast<Button*>(&keysym));
        }
        else if(event.type == KeyRelease)
        {
            unsigned keysym = XKeycodeToKeysym(pimpl->display,
                event.xkey.keycode, 0);
            pimpl->keyMap[keysym] = false;
            if(onButtonUp)
                onButtonUp(*reinterpret_cast<Button*>(&keysym));
        }
        else if(event.type == ButtonPress)
        {
            unsigned id;
            switch (event.xbutton.button)
            {
            case Button1: id = msLeft; break;
            case Button2: id = msMiddle; break;
            case Button3: id = msRight; break;
            default: continue;
            }
            pimpl->keyMap[id] = true;
            if(onButtonDown)
                onButtonDown(*reinterpret_cast<Button*>(&id));
        }
        else if(event.type == ButtonRelease)
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
            if(onButtonUp)
                onButtonUp(*reinterpret_cast<Button*>(&id));
        }
        else if(event.type == MotionNotify)
        {
            pimpl->mouseX = event.xbutton.x;
            pimpl->mouseY = event.xbutton.y;
        }
    }
    pimpl->eventList.clear();
}

