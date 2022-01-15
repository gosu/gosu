#include "TexChunk.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include "DrawOpQueue.hpp"
#include "Texture.hpp"
#include <stdexcept>

void Gosu::TexChunk::set_tex_info()
{
    double width = m_texture->width(), height = m_texture->height();

    m_info.tex_name = m_texture->tex_name();
    m_info.left = m_x / width;
    m_info.top = m_y / height;
    m_info.right = (m_x + m_w) / width;
    m_info.bottom = (m_y + m_h) / height;
}

Gosu::TexChunk::TexChunk(std::shared_ptr<Texture> texture, int x, int y, int w, int h, int padding)
: m_texture{move(texture)},
  m_x{x},
  m_y{y},
  m_w{w},
  m_h{h},
  m_padding{padding}
{
    set_tex_info();
}

Gosu::TexChunk::TexChunk(const TexChunk& parent, int x, int y, int w, int h)
: m_texture{parent.m_texture},
  m_x{parent.m_x + x},
  m_y{parent.m_y + y},
  m_w{w},
  m_h{h},
  m_padding{0}
{
    if (x < 0 || y < 0 || x + w > parent.m_w || y + h > parent.m_h) {
        throw std::invalid_argument{"subimage bounds exceed those of its parent"};
    }
    if (w <= 0 || h <= 0) {
        throw std::invalid_argument{"cannot create empty image"};
    }

    set_tex_info();
    m_texture->block(m_x, m_y, m_w, m_h);
}

Gosu::TexChunk::~TexChunk()
{
    m_texture->free(m_x - m_padding, m_y - m_padding, m_w + 2 * m_padding, m_h + 2 * m_padding);
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

std::unique_ptr<Gosu::ImageData> Gosu::TexChunk::subimage(int x, int y, int width, int height) const
{
    return std::unique_ptr<Gosu::ImageData>(new TexChunk(*this, x, y, width, height));
}

Gosu::Bitmap Gosu::TexChunk::to_bitmap() const
{
    return m_texture->to_bitmap(m_x, m_y, m_w, m_h);
}

void Gosu::TexChunk::insert(const Bitmap& original_bitmap, int x, int y)
{
    Bitmap clipped_bitmap;
    const Bitmap* bitmap = &original_bitmap;

    // If inserting the bitmap at the given position exceeds the boundaries of the space allocated
    // for this image on the texture, we need to clip the bitmap and insert the clipped version
    // instead.
    if (x < 0 || y < 0 || x + original_bitmap.width() > m_w || y + original_bitmap.height() > m_h) {
        // How many pixels to remove at the top and left sides.
        int clip_left = 0, clip_top = 0;
        // How large the clipped version needs to be.
        int clipped_width = original_bitmap.width(), clipped_height = original_bitmap.height();

        // Clip away pixels on the left side, if necessary.
        if (x < 0) {
            clip_left = -x;
            clipped_width -= -x;
            x = 0;
        }
        // Clip away pixels at the top, if necessary.
        if (y < 0) {
            clip_top = -y;
            clipped_height -= -y;
            y = 0;
        }
        // Clip away pixels on the right side, if necessary.
        if (x + clipped_width > m_w) {
            clipped_width = (m_w - x);
        }
        // Clip away pixels on the bottom, if necessary.
        if (y + clipped_height > m_h) {
            clipped_height = (m_h - y);
        }

        if (clipped_width <= 0 || clipped_height <= 0) return;

        clipped_bitmap.resize(clipped_width, clipped_height);
        clipped_bitmap.insert(-clip_left, -clip_top, original_bitmap);
        bitmap = &clipped_bitmap;
    }

    glBindTexture(GL_TEXTURE_2D, tex_name());
    glTexSubImage2D(GL_TEXTURE_2D, 0, m_x + x, m_y + y, bitmap->width(), bitmap->height(),
                    Color::GL_FORMAT, GL_UNSIGNED_BYTE, bitmap->data());
}
