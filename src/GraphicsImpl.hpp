#pragma once

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Platform.hpp>

#if defined(GOSU_IS_IPHONE) || defined(GOSU_IS_OPENGLES)
#include <OpenGLES/ES1/gl.h>
#include <OpenGLES/ES1/glext.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include <algorithm>
#include <list>
#include <mutex>
#include <vector>

namespace Gosu
{
    struct RenderState;
    class RenderStateManager;

    const GLuint NO_TEXTURE = static_cast<GLuint>(-1);
    const unsigned NO_CLIPPING = 0xffffffff;

    // In various places in Gosu, width==NO_CLIPPING by convention means
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

    enum QueueMode
    {
        QM_RENDER_TO_SCREEN,
        QM_RENDER_TO_TEXTURE,
        QM_RECORD_MACRO,
    };

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
        GLfloat tex_coords[2];
        GLuint color;
        GLfloat vertices[3];
    };

    template<typename T>
    bool is_p_to_the_left_of_ab(T xa, T ya, T xb, T yb, T xp, T yp)
    {
        return (xb - xa) * (yp - ya) - (xp - xa) * (yb - ya) > 0;
    }

    template<typename T, typename C>
    void normalize_coordinates(T& x1, T& y1, T& x2, T& y2, T& x3, T& y3, C& c3, T& x4, T& y4, C& c4)
    {
        if (is_p_to_the_left_of_ab(x1, y1, x2, y2, x3, y3) ==
                is_p_to_the_left_of_ab(x2, y2, x3, y3, x4, y4)) {
            std::swap(x3, x4);
            std::swap(y3, y4);
            std::swap(c3, c4);
        }
    }

#ifdef GOSU_IS_IPHONE
    int clip_rect_base_factor();
#else
    inline int clip_rect_base_factor() { return 1; }
#endif

    [[nodiscard]] std::unique_lock<std::recursive_mutex> lock_gl_context();

    inline std::string escape_markup(const std::string& text) {
        auto markup = text;
        for (std::string::size_type pos = 0; pos < markup.length(); ++pos) {
            if (markup[pos] == '&') {
                markup.replace(pos, 1, "&amp;");
            }
            else if (markup[pos] == '<') {
                markup.replace(pos, 1, "&lt;");
            }
        }
        return markup;
    }
}
