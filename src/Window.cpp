#include <Gosu/Gosu.hpp>
#include <SDL2/SDL.h>
#include <cstdlib>
#include <memory>
#include <stdexcept>

using namespace std::tr1::placeholders;

namespace Gosu
{
    namespace FPS
    {
        void registerFrame();
    }
    
    void throwSDLError(const std::string& operation)
    {
        const char *error = SDL_GetError();
        throw std::runtime_error(operation + ": " + (error ? error : "(unknown error)"));
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
        throwSDLError("Could not initialize SDL Video");
    
    int actualWidth = width;
    int actualHeight = height;
    double scaleFactor = 1.0;
    double blackBarWidth = 0;
    double blackBarHeight = 0;
    
    if (fullscreen) {
        actualWidth = Gosu::screenWidth();
        actualHeight = Gosu::screenHeight();

        double scaleX = 1.0 * actualWidth / width;
        double scaleY = 1.0 * actualHeight / height;
        scaleFactor = std::min(scaleX, scaleY);

        if (scaleX < scaleY) {
            blackBarHeight = (actualHeight / scaleX - height) / 2;
        }
        else if (scaleY < scaleX) {
            blackBarWidth = (actualWidth / scaleY - width) / 2;
        }
    }
    else {
        double maxWidth = Gosu::availableWidth();
        double maxHeight = Gosu::availableHeight();
        
        if (width > maxWidth || height > maxHeight) {
            scaleFactor = std::min(maxWidth / width, maxHeight / height);
            actualWidth = width * scaleFactor;
            actualHeight = height * scaleFactor;
        }
    }
    
    Uint32 flags = SDL_WINDOW_OPENGL;
#if SDL_VERSION_ATLEAST(2, 0, 1)
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif
    if (fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    pimpl->window = SDL_CreateWindow("",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        actualWidth, actualHeight, flags);
    if (! pimpl->window)
        throwSDLError("Could not open window");

#ifdef GOSU_IS_OPENGLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

#if SDL_VERSION_ATLEAST(2, 0, 1)
    SDL_GL_GetDrawableSize(pimpl->window, &actualWidth, &actualHeight);
#endif
    
    pimpl->context = SDL_GL_CreateContext(pimpl->window);
    if (! pimpl->context)
        throwSDLError("Could not create OpenGL context");
    
    SDL_GL_MakeCurrent(pimpl->window, pimpl->context);
    SDL_GL_SetSwapInterval(1);
    
    pimpl->graphics.reset(new Graphics(actualWidth, actualHeight, fullscreen));
    pimpl->graphics->setResolution(width, height, blackBarWidth, blackBarHeight);
    pimpl->input.reset(new Input());
    pimpl->input->setMouseFactors(1 / scaleFactor, 1 / scaleFactor, blackBarWidth, blackBarHeight);
    input().onButtonDown = std::tr1::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = std::tr1::bind(&Window::buttonUp, this, _1);
    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
    SDL_GL_DeleteContext(pimpl->context);
    SDL_DestroyWindow(pimpl->window);
    
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
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

namespace GosusDarkSide
{
    // TODO: Find a way for this to fit into Gosu's design.
    // This can point to a function that wants to be called every
    // frame, e.g. rb_thread_schedule.
    typedef void (*HookOfHorror)();
    HookOfHorror oncePerTick = 0;
}

void Gosu::Window::show()
{
    while (true) {
        unsigned long startTime = milliseconds();
        
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                return;
            else
                input().feedSDLEvent(&e);
		}
        
        Song::update();
        
        input().update();
        
        update();
        
        SDL_ShowCursor(needsCursor());
        
        if (needsRedraw()) {
            if (graphics().begin()) {
                draw();
                graphics().end();
                FPS::registerFrame();
            }
            
            SDL_GL_SwapWindow(pimpl->window);
            
            if (GosusDarkSide::oncePerTick) GosusDarkSide::oncePerTick();
        }
        
        // Sleep to keep this loop from eating 100% CPU.
        unsigned long frameTime = milliseconds() - startTime;
        if (frameTime < pimpl->updateInterval) {
            sleep(pimpl->updateInterval - frameTime);
        }
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
