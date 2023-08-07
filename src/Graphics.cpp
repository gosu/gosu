#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Utility.hpp>
#include "DrawOp.hpp"
#include "DrawOpQueue.hpp"
#include "GraphicsImpl.hpp"
#include "Macro.hpp"
#include "OffScreenTarget.hpp"
#include "OpenGLContext.hpp"
#include <algorithm>
#include <memory>

namespace Gosu
{
    namespace
    {
        Viewport* current_viewport_pointer = nullptr;

        Viewport& current_viewport()
        {
            if (current_viewport_pointer == nullptr) {
                throw std::logic_error("Gosu::Graphics can only be drawn to while rendering");
            }
            return *current_viewport_pointer;
        }

        DrawOpQueueStack queues;

        DrawOpQueue& current_queue()
        {
            if (queues.empty()) {
                throw std::logic_error("There is no rendering queue for this operation");
            }
            return queues.back();
        }
    }
}

struct Gosu::Viewport::Impl : private Gosu::Noncopyable
{
    int virt_width = 0, virt_height = 0;
    int phys_width = 0, phys_height = 0;
    double black_width = 0.0, black_height = 0.0;
    Transform base_transform;

    DrawOpQueueStack warmed_up_queues;

    void update_base_transform()
    {
        double scale_x = 1.0 * phys_width / virt_width;
        double scale_y = 1.0 * phys_height / virt_height;
        double scale_factor = std::min(scale_x, scale_y);

        Transform scale_transform = Transform::scale(scale_factor);
        Transform translate_transform = Transform::translate(black_width, black_height);
        base_transform = translate_transform * scale_transform;
    }

#ifndef GOSU_IS_OPENGLES
    void begin_gl() // NOLINT(readability-convert-member-functions-to-static)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glDisable(GL_BLEND);
        // Reset the color to white to avoid surprises.
        // https://www.libgosu.org/cgi-bin/mwf/topic_show.pl?pid=9115#pid9115
        glColor4ubv(reinterpret_cast<const GLubyte*>(&Color::WHITE));
        while (glGetError() != GL_NO_ERROR)
            ;
    }

    void end_gl()
    {
        glPopAttrib();

        // Restore matrices.
        // TODO: Should be merged into RenderState and removed from Graphics.

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, static_cast<GLsizei>(phys_width), static_cast<GLsizei>(phys_height));
        glOrtho(0, phys_width, phys_height, 0, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glEnable(GL_BLEND);
    }
#endif
};

Gosu::Viewport::Viewport(int phys_width, int phys_height)
    : m_impl(new Impl)
{
    m_impl->virt_width = phys_width;
    m_impl->virt_height = phys_height;
    m_impl->black_width = 0;
    m_impl->black_height = 0;

    // TODO: Should be merged into RenderState and removed from Graphics.
    const OpenGLContext current_context;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);

    set_physical_resolution(phys_width, phys_height);
}

Gosu::Viewport::~Viewport()
{
    if (current_viewport_pointer == this) {
        current_viewport_pointer = nullptr;
    }
}

int Gosu::Viewport::width() const
{
    return m_impl->virt_width;
}

int Gosu::Viewport::height() const
{
    return m_impl->virt_height;
}

void Gosu::Viewport::set_resolution(int virtual_width, int virtual_height,
                                    double horizontal_black_bar_width,
                                    double vertical_black_bar_height)
{
    if (virtual_width <= 0 || virtual_height <= 0) {
        throw std::invalid_argument("Invalid virtual resolution.");
    }

    m_impl->virt_width = virtual_width;
    m_impl->virt_height = virtual_height;
    m_impl->black_width = horizontal_black_bar_width;
    m_impl->black_height = vertical_black_bar_height;

    m_impl->update_base_transform();
}

void Gosu::Viewport::frame(const std::function<void()>& f)
{
    if (current_viewport_pointer != nullptr) {
        throw std::logic_error("Cannot nest calls to Gosu::Graphics::frame()");
    }

    // Cancel all recording or whatever that might still be in progress...
    queues.clear();

    if (m_impl->warmed_up_queues.size() == 1) {
        // If we already have a "warmed up" queue, use that instead.
        // -> All internal std::vectors will already have a lot of capacity.
        // This helps reduce allocations during normal operation.
        queues.clear();
        queues.swap(m_impl->warmed_up_queues);
    }
    else {
        // Create default draw-op queue.
        queues.emplace_back(QM_RENDER_TO_SCREEN);
    }

    queues.back().set_base_transform(m_impl->base_transform);

    const OpenGLContext current_context;

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    current_viewport_pointer = this;

    f();

    // Cancel all intermediate queues that have not been cleaned up.
    while (queues.size() > 1) {
        queues.pop_back();
    }

    flush();

    if (m_impl->black_height != 0 || m_impl->black_width != 0) {
        if (m_impl->black_height != 0) {
            // Top black bar.
            draw_rect(0, 0, width(), -m_impl->black_height, Color::BLACK, 0);
            // Bottom black bar.
            draw_rect(0, height(), width(), +m_impl->black_height, Color::BLACK, 0);
        }
        if (m_impl->black_width != 0) {
            // Left black bar.
            draw_rect(0, 0, -m_impl->black_width, height(), Color::BLACK, 0);
            // Right black bar.
            draw_rect(width(), 0, +m_impl->black_width, height(), Color::BLACK, 0);
        }
        flush();
    }

    current_viewport_pointer = nullptr;

    // Clear leftover transforms, clip rects etc.
    if (queues.size() == 1) {
        queues.swap(m_impl->warmed_up_queues);
        m_impl->warmed_up_queues.back().reset();
    }
    else {
        queues.clear();
    }
}

