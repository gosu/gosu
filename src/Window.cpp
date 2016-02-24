#include <Gosu/Gosu.hpp>
#include "Graphics/Common.hpp"
#include <SDL.h>
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

    void cleanup();

    SDL_Window* sharedWindow()
    {
        static SDL_Window *window = 0;
        if (window == 0)
        {
            if (SDL_Init(SDL_INIT_VIDEO) < 0)
                throwSDLError("Could not initialize SDL Video");

            std::atexit(cleanup);

            Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
            
            #if SDL_VERSION_ATLEAST(2, 0, 1)
            flags |= SDL_WINDOW_ALLOW_HIGHDPI;
            #endif
            
            window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64, 64, flags);
            if (window == 0)
                throwSDLError("Could not create window");
        }
        return window;
    }
    
    SDL_GLContext sharedGLContext()
    {
        static SDL_GLContext context = 0;
        if (context == 0)
        {
            #ifdef GOSU_IS_OPENGLES
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            #endif
            
            context = SDL_GL_CreateContext(sharedWindow());
            
            if (context == 0)
                throwSDLError("Could not create OpenGL context");
        }
        return context;
    }
    
    void ensureCurrentContext()
    {
        SDL_GL_MakeCurrent(sharedWindow(), sharedGLContext());
    }

    void cleanup()
    {
        SDL_GL_DeleteContext(sharedGLContext());
        SDL_DestroyWindow(sharedWindow());
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
    }
}

struct Gosu::Window::Impl
{
    double updateInterval;
    
    std::auto_ptr<Graphics> graphics;
    std::auto_ptr<Input> input;
};

Gosu::Window::Window(unsigned width, unsigned height, bool fullscreen, double updateInterval)
: pimpl(new Impl)
{
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
    
    SDL_SetWindowTitle(sharedWindow(), "");
    SDL_SetWindowSize(sharedWindow(), actualWidth, actualHeight);
    SDL_SetWindowPosition(sharedWindow(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    if (fullscreen)
    {
        SDL_SetWindowFullscreen(sharedWindow(), fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }
    
    #if SDL_VERSION_ATLEAST(2, 0, 1)
    SDL_GL_GetDrawableSize(sharedWindow(), &actualWidth, &actualHeight);
    #endif
    
    ensureCurrentContext();
    SDL_GL_SetSwapInterval(1);
    
    pimpl->graphics.reset(new Graphics(actualWidth, actualHeight, fullscreen));
    pimpl->graphics->setResolution(width, height, blackBarWidth, blackBarHeight);
    pimpl->input.reset(new Input(sharedWindow()));
    pimpl->input->setMouseFactors(1 / scaleFactor, 1 / scaleFactor, blackBarWidth, blackBarHeight);
    input().onButtonDown = std::tr1::bind(&Window::buttonDown, this, _1);
    input().onButtonUp = std::tr1::bind(&Window::buttonUp, this, _1);
    pimpl->updateInterval = updateInterval;
}

Gosu::Window::~Window()
{
    SDL_HideWindow(sharedWindow());
}

std::wstring Gosu::Window::caption() const
{
    return utf8ToWstring(SDL_GetWindowTitle(sharedWindow()));
}

void Gosu::Window::setCaption(const std::wstring& caption)
{
    std::string utf8 = wstringToUTF8(caption);
    SDL_SetWindowTitle(sharedWindow(), utf8.c_str());
}

double Gosu::Window::updateInterval() const
{
    return pimpl->updateInterval;
}

void Gosu::Window::show()
{
    unsigned long timeBeforeTick = milliseconds();
    
    while (tick()) {
        // Sleep to keep this loop from eating 100% CPU.
        unsigned long tickTime = milliseconds() - timeBeforeTick;
        if (tickTime < updateInterval())
            sleep(updateInterval() - tickTime);
        
        timeBeforeTick = milliseconds();
    }
}

bool Gosu::Window::tick()
{
    if (SDL_GetWindowFlags(sharedWindow()) & SDL_WINDOW_HIDDEN) {
        SDL_ShowWindow(sharedWindow());

        // SDL_GL_GetDrawableSize returns different values before and after showing the window.
        // -> When first showing the window, update the physical size of Graphics (=glViewport).
        // Fixes https://github.com/gosu/gosu/issues/318
        int width, height;
        SDL_GL_GetDrawableSize(sharedWindow(), &width, &height);
        graphics().setPhysicalResolution(width, height);
    }
    
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            SDL_HideWindow(sharedWindow());
            return false;
        }
        else {
            input().feedSDLEvent(&e);
        }
    }
    
    Song::update();
    
    input().update();
    
    update();
    
    SDL_ShowCursor(needsCursor());
    
    if (needsRedraw()) {
        ensureCurrentContext();
        if (graphics().begin()) {
            draw();
            graphics().end();
            FPS::registerFrame();
        }
        
        SDL_GL_SwapWindow(sharedWindow());
    }
    
    return true;
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
