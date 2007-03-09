#ifndef GOSUIMPL_OPENGL_BLIT_HPP
#define GOSUIMPL_OPENGL_BLIT_HPP

#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Color.hpp>
#include <GosuImpl/OpenGL/OpenGL.hpp>

struct Gosu::OpenGL::Blit
{
    struct Vertex
    {
        double x, y;
        double a, r, g, b;
        
        Vertex() {}
        Vertex(double x, double y, const Color& color)
        : x(x), y(y)
        {
            a = color.alpha() / 255.0;
            r = color.red() / 255.0;
            g = color.green() / 255.0;
            b = color.blue() / 255.0;
        }
    };

    Vertex vertices[4];
    unsigned usedVertices;
    const TexChunk* chunk;
    AlphaMode mode;

    Blit() { chunk = 0; }
    void perform();
};

#endif


