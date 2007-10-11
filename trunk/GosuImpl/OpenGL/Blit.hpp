#ifndef GOSUIMPL_DRAWOP_HPP
#define GOSUIMPL_DRAWOP_HPP

#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Color.hpp>
#include <GosuImpl/Graphics/Graphics.hpp>
#include <set>

struct Gosu::DrawOp
{
    struct Vertex
    {
        double x, y;
        Color c;
        Vertex() {}
        Vertex(double x, double y, Color c) : x(x), y(y), c(c) {}
    };

    ZPos z;
    Vertex vertices[4];
    unsigned usedVertices;
    const TexChunk* chunk;
    AlphaMode mode;

    DrawOp() { usedVertices = 0; chunk = 0; }
    void perform() const;
    
    bool operator<(const DrawOp& other) const
    {
        return z < other.z;
    }
};

class Gosu::DrawOpQueue
{
    std::multiset<DrawOp> set;

public:
    void addDrawOp(DrawOp op, ZPos z)
    {
        op.z = z;
        set.insert(op);
    }

    void performDrawOps()
    {
        std::multiset<DrawOp>::iterator cur = set.begin(), end = set.end();
        while (cur != end)
        {
            cur->perform();
            ++cur;
        }
        set.clear();
    }
};

#endif


