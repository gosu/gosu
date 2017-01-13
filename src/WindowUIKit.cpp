#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_IPHONE)

#include "GosuViewController.h"
#include <Gosu/Gosu.hpp>

namespace Gosu
{
    unsigned screen_width()
    {
        static CGSize screen_size = [UIScreen mainScreen].bounds.size;
        static CGFloat width = MIN(screen_size.width, screen_size.height);
        return width;
    }
    
    unsigned screen_height()
    {
        static CGSize screen_size = [UIScreen mainScreen].bounds.size;
        static CGFloat width = MAX(screen_size.width, screen_size.height);
        return width;
    }
    
    unsigned available_width()
    {
        return screen_width();
    }
    
    unsigned available_height()
    {
        return screen_height();
    }
}

struct Gosu::Window::Impl
{
    ::UIWindow* window;
    GosuViewController* controller;
    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<Input> input;
    
    bool fullscreen;
    double update_interval;
    std::string caption;
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen, double update_interval)
: pimpl(new Impl)
{
    pimpl->window = [[::UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    pimpl->controller = [GosuViewController new];
    pimpl->controller.gosuWindow = this;
    pimpl->window.rootViewController = pimpl->controller;
    
    // It is important to load the view before creating the Graphics instance.
    [pimpl->controller loadView];
    
    pimpl->graphics.reset(new Graphics(screen_height(), screen_width()));
    pimpl->graphics->set_resolution(width, height);
    
    pimpl->input.reset(new Input((__bridge void*) pimpl->controller.view, update_interval));
    pimpl->input->set_mouse_factors(1.0 * width / screen_height(), 1.0 * height / screen_width());
    
    pimpl->input->on_touch_began = [this](Gosu::Touch touch) { touch_began(touch); };
    pimpl->input->on_touch_moved = [this](Gosu::Touch touch) { touch_moved(touch); };
    pimpl->input->on_touch_ended = [this](Gosu::Touch touch) { touch_ended(touch); };
    pimpl->input->on_touch_cancelled = [this](Gosu::Touch touch) { touch_cancelled(touch); };
    
    pimpl->fullscreen = fullscreen;
    pimpl->update_interval = update_interval;
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

double Gosu::Window::update_interval() const
{
    return pimpl->update_interval;
}

void Gosu::Window::set_update_interval(double update_interval)
{
    throw std::logic_error("Cannot change the update interval on iOS");
}

std::string Gosu::Window::caption() const
{
    return pimpl->caption;
}

void Gosu::Window::set_caption(const std::string& caption)
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

void Gosu::Window::button_down(Button button)
{
}

void* Gosu::Window::UIWindow() const
{
    return (__bridge void*) pimpl->window;
}

#endif
