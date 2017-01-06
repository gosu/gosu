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
    const unsigned MAX_TEXTURE_SIZE = 1024;
    
    //! Serves as the target of all drawing and provides primitive drawing
    //! functionality.
    //! Usually created internally by Gosu::Window.
    class Graphics
    {
        struct Impl;
        // Non-movable (const) to avoid dangling internal references.
        const std::unique_ptr<Impl> pimpl;
        
    public:
        Graphics(unsigned physical_width, unsigned physical_height);
        ~Graphics();

        void set_resolution(unsigned logical_width, unsigned logical_height,
            double horizontal_black_bar_width = 0, double vertical_black_bar_height = 0);
        
        unsigned width() const;
        unsigned height() const;

        //! Prepares the graphics object for drawing. Nothing must be drawn
        //! without calling begin.
        bool begin(Color clear_with_color = Color::BLACK);
        //! Every call to begin must have a matching call to end.
        void end();
        
        //! Flushes the Z queue to the screen and starts a new one.
        //! Useful for games that are *very* composite in nature (splitscreen).
        static void flush();
        
        //! Finishes all pending Gosu drawing operations and executes
        //! the following OpenGL code in a clean environment.
        static void begin_gl();
        //! Resets Gosu into its default rendering state.
        static void end_gl();
        //! Schedules a custom GL functor to be executed at a certain Z level.
        //! The functor is called in a clean GL context (as given by begin_gl/end_gl).
        //! Gosu's rendering up to the Z level may not yet have been glFlush()ed.
        //! Note: You may not call any Gosu rendering functions from within the
        //! functor, and you must schedule it from within Window::draw's call tree.
        static void gl(const std::function<void ()>& functor, ZPos z);
        
        //! Enables clipping to a specified rectangle.
        static void begin_clipping(double x, double y, double width, double height);
        //! Disables clipping.
        static void end_clipping();
        
        //! Starts recording a macro. Cannot be nested.
        static void begin_recording();
        //! Finishes building the macro and returns it as a drawable object.
        //! The width and height affect nothing about the recording process,
        //! the resulting macro will simply return these values when you ask
        //! it.
        //! Most usually, the return value is passed to Image::Image().
        static std::unique_ptr<Gosu::ImageData> end_recording(int width, int height);
        
        //! Pushes one transformation onto the transformation stack.
        static void push_transform(const Transform& transform);
        //! Pops one transformation from the transformation stack.
        static void pop_transform();

        //! Draws a line from one point to another (last pixel exclusive).
        //! Note: OpenGL lines are not reliable at all and may have a missing pixel at the start
        //! or end point. Please only use this for debugging purposes. Otherwise, use a quad or
        //! image to simulate lines, or contribute a better draw_line to Gosu.
        static void draw_line(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            ZPos z, AlphaMode mode = AM_DEFAULT);

        static void draw_triangle(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            ZPos z, AlphaMode mode = AM_DEFAULT);

        static void draw_quad(double x1, double y1, Color c1,
            double x2, double y2, Color c2,
            double x3, double y3, Color c3,
            double x4, double y4, Color c4,
            ZPos z, AlphaMode mode = AM_DEFAULT);

        //! For internal use only.
        void set_physical_resolution(unsigned physical_width, unsigned physical_height);

        //! For internal use only.
        static void schedule_draw_op(const DrawOp& op);

        //! Turns a portion of a bitmap into something that can be drawn on a Graphics object.
        static std::unique_ptr<ImageData> create_image(const Bitmap& src,
            unsigned src_x, unsigned src_y, unsigned src_width, unsigned src_height,
            unsigned image_flags);
    };
}
