#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_IPHONE)

#include "GosuViewController.hpp"
#include <Gosu/Gosu.hpp>

struct Gosu::Window::Impl : private Gosu::Noncopyable
{
    UIWindow* window;
    GosuViewController* controller;
    std::unique_ptr<Graphics> graphics;
    std::unique_ptr<Input> input;

    double update_interval;
    std::string caption;
};

Gosu::Window::Window(int width, int height, unsigned window_flags, double update_interval)
: m_impl{new Impl}
{
    m_impl->window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    m_impl->controller = [GosuViewController new];
    m_impl->controller.gosuWindow = this;
    m_impl->window.rootViewController = m_impl->controller;

    // It is important to (implicitly) load the view before creating the Graphics instance.
    [m_impl->controller view];

    m_impl->graphics.reset(new Graphics(screen_width(), screen_height()));
    m_impl->graphics->set_resolution(width, height);

    m_impl->input.reset(new Input((__bridge void*) m_impl->controller.view, update_interval));
    m_impl->input->set_mouse_factors(1.0 * width / available_width(),
                                     1.0 * height / available_height());

    m_impl->input->on_touch_began = [this](Gosu::Touch touch) { touch_began(touch); };
    m_impl->input->on_touch_moved = [this](Gosu::Touch touch) { touch_moved(touch); };
    m_impl->input->on_touch_ended = [this](Gosu::Touch touch) { touch_ended(touch); };
    m_impl->input->on_touch_cancelled = [this](Gosu::Touch touch) { touch_cancelled(touch); };

    // Now let the controller know about our Input instance.
    [m_impl->controller trackTextInput:*m_impl->input];

    m_impl->update_interval = update_interval;
}

Gosu::Window::~Window() = default;

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

void Gosu::Window::set_resizable(bool)
{
}

bool Gosu::Window::borderless() const
{
    return true;
}

void Gosu::Window::set_borderless(bool)
{
}

void Gosu::Window::resize(int, int, bool)
{
    throw std::logic_error{"Cannot resize windows on iOS"};
}

double Gosu::Window::update_interval() const
{
    return m_impl->update_interval;
}

void Gosu::Window::set_update_interval(double)
{
    throw std::logic_error{"Cannot change the update interval on iOS"};
}

std::string Gosu::Window::caption() const
{
    return m_impl->caption;
}

void Gosu::Window::set_caption(const std::string& caption)
{
    m_impl->caption = caption;
}

const Gosu::Graphics& Gosu::Window::graphics() const
{
    return *m_impl->graphics;
}

Gosu::Graphics& Gosu::Window::graphics()
{
    return *m_impl->graphics;
}

const Gosu::Input& Gosu::Window::input() const
{
    return *m_impl->input;
}

Gosu::Input& Gosu::Window::input()
{
    return *m_impl->input;
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
    throw std::logic_error{"Cannot close windows manually on iOS"};
}

void Gosu::Window::button_down(Button button)
{
}

void* Gosu::Window::uikit_window() const
{
    return (__bridge void*) m_impl->window;
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
