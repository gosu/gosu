//! \file Graphics.hpp
//! Interface of the Graphics class.

#ifndef GOSU_GRAPHICS_HPP
#define GOSU_GRAPHICS_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <boost/scoped_ptr.hpp>
#include <memory>

namespace Gosu
{
    //! Flags that affect the softness of a border.
    enum BorderFlags
    {
        bfSoft = 0,
        bfHardLeft = 1,
        bfHardTop = 2,
        bfHardRight = 4,
        bfHardBottom = 8,
        bfHard = bfHardLeft | bfHardTop | bfHardRight | bfHardBottom,
    };        
    
    //! Serves as the target of all drawing and provides basic drawing
    //! functionality.
    class Graphics
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        Graphics(unsigned width, unsigned height, bool fullscreen);
        ~Graphics();

        // Undocumented until I have thought about this...
        double factorX() const;
        double factorY() const;
        double virtualWidth() const;
        double virtualHeight() const;
        void setVirtualResolution(double virtualWidth, double virtualHeight);
        // End of Undocumented

        unsigned width() const;
        unsigned height() const;
        bool fullscreen() const;

        //! Prepares the graphics object for drawing. Nothing must be drawn
        //! without calling begin.
        bool begin(Color clearWithColor = Colors::black);
        //! Every call to begin must have a matching call to end.
        void end();
        // Processes all enqueued Gosu drawing operations, so custom OpenGL code can start.
        void flush();

        void drawLine(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            ZPos z, AlphaMode mode = amDefault);

        void drawTriangle(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            ZPos z, AlphaMode mode = amDefault);

        void drawQuad(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, AlphaMode mode = amDefault);

        //! Turns a portion of a bitmap into something that can be drawn on
        //! this graphics object.
        std::auto_ptr<ImageData> createImage(const Bitmap& src,
            unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
            unsigned borderFlags);
    };
}

#endif
