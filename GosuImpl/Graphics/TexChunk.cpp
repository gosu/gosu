#include <GosuImpl/Graphics/TexChunk.hpp>
#include <GosuImpl/Graphics/Texture.hpp>
#include <GosuImpl/Graphics/DrawOpQueue.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>

Gosu::TexChunk::TexChunk(Graphics& graphics, DrawOpQueueStack& queues,
    std::tr1::shared_ptr<Texture> texture, int x, int y, int w, int h, int padding)
: graphics(graphics), queues(queues), texture(texture), x(x), y(y), w(w), h(h), padding(padding)
{
    info.texName = texture->texName();
    float textureSize = texture->size();
    info.left = x / textureSize;
    info.top = y / textureSize;
    info.right = (x + w) / textureSize;
    info.bottom = (y + h) / textureSize;
}

Gosu::TexChunk::~TexChunk()
{
    texture->free(x - padding, y - padding);
}

void Gosu::TexChunk::draw(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    double x4, double y4, Color c4,
    ZPos z, AlphaMode mode) const
{
    DrawOp op;
    op.renderState.texName = texName();
    op.renderState.mode = mode;
    
    reorderCoordinatesIfNecessary(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);
    
    op.verticesOrBlockIndex = 4;
    op.vertices[0] = DrawOp::Vertex(x1, y1, c1);
    op.vertices[1] = DrawOp::Vertex(x2, y2, c2);
// TODO: Should be harmonized
#ifdef GOSU_IS_IPHONE
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
    queues.back().scheduleDrawOp(op);
}

const Gosu::GLTexInfo* Gosu::TexChunk::glTexInfo() const
{
    return &info;
}

Gosu::Bitmap Gosu::TexChunk::toBitmap() const
{
    return texture->toBitmap(x, y, w, h);
}

void Gosu::TexChunk::insert(const Bitmap& original, int x, int y)
{
    // TODO: Should respect borderFlags.
    
    Bitmap alternate;
    const Bitmap* bitmap = &original;
    if (x < 0 || y < 0 || x + original.width() > w || y + original.height() > h)
    {
        int offsetX = 0, offsetY = 0, trimmedWidth = original.width(), trimmedHeight = original.height();
        if (x < 0)
            offsetX = x, trimmedWidth  += x, x = 0;
        if (y < 0)
            offsetY = y, trimmedHeight += y, y = 0;
        if (x + trimmedWidth > w)
            trimmedWidth  -= (w - x - trimmedWidth);
        if (y + trimmedHeight > h)
            trimmedHeight -= (h - y - trimmedHeight);
            
        if (trimmedWidth <= 0 || trimmedHeight <= 0)
            return;
        
        alternate.resize(trimmedWidth, trimmedHeight);
        alternate.insert(original, offsetX, offsetY);
        bitmap = &alternate;
    }
    
    glBindTexture(GL_TEXTURE_2D, texName());
    glTexSubImage2D(GL_TEXTURE_2D, 0, this->x + x, this->y + y, bitmap->width(), bitmap->height(),
        Color::GL_FORMAT, GL_UNSIGNED_BYTE, bitmap->data());
}
