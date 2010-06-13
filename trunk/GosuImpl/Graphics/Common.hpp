#ifndef GOSUIMPL_GRAPHICS_COMMON_HPP
#define GOSUIMPL_GRAPHICS_COMMON_HPP

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Platform.hpp>

#if defined(GOSU_IS_WIN)
#define NOMINMAX
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

#include <algorithm>
#include <list>
#include <vector>

namespace Gosu
{
    class Texture;
    class TexChunk;
    struct DrawOp;
    class DrawOpQueue;
    typedef std::list<Transform> Transforms;
    typedef std::vector<DrawOpQueue> DrawOpQueueStack;
    class Macro;
    
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
    
    inline Transform multiply(const Transform& left, const Transform& right)
    {
        Gosu::Transform result;
        result.assign(0);
        for (int i = 0; i < 16; ++i)
            for (int j = 0; j < 4; ++j)
                result[i] += left[i / 4 * 4 + j] * right[i % 4 + j * 4];
        return result;
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
    
    const Bitmap& entityBitmap(const std::wstring& name);
}

#endif
