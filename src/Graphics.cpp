#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Utility.hpp>
#include "DrawOp.hpp"
#include "DrawOpQueue.hpp"
#include "GraphicsImpl.hpp"
#include "LargeImageData.hpp"
#include "Macro.hpp"
#include "OffScreenTarget.hpp"
#include "Texture.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>

namespace Gosu
{
    namespace
    {
        Graphics* current_graphics_pointer = nullptr;

        Graphics& current_graphics()
        {
            if (current_graphics_pointer == nullptr) {
                throw std::logic_error{"Gosu::Graphics can only be drawn to while rendering"};
            }
            return *current_graphics_pointer;
        }

        std::vector<std::shared_ptr<Texture>> textures;

        DrawOpQueueStack queues;

        DrawOpQueue& current_queue()
        {
            if (queues.empty()) {
                throw std::logic_error{"There is no rendering queue for this operation"};
            }
            return queues.back();
        }
    }
}

struct Gosu::Graphics::Impl : private Gosu::Noncopyable
{
    unsigned virt_width = 0, virt_height = 0;
    unsigned phys_width = 0, phys_height = 0;
    double black_width = 0.0, black_height = 0.0;
    Transform base_transform;

    DrawOpQueueStack warmed_up_queues;

    void update_base_transform()
    {
        double scale_x = 1.0 * phys_width / virt_width;
        double scale_y = 1.0 * phys_height / virt_height;
        double scale_factor = std::min(scale_x, scale_y);

        Transform scale_transform = scale(scale_factor);
        Transform translate_transform = translate(black_width, black_height);
        base_transform = concat(translate_transform, scale_transform);
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

Gosu::Graphics::Graphics(unsigned phys_width, unsigned phys_height)
: m_impl(new Impl)
{
    m_impl->virt_width = phys_width;
    m_impl->virt_height = phys_height;
    m_impl->black_width = 0;
    m_impl->black_height = 0;

    // TODO: Should be merged into RenderState and removed from Graphics.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);

    set_physical_resolution(phys_width, phys_height);
}

Gosu::Graphics::~Graphics()
{
    if (current_graphics_pointer == this) {
        current_graphics_pointer = nullptr;
    }
}

unsigned Gosu::Graphics::width() const
{
    return m_impl->virt_width;
}

unsigned Gosu::Graphics::height() const
{
    return m_impl->virt_height;
}

void Gosu::Graphics::set_resolution(unsigned virtual_width, unsigned virtual_height,
                                    double horizontal_black_bar_width,
                                    double vertical_black_bar_height)
{
    if (virtual_width == 0 || virtual_height == 0) {
        throw std::invalid_argument{"Invalid virtual resolution."};
    }

    m_impl->virt_width = virtual_width;
    m_impl->virt_height = virtual_height;
    m_impl->black_width = horizontal_black_bar_width;
    m_impl->black_height = vertical_black_bar_height;

    m_impl->update_base_transform();
}

void Gosu::Graphics::frame(const std::function<void()>& f)
{
    if (current_graphics_pointer != nullptr) {
        throw std::logic_error{"Cannot nest calls to Gosu::Graphics::begin()"};
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

    ensure_current_context();
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    current_graphics_pointer = this;

    f();

    // Cancel all intermediate queues that have not been cleaned up.
    while (queues.size() > 1) queues.pop_back();

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

    glFlush();

    current_graphics_pointer = nullptr;

    // Clear leftover transforms, clip rects etc.
    if (queues.size() == 1) {
        queues.swap(m_impl->warmed_up_queues);
        m_impl->warmed_up_queues.back().reset();
    }
    else {
        queues.clear();
    }
}

void Gosu::Graphics::flush()
{
    current_queue().perform_draw_ops_and_code();
    current_queue().clear_queue();
}

void Gosu::Graphics::gl(const std::function<void()>& f)
{
    if (current_queue().mode() == QM_RECORD_MACRO) {
        throw std::logic_error{"Custom OpenGL is not allowed while creating a macro"};
    }

#ifdef GOSU_IS_OPENGLES
    throw std::logic_error{"Custom OpenGL ES is not supported yet"};
#else
    Graphics& cg = current_graphics();

    flush();

    cg.m_impl->begin_gl();

    f();

    cg.m_impl->end_gl();
#endif
}

void Gosu::Graphics::gl(Gosu::ZPos z, const std::function<void()>& f)
{
#ifdef GOSU_IS_OPENGLES
    throw std::logic_error{"Custom OpenGL ES is not supported yet"};
#else
    const auto wrapped_f = [f] {
        Graphics& cg = current_graphics();
        cg.m_impl->begin_gl();
        f();
        cg.m_impl->end_gl();
    };
    current_queue().gl(wrapped_f, z);
#endif
}

void Gosu::Graphics::clip_to(double x, double y, double width, double height,
                             const std::function<void()>& f)
{
    double screen_height = current_graphics().m_impl->phys_height;
    current_queue().begin_clipping(x, y, width, height, screen_height);
    f();
    current_queue().end_clipping();
}

Gosu::Image Gosu::Graphics::render(int width, int height, const std::function<void()>& f,
                                   unsigned image_flags)
{
    ensure_current_context();

    // Prepare for rendering at the requested size, but save the previous matrix and viewport.
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    GLint prev_viewport[4];
    glGetIntegerv(GL_VIEWPORT, prev_viewport);
    glViewport(0, 0, width, height);
    // Note the flipped vertical axis in the glOrtho call - this is so we don't have to vertically
    // flip the texture afterwards.
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
        glFlush();
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

Gosu::Image Gosu::Graphics::record(int width, int height, const std::function<void()>& f)
{
    queues.emplace_back(QM_RECORD_MACRO);

    f();

    std::unique_ptr<ImageData> result(new Macro(current_queue(), width, height));
    queues.pop_back();
    return Image(move(result));
}

void Gosu::Graphics::transform(const Gosu::Transform& transform, const std::function<void()>& f)
{
    current_queue().push_transform(transform);
    f();
    current_queue().pop_transform();
}

void Gosu::Graphics::draw_line(double x1, double y1, Color c1, double x2, double y2, Color c2,
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

void Gosu::Graphics::draw_triangle(double x1, double y1, Color c1, double x2, double y2, Color c2,
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

void Gosu::Graphics::draw_quad(double x1, double y1, Color c1, double x2, double y2, Color c2,
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

void Gosu::Graphics::draw_rect(double x, double y, double width, double height, Color c, ZPos z,
                               Gosu::BlendMode mode)
{
    draw_quad(x, y, c, x + width, y, c, x, y + height, c, x + width, y + height, c, z, mode);
}

void Gosu::Graphics::schedule_draw_op(const Gosu::DrawOp& op)
{
    current_queue().schedule_draw_op(op);
}

void Gosu::Graphics::set_physical_resolution(unsigned phys_width, unsigned phys_height)
{
    m_impl->phys_width = phys_width;
    m_impl->phys_height = phys_height;
    // TODO: Should be merged into RenderState and removed from Graphics.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, phys_width, phys_height);
#ifdef GOSU_IS_OPENGLES
    glOrthof(0, phys_width, phys_height, 0, -1, 1);
#else
    glOrtho(0, phys_width, phys_height, 0, -1, 1);
#endif

    m_impl->update_base_transform();
}

std::unique_ptr<Gosu::ImageData> Gosu::Graphics::create_image(const Bitmap& src, unsigned src_x,
                                                              unsigned src_y, unsigned src_width,
                                                              unsigned src_height, unsigned flags)
{
    static const unsigned max_size = MAX_TEXTURE_SIZE;

    // Backward compatibility: This used to be 'bool tileable'.
    if (flags == 1) flags = IF_TILEABLE;

    bool wants_retro = (flags & IF_RETRO);

    // Special case: If the texture is supposed to have hard borders, is
    // quadratic, has a size that is at least 64 pixels but no more than max_size
    // pixels and a power of two, create a single texture just for this image.
    if ((flags & IF_TILEABLE) == IF_TILEABLE && src_width == src_height &&
        (src_width & (src_width - 1)) == 0 && src_width >= 64 && src_width <= max_size) {
        std::shared_ptr<Texture> texture{new Texture(src_width, src_height, wants_retro)};
        std::unique_ptr<ImageData> data;

        // Use the source bitmap directly if the source area completely covers
        // it.
        if (src_x == 0 && src_width == src.width() && src_y == 0 && src_height == src.height()) {
            data = texture->try_alloc(src, 0);
        }
        else {
            Bitmap bmp(src_width, src_height);
            bmp.insert(0, 0, src, src_x, src_y, src_width, src_height);
            data = texture->try_alloc(bmp, 0);
        }

        if (!data) throw std::logic_error{"Internal texture block allocation error"};
        return data;
    }

    // Too large to fit on a single texture.
    if (src_width > max_size - 2 || src_height > max_size - 2) {
        Bitmap bmp(src_width, src_height);
        bmp.insert(0, 0, src, src_x, src_y, src_width, src_height);
        return std::unique_ptr<ImageData>{
                new LargeImageData(bmp, max_size - 2, max_size - 2, flags)};
    }

    Bitmap bmp = apply_border_flags(flags, src, src_x, src_y, src_width, src_height);

    // Try to put the bitmap into one of the already allocated textures.
    for (const auto& texture : textures) {
        if (texture->retro() != wants_retro) continue;

        std::unique_ptr<ImageData> data = texture->try_alloc(bmp, 1);
        if (data) return data;
    }

    // All textures are full: Create a new one.

    std::shared_ptr<Texture> texture{new Texture(max_size, max_size, wants_retro)};
    textures.push_back(texture);

    std::unique_ptr<ImageData> data{texture->try_alloc(bmp, 1)};
    if (!data.get()) throw std::logic_error("Internal texture block allocation error");

    return data;
}
