#ifndef GOSUIMPL_OPENGL_OPENGL_HPP
#define GOSUIMPL_OPENGL_OPENGL_HPP

#ifndef WIN32
#ifdef __linux__
#include <GL/gl.h>
#else
#include <OpenGL/gl.h>
#endif
#endif

namespace Gosu
{
    namespace OpenGL
    {
        class TexChunk;
        class Texture;
        struct Blit;
    }
}

#endif