void Gosu::flush()
{
    current_queue().perform_draw_ops_and_code();
    current_queue().clear_queue();
}

void Gosu::gl(const std::function<void()>& f)
{
    if (current_queue().mode() == QM_RECORD_MACRO) {
        throw std::logic_error("Custom OpenGL is not allowed while creating a macro");
    }

#ifdef GOSU_IS_OPENGLES
    throw std::logic_error("Custom OpenGL ES is not supported yet");
#else
    Viewport& cg = current_viewport();

    flush();

    cg.m_impl->begin_gl();

    f();

    cg.m_impl->end_gl();
#endif
}

void Gosu::gl(Gosu::ZPos z, const std::function<void()>& f)
{
#ifdef GOSU_IS_OPENGLES
    throw std::logic_error("Custom OpenGL ES is not supported yet");
#else
    const auto wrapped_f = [f] {
        Viewport& cg = current_viewport();
        cg.m_impl->begin_gl();
        f();
        cg.m_impl->end_gl();
    };
    current_queue().gl(wrapped_f, z);
#endif
}

void Gosu::clip_to(double x, double y, double width, double height,
                             const std::function<void()>& f)
{
    double screen_height = current_viewport().m_impl->phys_height;
    current_queue().begin_clipping(x, y, width, height, screen_height);
    f();
    current_queue().end_clipping();
}

Gosu::Image Gosu::render(int width, int height, const std::function<void()>& f,
                                   unsigned image_flags)
{
    const OpenGLContext current_context;

    // Prepare for rendering at the requested size, but save the previous matrix and viewport.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glViewport(0, 0, width, height);
    // Note the flipped vertical axis in the glOrtho call, so we don't have to vertically
    // flip the texture afterward.
#ifdef GOSU_IS_OPENGLES
    glOrthof(0, width, 0, height, -1, 1); // NOLINT(readability-suspicious-call-argument)
#else
    glOrtho(0, width, 0, height, -1, 1); // NOLINT(readability-suspicious-call-argument)
#endif

    // This is the actual render-to-texture step.
    Image result = OffScreenTarget(width, height, image_flags).render([&] {
#ifndef GOSU_IS_OPENGLES
        glPushAttrib(GL_ALL_ATTRIB_BITS);
#endif
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        queues.emplace_back(QM_RENDER_TO_TEXTURE);
        f();
        queues.back().perform_draw_ops_and_code();
        queues.pop_back();
#ifndef GOSU_IS_OPENGLES
        glPopAttrib();
#endif
    });

    // Restore previous matrix and glViewport.
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);

    return result;
}

Gosu::Image Gosu::record(int width, int height, const std::function<void()>& f)
{
    queues.emplace_back(QM_RECORD_MACRO);

    f();

    std::unique_ptr<Drawable> result(new Macro(current_queue(), width, height));
    queues.pop_back();
    return Image(std::move(result));
}

void Gosu::transform(const Gosu::Transform& transform, const std::function<void()>& f)
{
    current_queue().push_transform(transform);
    f();
    current_queue().pop_transform();
}

void Gosu::draw_line(double x1, double y1, Color c1, double x2, double y2, Color c2,
                               ZPos z, BlendMode mode)
{
    DrawOp op;
    op.render_state.mode = mode;
    op.vertices_or_block_index = 2;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
    op.z = z;

    current_queue().schedule_draw_op(op);
}

void Gosu::draw_triangle(double x1, double y1, Color c1, double x2, double y2, Color c2,
                                   double x3, double y3, Color c3, ZPos z, BlendMode mode)
{
    DrawOp op;
    op.render_state.mode = mode;
    op.vertices_or_block_index = 3;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
    op.vertices[2] = DrawOp::Vertex(x3, y3, c3);
#ifdef GOSU_IS_OPENGLES
    op.vertices_or_block_index = 4;
    op.vertices[3] = op.vertices[2];
#endif
    op.z = z;

    current_queue().schedule_draw_op(op);
}

void Gosu::draw_quad(double x1, double y1, Color c1, double x2, double y2, Color c2,
                               double x3, double y3, Color c3, double x4, double y4, Color c4,
                               ZPos z, BlendMode mode)
{
    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);

    DrawOp op;
    op.render_state.mode = mode;
    op.vertices_or_block_index = 4;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
// TODO: Should be harmonized
#ifdef GOSU_IS_OPENGLES
    op.vertices[2] = DrawOp::Vertex(x3, y3, c3);
    op.vertices[3] = DrawOp::Vertex(x4, y4, c4);
#else
    op.vertices[3] = DrawOp::Vertex(x3, y3, c3);
    op.vertices[2] = DrawOp::Vertex(x4, y4, c4);
#endif
    op.z = z;

    current_queue().schedule_draw_op(op);
}

void Gosu::draw_rect(double x, double y, double width, double height, Color c, ZPos z,
                               Gosu::BlendMode mode)
{
    draw_quad(x, y, c, x + width, y, c, x, y + height, c, x + width, y + height, c, z, mode);
}

void Gosu::schedule_draw_op(const Gosu::DrawOp& op)
{
    current_queue().schedule_draw_op(op);
}

void Gosu::Viewport::set_physical_resolution(int phys_width, int phys_height)
{
    m_impl->phys_width = phys_width;
    m_impl->phys_height = phys_height;
    // TODO: Should be merged into RenderState and removed from Graphics.
    const OpenGLContext current_context;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, static_cast<GLsizei>(phys_width), static_cast<GLsizei>(phys_height));
#ifdef GOSU_IS_OPENGLES
    glOrthof(0, phys_width, phys_height, 0, -1, 1);
#else
    glOrtho(0, phys_width, phys_height, 0, -1, 1);
#endif

    m_impl->update_base_transform();
}
