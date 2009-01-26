#ifndef GOSUIMPL_GRAPHICS_COMMON_HPP
#define GOSUIMPL_GRAPHICS_COMMON_HPP

#include <Gosu/Platform.hpp>

#if defined(GOSU_IS_WIN)
#include <windows.h>
#include <GL/gl.h>
#elif defined(GOSU_IS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif defined(GOSU_IS_MAC)
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
