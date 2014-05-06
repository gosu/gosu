#include <Gosu/Gosu.hpp>
#include <SDL.h>
#ifdef GOSU_IS_MAC
#include <Carbon/Carbon.h>
#endif
#include <cstdlib>
#include <memory>

namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }
}

struct Gosu::Window::Impl
{
    SDL_Window *window;
    SDL_GLContext context;
    double updateInterval;
    
    std::auto_ptr<Graphics> graphics;
    std::auto_ptr<Input> input;
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to initialize SDL Video");
    
    std::atexit(SDL_Quit);
    
    pimpl->window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI |
            (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));
    pimpl->context = SDL_GL_CreateContext(pimpl->window);
    SDL_GL_MakeCurrent(pimpl->window, pimpl->context);
    SDL_GL_SetSwapInterval(1);
    
    pimpl->graphics.reset(new Graphics(width, height, fullscreen));
    pimpl->input.reset(new Input(nullptr));
    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
    SDL_GL_DeleteContext(pimpl->context);
    SDL_DestroyWindow(pimpl->window);
}

std::wstring Gosu::Window::caption() const
{
    return utf8ToWstring(SDL_GetWindowTitle(pimpl->window));
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    std::string utf8 = wstringToUTF8(caption);
    SDL_SetWindowTitle(pimpl->window, utf8.c_str());
}

double Gosu::Window::updateInterval() const
{
    return pimpl->updateInterval;
}

void Gosu::Window::show()
{
    #ifdef GOSU_IS_MAC
    // This is for Ruby/Gosu and misc. hackery:
    // Usually, applications on the Mac can only get keyboard and mouse input if
    // run by double-clicking an .app. So if this is run from the Terminal (i.e.
    // during Ruby/Gosu game development), tell the OS we need input in any case.
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    TransformProcessType(&psn, kProcessTransformToForegroundApplication);
    SetFrontProcess(&psn);
    #endif
    
	while (true) {
        SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT ||
                e.type == SDL_KEYDOWN ||
                e.type == SDL_MOUSEBUTTONDOWN)
				return;
		}
        
        update();
        
        if (graphics().begin()) {
            draw();
            graphics().end();
        }
	    
        SDL_GL_SwapWindow(pimpl->window);
    }
}

void Gosu::Window::close()
{
    SDL_Event e;
    e.type = SDL_QUIT;
    SDL_PushEvent(&e);
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

// Deprecated.

class Gosu::Audio {};
namespace { Gosu::Audio dummyAudio; }

const Gosu::Audio& Gosu::Window::audio() const
{
    return dummyAudio;
}

Gosu::Audio& Gosu::Window::audio()
{
    return dummyAudio;
}
