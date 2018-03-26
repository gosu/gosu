//! \file ImageData.hpp
//! Interface of the ImageData class.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Platform.hpp>
#include <memory>

namespace Gosu
{
    //! Contains information about the underlying OpenGL texture and the
    //! u/v space used for image data. Can be retrieved from some images
    //! to use them in OpenGL operations.
    struct GLTexInfo
    {
        int tex_name;
        double left, right, top, bottom;
    };

    //! The ImageData class is an abstract base class for drawable images.
    //! Instances of classes derived by ImageData are usually returned by
    //! Graphics::create_image and usually only used to implement drawing
    //! primitives like Image, which then provide a more specialized and
    //! intuitive drawing interface.
    class ImageData
    {
        // Non-copyable
        ImageData(const ImageData&);
        ImageData& operator=(const ImageData&);
        
    public:
        ImageData() {}
        
        virtual ~ImageData() {}

        virtual int width() const = 0;
        
        virtual int height() const = 0;

        virtual void draw(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, BlendMode mode) const = 0;

        virtual const GLTexInfo* gl_tex_info() const = 0;
        
        virtual Bitmap to_bitmap() const = 0;
        
        virtual std::unique_ptr<ImageData> subimage(int x, int y, int width, int height) const = 0;
        
        virtual void insert(const Bitmap& bitmap, int x, int y) = 0;
    };
}
