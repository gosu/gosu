#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Platform.hpp>
#include <Gosu/Utility.hpp>
#include <memory>

namespace Gosu
{
    /// Contains information about the underlying OpenGL texture and the u/v space used for image
    /// data. Can be retrieved from some images to use them in OpenGL operations.
    struct GLTexInfo
    {
        unsigned tex_name;
        double left, right, top, bottom;
    };

    /// Abstract base class for a rectangular thing that can be drawn on screen.
    /// Instances are usually created through Graphics::create_image and used to implement drawing
    /// primitives like Image, which then provide a more convenient drawing interface.
    class ImageData : Noncopyable
    {
    public:
        ImageData() = default;
        virtual ~ImageData() = default;

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

        virtual std::unique_ptr<ImageData> subimage(const Rect& rect) const = 0;

        virtual void insert(const Bitmap& bitmap, int x, int y) = 0;
    };
}
