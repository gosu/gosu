#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Utility.hpp>
#include <functional>
#include <memory>

namespace Gosu
{
    /// Serves as the target of all drawing. Usually created internally by Gosu::Window.
    class Viewport : private Noncopyable
    {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        Viewport(int physical_width, int physical_height);
        ~Viewport();

        void set_resolution(int logical_width, int logical_height, //
                            double black_bar_width = 0, double black_bar_height = 0);

        int width() const;
        int height() const;

        Cleanup make_current();

        /// Prepares the graphics object for drawing and then runs the rendering code in f.
        /// Nothing must be drawn outside of frame() and record().
        void frame(const std::function<void()>& f);

        /// For internal use only.
        void set_physical_resolution(int physical_width, int physical_height);

        friend void gl(const std::function<void()>&);
        friend void gl(ZPos, const std::function<void()>&);
        friend void clip_to(double, double, double, double, const std::function<void()>&);
    };

    /// Flushes the Z queue to the screen and starts a new one.
    /// This can be useful to separate the Z queues of two parts of the game, e.g. the two
    /// halves of a game that runs in split-screen mode.
    void flush();

    /// Finishes all pending Gosu drawing operations and executes the code in f in a clean
    /// OpenGL environment.
    void gl(const std::function<void()>& f);

    /// Schedules a custom GL functor to be executed at a certain Z level.
    /// The functor f is run in a clean GL context.
    /// Note: You may not call any Gosu rendering functions from within the functor.
    void gl(ZPos z, const std::function<void()>& f);

    /// Renders everything drawn in f clipped to a rectangle on the screen.
    void clip_to(double x, double y, double width, double height, const std::function<void()>& f);

    /// Renders everything drawn in f onto a new Image of size (width, height).
    /// @param image_flags Pass Gosu::IF_RETRO if you do not want the resulting image to use
    /// interpolation when it is scaled or rotated.
    Gosu::Image render(int width, int height, const std::function<void()>& f,
                       unsigned image_flags = 0);

    /// Records a macro and returns it as an Image.
    Gosu::Image record(int width, int height, const std::function<void()>& f);

    /// Pushes one transformation onto the transformation stack.
    void transform(const Transform& transform, const std::function<void()>& f);

    /// Draws a line from one point to another (last pixel exclusive).
    /// Note: OpenGL lines are not reliable at all and may have a missing pixel at the start
    /// or end point. Please only use this for debugging purposes. Otherwise, use a quad or
    /// image to simulate lines, or contribute a better draw_line to Gosu.
    void draw_line(double x1, double y1, Color c1, double x2, double y2, Color c2, ZPos z,
                   BlendMode mode = BM_DEFAULT);

    void draw_triangle(double x1, double y1, Color c1, //
                       double x2, double y2, Color c2, //
                       double x3, double y3, Color c3, //
                       ZPos z, BlendMode mode = BM_DEFAULT);

    void draw_quad(double x1, double y1, Color c1, double x2, double y2, Color c2, double x3,
                   double y3, Color c3, double x4, double y4, Color c4, ZPos z,
                   BlendMode mode = BM_DEFAULT);

    void draw_rect(double x, double y, double width, double height, Color c, ZPos z,
                   BlendMode mode = BM_DEFAULT);

}
