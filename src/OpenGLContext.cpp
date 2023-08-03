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
    struct SDLWindowAndGLContexts : Gosu::Noncopyable
    {
        /// The ID of the thread that created the window. We assume that this is the main thread on
        /// which rendering will happen, and it receives its own OpenGL context.
        std::thread::id main_thread;
        SDL_Window* window;
        SDL_GLContext main_thread_context;
        /// OpenGL context for all threads except main_thread. Multiple threads creating and
        /// deleting images, for example, will receive serialized access to this context.
        SDL_GLContext background_threads_context;

        SDLWindowAndGLContexts()
        {
            static const int initialized = SDL_Init(SDL_INIT_VIDEO);
            if (initialized < 0) {
                throw_sdl_error("Could not initialize SDL");
            }

#ifdef GOSU_IS_MAC
            if (![NSThread isMainThread]) {
                throw std::logic_error(
                    "First use of Gosu::Window or OpenGL must happen the main thread");
            }
#endif
            // Except for Apple systems, which insist on having the UI on the main (first) thread,
            // we assume that whatever thread calls this first is the main rendering thread.
            main_thread = std::this_thread::get_id();

            SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
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

            background_threads_context = SDL_GL_CreateContext(window);
            if (!background_threads_context) {
                SDL_DestroyWindow(window);
                throw_sdl_error("Could not create background OpenGL context");
            }

            main_thread_context = SDL_GL_CreateContext(window);
            if (!main_thread_context) {
                // Probably better to unset the current context before deleting it.
                // Hard to make convert this to RAII without lots of lines.
                SDL_GL_MakeCurrent(nullptr, nullptr);
                SDL_GL_DeleteContext(background_threads_context);
                SDL_DestroyWindow(window);
                throw_sdl_error("Could not create main OpenGL context");
            }
        }

        static SDLWindowAndGLContexts& instance()
        {
            static SDLWindowAndGLContexts instance;
            return instance;
        }
    };
}

Gosu::OpenGLContext::OpenGLContext()
{
    static const auto& window = SDLWindowAndGLContexts::instance();

    if (std::this_thread::get_id() == window.main_thread) {
        if (SDL_GL_MakeCurrent(shared_sdl_window(), window.main_thread_context) != 0) {
            throw_sdl_error("Could not set current GL context on UI thread");
        }
        // No need to lock anything.
    }
    else {
        // All non-UI threads use a single mutex to coordinate access to their OpenGL
        // context.
        static std::recursive_mutex other_threads_mutex;
        m_lock = std::unique_lock(other_threads_mutex);
        // Making a GL context current on a background thread, but with a window, causes a
        // deadlock on macOS because SDL tries to use dispatch_sync onto the main thread.
        // -> Try without a window first (for macOS), then with a window (for other
        // platforms).
        if (SDL_GL_MakeCurrent(nullptr, window.background_threads_context) != 0
            && SDL_GL_MakeCurrent(window.window, window.background_threads_context) != 0) {
            throw_sdl_error("Could not set current GL context on background thread");
        }
    }
}

SDL_Window* Gosu::OpenGLContext::shared_sdl_window()
{
    static const auto& window = SDLWindowAndGLContexts::instance();
    return window.window;
}

#endif
