#ifndef GOSUIMPL_OPENGL_OPENGL_HPP
#define GOSUIMPL_OPENGL_OPENGL_HPP

#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
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
