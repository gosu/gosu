#include "OpenGLContext.hpp"
#if !defined(GOSU_IS_IPHONE) // on iPhone, GosuGLView.cpp implements OpenGLContext.

#if defined(GOSU_IS_MAC)
#include <Foundation/Foundation.h> // for -[NSThread isMainThread]
#endif

#include <stdexcept>
#include <thread>

namespace
{
    [[noreturn]] void throw_sdl_error(const std::string& operation)
    {
        throw std::runtime_error(operation + ": " + SDL_GetError());
    }

    /// Groups the global SDL variables into a single struct so that they all either exist, or not.
    struct SDLWindowAndGLContext : Gosu::Noncopyable
    {
        SDL_Window* window;
        /// We only use one OpenGL context, even from background threads. It would be possible to
        /// set up shared OpenGL contexts here (c.f. git history of this file), but according to
        /// various sources on the internet, sharing OpenGL contexts does not help performance.
        /// https://discourse.libsdl.org/t/feasibility-correctness-of-calling-gl-in-another-thread/20292/6
        SDL_GLContext gl_context;

        SDLWindowAndGLContext()
        {
            static const int initialized = SDL_Init(SDL_INIT_VIDEO);
            if (initialized < 0) {
                throw_sdl_error("Could not initialize SDL");
            }

#ifdef GOSU_IS_MAC
            if (![NSThread isMainThread]) {
                throw std::logic_error("Gosu::Window/OpenGL must be initialized on main thread");
            }
#endif
#ifdef GOSU_IS_OPENGLES
            // We want to use OpenGL ES 1.1, which does not yet require us to use shaders.
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif

            window = //
                SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64, 64,
                                 SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI);
            if (!window) {
                throw_sdl_error("Could not create SDL window");
            }

            gl_context = SDL_GL_CreateContext(window);
            if (!gl_context) {
                SDL_DestroyWindow(window);
                throw_sdl_error("Could not create main OpenGL context");
            }
        }

        static SDLWindowAndGLContext& instance()
        {
            static SDLWindowAndGLContext instance;
            return instance;
        }
    };

    /// The current nesting level of OpenGLContext objects.
    thread_local int current_nesting_depth = 0; // NOLINT(*-avoid-non-const-global-variables)
}

Gosu::OpenGLContext::OpenGLContext(bool for_rendering_to_window)
{
    static const auto& window = SDLWindowAndGLContext::instance();

    // All threads use a single mutex to coordinate access to their OpenGL context.
    static std::mutex mutex;
    if (current_nesting_depth == 0) {
        // ...but only the innermost OpenGLContext in a thread owns the lock.
        m_lock = std::unique_lock(mutex);
    }

    // Explicitly make the context current every time for_rendering_to_window is set, so that we set
    // the current SDL_Window even if we are somehow nested into something else.
    if (for_rendering_to_window) {
        if (SDL_GL_MakeCurrent(window.window, window.gl_context) != 0) {
            throw_sdl_error("Could not set current GL context for rendering to the window");
        }
    }
    else if (current_nesting_depth == 0) {
        // Making a GL context current on a background thread, but with a window, causes a deadlock
        // on macOS because SDL tries to use dispatch_sync onto the main thread.
        // -> Try without a window first (for macOS), then with a window (for other platforms).
        if (SDL_GL_MakeCurrent(nullptr, window.gl_context) != 0
            && SDL_GL_MakeCurrent(window.window, window.gl_context) != 0) {
            throw_sdl_error("Could not set current GL context");
        }
    }

    ++current_nesting_depth;
}

Gosu::OpenGLContext::~OpenGLContext()
{
    --current_nesting_depth;

    if (m_lock) {
        SDL_GL_MakeCurrent(nullptr, nullptr);
    }
}

SDL_Window* Gosu::OpenGLContext::shared_sdl_window()
{
    static const auto& window = SDLWindowAndGLContext::instance();
    return window.window;
}

#endif
