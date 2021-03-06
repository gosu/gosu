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
        std::unique_ptr<Impl> pimpl;

    public:
        Graphics(unsigned physical_width, unsigned physical_height);
        ~Graphics();

        void set_resolution(unsigned logical_width, unsigned logical_height,
                            double black_bar_width = 0, double black_bar_height = 0);
        
        unsigned width() const;
        unsigned height() const;

        //! Prepares the graphics object for drawing and then runs the rendering code in f.
        //! Nothing must be drawn outside of frame() and record().
        void frame(const std::function<void ()>& f);
        
        //! Flushes the Z queue to the screen and starts a new one.
        //! This can be useful to separate the Z queues of two parts of the game, e.g. the two
        //! halves of a game that runs in split-screen mode.
        static void flush();
        
        //! Finishes all pending Gosu drawing operations and executes the code in f in a clean
        //! OpenGL environment.
        static void gl(const std::function<void ()>& f);

        //! Schedules a custom GL functor to be executed at a certain Z level.
        //! The functor f is run in a clean GL context.
        //! Note: You may not call any Gosu rendering functions from within the
        //! functor.
        static void gl(ZPos z, const std::function<void ()>& f);
        
        //! Renders everything drawn in f clipped to a rectangle on the screen.
        static void clip_to(double x, double y, double width, double height,
                            const std::function<void ()>& f);
        
        //! Renders everything drawn in f onto a new Image of size (width, height).
        //! \param image_flags Pass Gosu::IF_RETRO if you do not want the resulting image to use
        //! interpolation when it is scaled or rotated.
        static Gosu::Image render(int width, int height, const std::function<void ()>& f,
                                  unsigned image_flags = 0);
        
        //! Records a macro and returns it as an Image.
        static Gosu::Image record(int width, int height, const std::function<void ()>& f);
        
        //! Pushes one transformation onto the transformation stack.
        static void transform(const Transform& transform,
                              const std::function<void ()>& f);

        //! Draws a line from one point to another (last pixel exclusive).
        //! Note: OpenGL lines are not reliable at all and may have a missing pixel at the start
        //! or end point. Please only use this for debugging purposes. Otherwise, use a quad or
        //! image to simulate lines, or contribute a better draw_line to Gosu.
        static void draw_line(double x1, double y1, Color c1,
                              double x2, double y2, Color c2,
                              ZPos z, BlendMode mode = BM_DEFAULT);

        static void draw_triangle(double x1, double y1, Color c1,
                                  double x2, double y2, Color c2,
                                  double x3, double y3, Color c3,
                                  ZPos z,
                                  BlendMode mode = BM_DEFAULT);

        static void draw_quad(double x1, double y1, Color c1,
                              double x2, double y2, Color c2,
                              double x3, double y3, Color c3,
                              double x4, double y4, Color c4,
                              ZPos z, BlendMode mode = BM_DEFAULT);
        
        static void draw_rect(double x, double y, double width, double height,
                              Color c, ZPos z,
                              BlendMode mode = BM_DEFAULT);

        //! For internal use only.
        void set_physical_resolution(unsigned physical_width, unsigned physical_height);

        //! For internal use only.
        static void schedule_draw_op(const DrawOp& op);

        //! Turns a portion of a bitmap into something that can be drawn on a Graphics object.
        static std::unique_ptr<ImageData> create_image(const Bitmap& src,
                                                       unsigned src_x,     unsigned src_y,
                                                       unsigned src_width, unsigned src_height,
                                                       unsigned image_flags);
    };
}
