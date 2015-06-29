#include <Gosu/Gosu.hpp>
#include "AppleUtility.hpp"
#include "UIKit/GosuViewController.h"

namespace Gosu
{
    unsigned screenWidth()
    {
        static CGSize screenSize = [UIScreen mainScreen].bounds.size;
        static CGFloat width = MIN(screenSize.width, screenSize.height);
        return width;
    }
    
    unsigned screenHeight()
    {
        static CGSize screenSize = [UIScreen mainScreen].bounds.size;
        static CGFloat width = MAX(screenSize.width, screenSize.height);
        return width;
    }
}

class Gosu::Audio {};

struct Gosu::Window::Impl
{
    ObjCRef<::UIWindow> window;
    ObjCRef<GosuViewController> controller;
    std::auto_ptr<Graphics> graphics;
    std::auto_ptr<Input> input;
    double interval;
    std::wstring caption;
};

Gosu::Window::Window(unsigned width, unsigned height,
    bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
    pimpl->window.reset([[::UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]]);
    pimpl->controller.reset([[GosuViewController alloc] init]);
    pimpl->controller.obj().gosuWindow = this;
    pimpl->window.obj().rootViewController = pimpl->controller.obj();
    
    // It is important to load the view before creating the Graphics instance.
    [pimpl->controller.obj() loadView];
    
    pimpl->graphics.reset(new Graphics(screenHeight(), screenWidth(), false));
    pimpl->graphics->setResolution(width, height);
    
    pimpl->input.reset(new Input(pimpl->controller.obj().view, updateInterval));
    
    using namespace std::tr1::placeholders;
    pimpl->input->onTouchBegan = std::tr1::bind(&Window::touchBegan, this, _1);
    pimpl->input->onTouchMoved = std::tr1::bind(&Window::touchMoved, this, _1);
    pimpl->input->onTouchEnded = std::tr1::bind(&Window::touchEnded, this, _1);
    
    pimpl->interval = updateInterval;
}

Gosu::Window::~Window()
{
}

std::wstring Gosu::Window::caption() const
{
    return pimpl->caption;
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    pimpl->caption = caption;
}

double Gosu::Window::updateInterval() const
{
    return pimpl->interval;
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *pimpl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *pimpl->graphics;
}

const Gosu::Audio& Gosu::Window::audio() const
{
    static Gosu::Audio audio;
    return audio;
}

Gosu::Audio& Gosu::Window::audio()
{
    static Gosu::Audio audio;
    return audio;
}

const Gosu::Input& Gosu::Window::input() const
{
    return *pimpl->input;
}

Gosu::Input& Gosu::Window::input()
{
    return *pimpl->input;
}

void Gosu::Window::show()
{
}

void Gosu::Window::close()
{
    throw std::logic_error("Cannot close windows manually on iOS");
}

void* Gosu::Window::UIWindow() const
{
    return pimpl->window.get();
}
