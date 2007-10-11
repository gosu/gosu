#ifndef GOSUIMPL_DRAWOP_HPP
#define GOSUIMPL_DRAWOP_HPP

#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Color.hpp>
#include <GosuImpl/Graphics/Graphics.hpp>

struct Gosu::DrawOp
{
    struct Vertex
    {
        double x, y;
        Color c;
        Vertex() {}
        Vertex(double x, double y, Color c) : x(x), y(y), c(c) {}
    };

    Vertex vertices[4];
    unsigned usedVertices;
    const TexChunk* chunk;
    AlphaMode mode;

    DrawOp() { usedVertices = 0; chunk = 0; }
    void perform();
};

class Gosu::DrawOpQueue
{
    boost::array<std::vector<DrawOp>, 256> layers;

public:
    void addDrawOp(const DrawOp& op, ZPos z)
    {
        layers[z].push_back(op);
    }

    void performDrawOps()
    {
        for (unsigned z = 0; z < 256; ++z)
        {
            for (unsigned i = 0; i < layers[z].size(); ++i)
                layers[z][i].perform();
            layers[z].clear();
        }
    }
};

#endif


