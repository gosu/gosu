#include "TexChunk.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include "DrawOpQueue.hpp"
#include "Texture.hpp"
#include <stdexcept>

Gosu::TexChunk::TexChunk(const std::shared_ptr<Texture>& texture, const Rect& rect,
                         const std::shared_ptr<void>& rect_handle)
    : m_texture(texture),
      m_rect(rect),
      m_info { .tex_name = texture->tex_name(),
               .left = 1.0 * m_rect.x / texture->width(),
               .right = 1.0 * (m_rect.x + m_rect.width) / texture->width(),
               .top = 1.0 * m_rect.y / texture->height(),
               .bottom = 1.0 * (m_rect.y + m_rect.height) / texture->height() },
      m_rect_handle(rect_handle)
{
    if (!Rect::covering(*m_texture).contains(rect)) {
        throw std::invalid_argument("Gosu::TexChunk exceeds its Gosu::Texture");
    }
    if (rect.width <= 0 || rect.height <= 0) {
        throw std::invalid_argument("Gosu::TexChunk cannot be empty");
    }
}

void Gosu::TexChunk::draw(double x1, double y1, Color c1, double x2, double y2, Color c2, //
                          double x3, double y3, Color c3, double x4, double y4, Color c4, //
                          ZPos z, BlendMode mode) const
{
    DrawOp op;
    op.render_state.texture = m_texture;
    op.render_state.mode = mode;

    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);

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
    op.left = m_info.left;
    op.top = m_info.top;
    op.right = m_info.right;
    op.bottom = m_info.bottom;

    op.z = z;
    Graphics::schedule_draw_op(op);
}

std::unique_ptr<Gosu::ImageData> Gosu::TexChunk::subimage(const Rect& rect) const
{
    // Note: m_rect is relative to m_texture, but rect should be relative to m_rect.
    if (!Rect::covering(*this).contains(rect)) {
        throw std::invalid_argument("Gosu::TexChunk::subimage cannot exceed parent size");
    }
    const Rect nested_rect { m_rect.x + rect.x, m_rect.y + rect.y, rect.width, rect.height };
    return std::make_unique<TexChunk>(m_texture, nested_rect, m_rect_handle);
}

Gosu::Bitmap Gosu::TexChunk::to_bitmap() const
{
    return m_texture->to_bitmap(m_rect);
}

void Gosu::TexChunk::insert(const Bitmap& bitmap, int x, int y)
{
    Bitmap clipped_bitmap;
    const Bitmap* source = &bitmap;

    Rect target_rect { x, y, bitmap.width(), bitmap.height() };
    int offset_x = 0, offset_y = 0;
    // If inserting the bitmap at the given position exceeds the boundaries of the space allocated
    // for this image on the texture, we need to clip the bitmap and insert the clipped version
    // instead.
    if (!Rect::covering(*this).contains(target_rect)) {
        target_rect.clip_to(Rect::covering(*this), &offset_x, &offset_y);

        if (target_rect.empty()) {
            return;
        }

        clipped_bitmap.resize(target_rect.width, target_rect.height);
        clipped_bitmap.insert(bitmap, -offset_x, -offset_y);
        source = &clipped_bitmap;
    }

    glBindTexture(GL_TEXTURE_2D, tex_name());
    glTexSubImage2D(GL_TEXTURE_2D, 0, m_rect.x + x + offset_x, m_rect.y + y + offset_y, //
                    source->width(), source->height(), Color::GL_FORMAT, GL_UNSIGNED_BYTE,
                    source->data());
}
