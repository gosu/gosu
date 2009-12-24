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
    //! Returns the width, in pixels, of the user's primary screen.
    unsigned screenWidth();
    
    //! Returns the height, in pixels, of the user's primary screen.
    unsigned screenHeight();
    
    // Returns the maximum size of an image that can fit on a single
    // texture.
    // Only useful when extending Gosu using OpenGL.
    // (Held back until it will not cause a stock Ubuntu to crash. Don't ask me!)
    //extern unsigned const MAX_TEXTURE_SIZE;
    
    //! Serves as the target of all drawing and provides basic drawing
    //! functionality.
    //! Usually created by Gosu::Window.
    class Graphics
    {
        struct Impl;
        boost::scoped_ptr<Impl> pimpl;

    public:
        Graphics(unsigned physicalWidth, unsigned physicalHeight, bool fullscreen);
        ~Graphics();

        // Undocumented until I have thought about this...
        double factorX() const;
        double factorY() const;
        void setResolution(unsigned virtualWidth, unsigned virtualHeight);
        // End of Undocumented
        
        unsigned width() const;
        unsigned height() const;
        bool fullscreen() const;

        //! Prepares the graphics object for drawing. Nothing must be drawn
        //! without calling begin.
        bool begin(Color clearWithColor = Color::BLACK);
        //! Every call to begin must have a matching call to end.
        void end();
        //! Finishes all pending Gosu drawing operations and executes
        //! the following OpenGL code in a clean environment.
        void beginGL();
        //! Resets Gosu into its default rendering state.
        void endGL();
        //! Enables clipping to a specified rectangle.
        void beginClipping(int x, int y, unsigned width, unsigned height);
        //! Disables clipping.
        void endClipping();
        
        //! Starts recording a macro. Cannot be nested.
        void beginRecording();
        //! Finishes building the macro and returns it as a drawable object.
        //! Most usually, the return value is passed to Image::Image().
        std::auto_ptr<Gosu::ImageData> endRecording();

        //! Draws a line from one point to another (last pixel exclusive).
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
