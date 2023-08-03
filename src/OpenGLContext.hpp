#ifndef GOSU_OPENGLCONTEXT_HPP
#define GOSU_OPENGLCONTEXT_HPP

#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>

#if defined(GOSU_IS_IPHONE) || defined(GOSU_IS_OPENGLES)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include <mutex>

namespace Gosu
{
    /// Scoped lock/RAII class that makes an OpenGL context the current context, and prevents other
    /// threads from taking over the context until this instance is destroyed.
    class OpenGLContext : Noncopyable
    {
        std::unique_lock<std::recursive_mutex> m_lock;

    public:
        /// Makes an OpenGL context current. The very first call of this constructor may throw, all
        /// other calls can only block.
        OpenGLContext();

#ifndef GOSU_IS_IPHONE
        // SDL does not allow creation of OpenGL contexts without creating an SDL_Window first.
        // Therefore, this class internally creates a global SDL_Window, and derives OpenGL contexts
        // from it. The Gosu::Window implementation can then show and hide this single SDL_Window
        // to simulate the window lifecycle.
        static SDL_Window* shared_sdl_window();
#endif
    };
}

#endif
