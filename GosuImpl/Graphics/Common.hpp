#ifndef GOSUIMPL_GRAPHICS_COMMON_HPP
#define GOSUIMPL_GRAPHICS_COMMON_HPP

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Platform.hpp>

#if defined(GOSU_IS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <GL/gl.h>
#elif defined(GOSU_IS_IPHONE)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#elif defined(GOSU_IS_MAC)
#include <OpenGL/gl.h>
#elif defined GOSU_IS_OPENGLES
#include <GLES/gl.h>
#else
#include <GL/gl.h>
#endif

#include <algorithm>
#include <list>
#include <vector>

namespace Gosu
{
    struct RenderState;
    class RenderStateManager;

    const GLuint NO_TEXTURE = static_cast<GLuint>(-1);
    const unsigned NO_CLIPPING = 0xffffffff;
    
    // In various places in Gosu, width==NO_CLIPPING conventionally means
    // that no clipping should happen.
    struct ClipRect
    {
        double x, y, width, height;
        
        bool operator==(const ClipRect& other) const
        {
            // No clipping
            return (width == NO_CLIPPING && other.width == NO_CLIPPING) ||
            // Clipping, but same
                (x == other.x && y == other.y && width == other.width && height == other.height);
        }
    };
}

namespace Gosu
{
    class Texture;
    class TexChunk;
    class ClipRectStack;
    struct DrawOp;
    class DrawOpQueue;
    typedef std::list<Transform> Transforms;
    typedef std::list<DrawOpQueue> DrawOpQueueStack;
    class Macro;
    struct ArrayVertex
    {
        GLfloat texCoords[2];
        GLuint color;
        GLfloat vertices[3];
    };
    
    template<typename T>
    bool isPToTheLeftOfAB(T xa, T ya,
        T xb, T yb, T xp, T yp)
    {
        return (xb - xa) * (yp - ya) - (xp - xa) * (yb - ya) > 0;
    }
    
    template<typename T, typename C>
    void reorderCoordinatesIfNecessary(T& x1, T& y1,
        T& x2, T& y2, T& x3, T& y3, C& c3, T& x4, T& y4, C& c4)
    {
        if (isPToTheLeftOfAB(x1, y1, x2, y2, x3, y3) ==
            isPToTheLeftOfAB(x2, y2, x3, y3, x4, y4))
        {
            std::swap(x3, x4);
            std::swap(y3, y4);
            std::swap(c3, c4);
        }
    }
    
    template<typename Float>
    void applyTransform(const Transform& transform, Float& x, Float& y)
    {
        Float in[4] = { x, y, 0, 1 };
        Float out[4] = { 0, 0, 0, 0 };
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                out[i] += in[j] * transform[j * 4 + i];
        x = out[0] / out[3];
        y = out[1] / out[3];
    }
    
    inline void multiplyBitmapAlpha(Bitmap& bmp, Color::Channel alpha)
    {
        for (int y = 0; y < bmp.height(); ++y)
            for (int x = 0; x < bmp.width(); ++x)
            {
                Color c = bmp.getPixel(x, y);
                c.setAlpha(c.alpha() * alpha / 255);
                bmp.setPixel(x, y, c);
            }
    }
    
    #ifdef GOSU_IS_IPHONE
    int clipRectBaseFactor();
    #else
    inline int clipRectBaseFactor() { return 1; }
    #endif
    
    bool isEntity(const std::wstring& name);
    const Bitmap& entityBitmap(const std::wstring& name);
}

#endif
