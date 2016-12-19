//! \file Graphics.hpp
//! Interface of the Graphics class.

#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <functional>
#include <memory>

namespace Gosu
{
    struct DrawOp;
    
    //! Returns the maximum size of an texture that will be allocated
    //! internally by Gosu.
    //! Useful when extending Gosu using OpenGL.
    unsigned const MAX_TEXTURE_SIZE = 1024;
    
    //! Serves as the target of all drawing and provides primitive drawing
    //! functionality.
    //! Usually created internally by Gosu::Window.
    class Graphics
    {
        struct Impl;
        // Non-movable (const) to avoid dangling internal references.
        const std::unique_ptr<Impl> pimpl;
        
    public:
        Graphics(unsigned physicalWidth, unsigned physicalHeight);
        ~Graphics();

        void setResolution(unsigned logicalWidth, unsigned logicalHeight,
            double horizontalBlackBarWidth = 0, double verticalBlackBarHeight = 0);
        
        unsigned width() const;
        unsigned height() const;

        //! Prepares the graphics object for drawing. Nothing must be drawn
        //! without calling begin.
        bool begin(Color clearWithColor = Color::BLACK);
        //! Every call to begin must have a matching call to end.
        void end();
        
        //! Flushes the Z queue to the screen and starts a new one.
        //! Useful for games that are *very* composite in nature (splitscreen).
        static void flush();
        
        //! Finishes all pending Gosu drawing operations and executes
        //! the following OpenGL code in a clean environment.
        static void beginGL();
        //! Resets Gosu into its default rendering state.
        static void endGL();
        //! Schedules a custom GL functor to be executed at a certain Z level.
        //! The functor is called in a clean GL context (as given by beginGL/endGL).
        //! Gosu's rendering up to the Z level may not yet have been glFlush()ed.
        //! Note: You may not call any Gosu rendering functions from within the
        //! functor, and you must schedule it from within Window::draw's call tree.
        static void scheduleGL(const std::function<void()>& functor, ZPos z);
        
        //! Enables clipping to a specified rectangle.
        static void beginClipping(double x, double y, double width, double height);
        //! Disables clipping.
        static void endClipping();
        
        //! Starts recording a macro. Cannot be nested.
        static void beginRecording();
        //! Finishes building the macro and returns it as a drawable object.
        //! The width and height affect nothing about the recording process,
        //! the resulting macro will simply return these values when you ask
        //! it.
        //! Most usually, the return value is passed to Image::Image().
        static std::unique_ptr<Gosu::ImageData> endRecording(int width, int height);
        
        //! Pushes one transformation onto the transformation stack.
        static void pushTransform(const Transform& transform);
        //! Pops one transformation from the transformation stack.
        static void popTransform();

        //! Draws a line from one point to another (last pixel exclusive).
        //! Note: OpenGL lines are not reliable at all and may have a missing pixel at the start
        //! or end point. Please only use this for debugging purposes. Otherwise, use a quad or
        //! image to simulate lines, or contribute a better drawLine to Gosu.
        static void drawLine(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            ZPos z, AlphaMode mode = amDefault);

        static void drawTriangle(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            ZPos z, AlphaMode mode = amDefault);

        static void drawQuad(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, AlphaMode mode = amDefault);

        //! For internal use only.
        void setPhysicalResolution(unsigned physicalWidth, unsigned physicalHeight);

        //! For internal use only.
        static void scheduleDrawOp(const DrawOp& op);

        //! Turns a portion of a bitmap into something that can be drawn on a Graphics object.
        static std::unique_ptr<ImageData> createImage(const Bitmap& src,
            unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
            unsigned imageFlags);
    };
}
