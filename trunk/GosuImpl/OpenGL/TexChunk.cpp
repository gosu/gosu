#include <GosuImpl/OpenGL/TexChunk.hpp>
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>

Gosu::OpenGL::TexChunk::TexChunk(Graphics& graphics, BlitQueue<Blit>& queue,
                                 boost::shared_ptr<Texture> texture,
                                 int x, int y, int w, int h, int padding)
: graphics(&graphics), queue(&queue), texture(texture), x(x), y(y), w(w), h(h), padding(padding)
{
}

Gosu::OpenGL::TexChunk::~TexChunk()
{
    texture->free(x - padding, y - padding);
}

unsigned int Gosu::OpenGL::TexChunk::width() const
{
    return w;
}

unsigned int Gosu::OpenGL::TexChunk::height() const
{
    return h;
}

GLuint Gosu::OpenGL::TexChunk::texName() const
{
    return texture->texName();
}

void Gosu::OpenGL::TexChunk::getCoords(double& left, double& top, double& right, double& bottom) const
{
    left = double(x) / texture->size();
    top = double(y) / texture->size();
    right = double(x + w) / texture->size();
    bottom = double(y + h) / texture->size();
}

void Gosu::OpenGL::TexChunk::draw(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    double x4, double y4, Color c4,
    ZPos z, AlphaMode mode) const
{
    Blit newBlit;

    x1 *= graphics->factorX();
    y1 *= graphics->factorY();
    x2 *= graphics->factorX();
    y2 *= graphics->factorY();
    x3 *= graphics->factorX();
    y3 *= graphics->factorY();
    x4 *= graphics->factorX();
    y4 *= graphics->factorY();
    
    newBlit.usedVertices = 4;
    newBlit.vertices[0] = Blit::Vertex(x1, y1, c1);
    newBlit.vertices[1] = Blit::Vertex(x2, y2, c2);
    newBlit.vertices[3] = Blit::Vertex(x3, y3, c3);
    newBlit.vertices[2] = Blit::Vertex(x4, y4, c4);
    newBlit.chunk = this;
    newBlit.mode = mode;

    queue->addBlit(newBlit, z);
}
