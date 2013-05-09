#include <Gosu/Window.hpp>
#include "Orientation.hpp"

#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>

#include <stdexcept>

namespace Gosu {
    android_app* app = nullptr;
}

using namespace std::tr1::placeholders;

class Gosu::Audio {};

struct Gosu::Window::Impl
{
    std::auto_ptr<Graphics> graphics;
    std::auto_ptr<Audio> audio;
    std::auto_ptr<Input> input;
    double interval;
};

Gosu::Window::Window(unsigned width, unsigned height,
    bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
    pimpl->graphics.reset(new Graphics(720, 1280, false));
    pimpl->audio.reset(new Audio());
    pimpl->input.reset(new Input());
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
    return L"";
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
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
    return *pimpl->audio;
}

Gosu::Audio& Gosu::Window::audio()
{
    return *pimpl->audio;
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
    exit(0);
}

Gosu::Orientation Gosu::currentOrientation()
{
    return orLandscapeLeft;
}

void* Gosu::Window::androidApp() const
{
    if (!app)
        throw std::logic_error("Accessing androidApp() too early");
    return app;
}
