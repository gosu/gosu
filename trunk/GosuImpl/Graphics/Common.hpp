#ifndef GOSUIMPL_GRAPHICS_COMMON_HPP
#define GOSUIMPL_GRAPHICS_COMMON_HPP

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
    class Texture;
    class TexChunk;
    struct DrawOp;
    class DrawOpQueue;
}

#endif
