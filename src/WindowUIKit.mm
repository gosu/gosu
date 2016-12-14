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
    
    unsigned availableWidth()
    {
        return screenWidth();
    }
    
    unsigned availableHeight()
    {
        return screenHeight();
    }
}

class Gosu::Audio {};

struct Gosu::Window::Impl
{
    ::UIWindow *window;
    GosuViewController *controller;
    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<Input> input;
    
    bool fullscreen;
    double updateInterval;
    std::wstring caption;
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
    pimpl->window = [[::UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    pimpl->controller = [GosuViewController new];
    pimpl->controller.gosuWindow = this;
    pimpl->window.rootViewController = pimpl->controller;
    
    // It is important to load the view before creating the Graphics instance.
    [pimpl->controller loadView];
    
    pimpl->graphics.reset(new Graphics(screenHeight(), screenWidth()));
    pimpl->graphics->setResolution(width, height);
    
    pimpl->input.reset(new Input((__bridge void *)pimpl->controller.view, updateInterval));
    pimpl->input->setMouseFactors(1.0 * width / screenHeight(), 1.0 * height / screenWidth());
    
    using namespace std::tr1::placeholders;
    pimpl->input->onTouchBegan = std::tr1::bind(&Window::touchBegan, this, _1);
    pimpl->input->onTouchMoved = std::tr1::bind(&Window::touchMoved, this, _1);
    pimpl->input->onTouchEnded = std::tr1::bind(&Window::touchEnded, this, _1);
    pimpl->input->onTouchCancelled = std::tr1::bind(&Window::touchCancelled, this, _1);
    
    pimpl->fullscreen = fullscreen;
    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
}

unsigned Gosu::Window::width() const
{
    return graphics().width();
}

unsigned Gosu::Window::height() const
{
    return graphics().height();
}

bool Gosu::Window::fullscreen() const
{
    return pimpl->fullscreen;
}

void Gosu::Window::resize(unsigned width, unsigned height, bool fullscreen)
{
    throw std::logic_error("Cannot resize windows on iOS");
}

double Gosu::Window::updateInterval() const
{
    return pimpl->updateInterval;
}

void Gosu::Window::setUpdateInterval(double updateInterval)
{
    throw std::logic_error("Cannot change the update interval on iOS");
}

std::wstring Gosu::Window::caption() const
{
    return pimpl->caption;
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    pimpl->caption = caption;
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

bool Gosu::Window::tick()
{
    return true;
}

void Gosu::Window::close()
{
    throw std::logic_error("Cannot close windows manually on iOS");
}

void *Gosu::Window::UIWindow() const
{
    return (__bridge void *)pimpl->window;
}
