#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Utility.hpp>
#include <cstdint>
#include <memory>

namespace Gosu
{
    /// Returns the maximum size of an texture that will be allocated internally by Gosu.
    /// Useful when extending Gosu using OpenGL.
    const unsigned MAX_TEXTURE_SIZE = 1024;

    /// Contains information about the underlying OpenGL texture and the u/v space used for image
    /// data. Can be retrieved from some drawables to use them in OpenGL operations.
    struct GLTexInfo
    {
        /// This uses std::uint32_t instead of GLuint to avoid publicly including OpenGL headers.
        /// Both types must be the same because GLuint is guaranteed to be an unsigned 32-bit type.
        std::uint32_t tex_name;
        double left, right, top, bottom;
    };

    /// Abstract base class for a rectangular thing that can be drawn.
    /// Instances are usually created through Graphics::create_drawable and used to implement
    /// drawing primitives like Image, which then provide a more convenient drawing interface.
    class Drawable : private Noncopyable
    {
    public:
        Drawable() = default;
        virtual ~Drawable() = default;

        virtual int width() const = 0;

        virtual int height() const = 0;

        virtual void draw(double x1, double y1, Color c1, //
                          double x2, double y2, Color c2, //
                          double x3, double y3, Color c3, //
                          double x4, double y4, Color c4, //
                          ZPos z, BlendMode mode) const
            = 0;

        virtual const GLTexInfo* gl_tex_info() const = 0;

        virtual Bitmap to_bitmap() const = 0;

        virtual std::unique_ptr<Drawable> subimage(const Rect& rect) const = 0;

        virtual void insert(const Bitmap& bitmap, int x, int y) = 0;
    };

    /// Turns a portion of a bitmap into something that can be drawn, typically a TexChunk instance.
    std::unique_ptr<Drawable> create_drawable(const Bitmap& source, const Rect& source_rect,
                                              unsigned image_flags);
}
