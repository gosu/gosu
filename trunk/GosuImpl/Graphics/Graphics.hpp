#ifndef GOSUIMPL_GRAPHICS_HPP
#define GOSUIMPL_GRAPHICS_HPP

#ifdef WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <Gosu/Fwd.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <boost/array.hpp>
#include <vector>

namespace Gosu
{
    class TexChunk;
    class Texture;
    struct DrawOp;
    class DrawOpQueue;

    void applyBorderFlags(Bitmap& dest, const Bitmap& source,
        unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
        unsigned borderFlags);
}

#endif
