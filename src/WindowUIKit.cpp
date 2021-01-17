#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_IPHONE)

#include "GosuViewController.hpp"
#include <Gosu/Gosu.hpp>

using namespace std;

struct Gosu::Window::Impl
{
    UIWindow* window;
    GosuViewController* controller;
    unique_ptr<Graphics> graphics;
    unique_ptr<Input> input;
    
    double update_interval;
    string caption;
};

Gosu::Window::Window(int width, int height, unsigned window_flags, double update_interval)
: pimpl(new Impl)
{
    pimpl->window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    pimpl->controller = [GosuViewController new];
    pimpl->controller.gosuWindow = this;
    pimpl->window.rootViewController = pimpl->controller;
    
    // It is important to load the view before creating the Graphics instance.
    [pimpl->controller loadView];
    
    pimpl->graphics.reset(new Graphics(screen_width(), screen_height()));
    pimpl->graphics->set_resolution(width, height);
    
    pimpl->input.reset(new Input((__bridge void*) pimpl->controller.view, update_interval));
    pimpl->input->set_mouse_factors(1.0 * width / available_width(), 1.0 * height / available_height());
    
    pimpl->input->on_touch_began = [this](Gosu::Touch touch) { touch_began(touch); };
    pimpl->input->on_touch_moved = [this](Gosu::Touch touch) { touch_moved(touch); };
    pimpl->input->on_touch_ended = [this](Gosu::Touch touch) { touch_ended(touch); };
    pimpl->input->on_touch_cancelled = [this](Gosu::Touch touch) { touch_cancelled(touch); };
    
    // Now let the controller know about our Input instance.
    [pimpl->controller trackTextInput:*pimpl->input];
    
    pimpl->update_interval = update_interval;
}

Gosu::Window::~Window()
{
}

int Gosu::Window::width() const
{
    return graphics().width();
}

int Gosu::Window::height() const
{
    return graphics().height();
}

bool Gosu::Window::fullscreen() const
{
    return true;
}

bool Gosu::Window::resizable() const
{
    return false;
}

void Gosu::Window::set_resizable(bool resizable)
{
}

bool Gosu::Window::borderless() const
{
    return true;
}

void Gosu::Window::set_borderless(bool borderless)
{
}

void Gosu::Window::resize(int width, int height, bool fullscreen)
{
    throw logic_error("Cannot resize windows on iOS");
}

double Gosu::Window::update_interval() const
{
    return pimpl->update_interval;
}

void Gosu::Window::set_update_interval(double update_interval)
{
    throw logic_error("Cannot change the update interval on iOS");
}

string Gosu::Window::caption() const
{
    return pimpl->caption;
}

void Gosu::Window::set_caption(const string& caption)
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
    throw logic_error("Cannot close windows manually on iOS");
}

void Gosu::Window::button_down(Button button)
{
}

void* Gosu::Window::uikit_window() const
{
    return (__bridge void*) pimpl->window;
}

int Gosu::screen_width(Window*)
{
    return available_width() * [UIScreen mainScreen].scale;
}

int Gosu::screen_height(Window*)
{
    return available_height() * [UIScreen mainScreen].scale;
}

int Gosu::available_width(Window*)
{
    static CGSize screen_size = [UIScreen mainScreen].bounds.size;
    static CGFloat width = MAX(screen_size.width, screen_size.height);
    return width;
}

int Gosu::available_height(Window*)
{
    static CGSize screen_size = [UIScreen mainScreen].bounds.size;
    static CGFloat height = MIN(screen_size.width, screen_size.height);
    return height;
}

#endif
