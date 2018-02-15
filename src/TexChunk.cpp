#include "TexChunk.hpp"
#include "DrawOpQueue.hpp"
#include "Texture.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <stdexcept>

using namespace std;

void Gosu::TexChunk::set_tex_info()
{
    double texture_size = texture->size();

    info.tex_name = texture->tex_name();
    info.left   = x / texture_size;
    info.top    = y / texture_size;
    info.right  = (x + w) / texture_size;
    info.bottom = (y + h) / texture_size;
}

Gosu::TexChunk::TexChunk(shared_ptr<Texture> texture, int x, int y, int w, int h, int padding)
: texture(move(texture)), x(x), y(y), w(w), h(h), padding(padding)
{
    set_tex_info();
}

Gosu::TexChunk::TexChunk(const TexChunk& parent, int x, int y, int w, int h)
: texture(parent.texture), x(parent.x + x), y(parent.y + y), w(w), h(h), padding(0)
{
    if (x < 0 || y < 0 || x + w > parent.w || y + h > parent.h) {
        throw invalid_argument("subimage bounds exceed those of its parent");
    }
    if (w <= 0 || h <= 0) {
        throw invalid_argument("cannot create empty image");
    }
    
    set_tex_info();
    texture->block(this->x, this->y, this->w, this->h);
}

Gosu::TexChunk::~TexChunk()
{
    texture->free(x - padding, y - padding, w + 2 * padding, h + 2 * padding);
}

void Gosu::TexChunk::draw(double x1, double y1, Color c1, double x2, double y2, Color c2,
    double x3, double y3, Color c3, double x4, double y4, Color c4, ZPos z, AlphaMode mode) const
{
    DrawOp op;
    op.render_state.texture = texture;
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
    op.left = info.left;
    op.top = info.top;
    op.right = info.right;
    op.bottom = info.bottom;
    
    op.z = z;
    Graphics::schedule_draw_op(op);
}

unique_ptr<Gosu::ImageData> Gosu::TexChunk::subimage(int x, int y, int width, int height) const
{
    return unique_ptr<Gosu::ImageData>(new TexChunk(*this, x, y, width, height));
}

Gosu::Bitmap Gosu::TexChunk::to_bitmap() const
{
    return texture->to_bitmap(x, y, w, h);
}

void Gosu::TexChunk::insert(const Bitmap& original, int x, int y)
{
    Bitmap alternate;
    const Bitmap* bitmap = &original;
    
    if (x < 0 || y < 0 || x + original.width() > w || y + original.height() > h) {
        int offset_x = 0, offset_y = 0;
        int trimmed_width = original.width(), trimmed_height = original.height();
        if (x < 0) {
            offset_x = x;
            trimmed_width += x;
            x = 0;
        }
        if (y < 0) {
            offset_y = y;
            trimmed_height += y;
            y = 0;
        }
        if (x + trimmed_width > w) {
            trimmed_width -= (w - x - trimmed_width);
        }
        if (y + trimmed_height > h) {
            trimmed_height -= (h - y - trimmed_height);
        }

        if (trimmed_width <= 0 || trimmed_height <= 0) return;
        
        alternate.resize(trimmed_width, trimmed_height);
        alternate.insert(original, offset_x, offset_y);
        bitmap = &alternate;
    }
    
    glBindTexture(GL_TEXTURE_2D, tex_name());
    glTexSubImage2D(GL_TEXTURE_2D, 0, this->x + x, this->y + y, bitmap->width(), bitmap->height(),
        Color::GL_FORMAT, GL_UNSIGNED_BYTE, bitmap->data());
}
