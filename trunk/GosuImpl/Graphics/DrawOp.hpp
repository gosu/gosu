#ifndef GOSUIMPL_DRAWOP_HPP
#define GOSUIMPL_DRAWOP_HPP

#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Color.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <set>

namespace Gosu
{
    struct DrawOp
    {
        enum { BEGIN_CLIPPING = 5, END_CLIPPING = 6 };
    
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
        
        void perform() const
        {
            if (usedVertices == BEGIN_CLIPPING)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(vertices[0].x, vertices[0].y, vertices[1].x, vertices[1].y);
                return;
            }
            else if (usedVertices == END_CLIPPING)
            {
                glDisable(GL_SCISSOR_TEST);
            }
        
            if (mode == amAdditive)
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            else
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (chunk)
            {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, chunk->texName());
            }

            if (usedVertices == 2)
                glBegin(GL_LINES);
            else if (usedVertices == 3)
                glBegin(GL_TRIANGLES);
            else if (usedVertices == 4)
                glBegin(GL_QUADS);

            double left, top, right, bottom;
            if (chunk)
                chunk->getCoords(left, top, right, bottom);

            for (unsigned i = 0; i < usedVertices; i++)
            {
                glColor4f(vertices[i].c.red() / 255.0, vertices[i].c.green() / 255.0,
                          vertices[i].c.blue() / 255.0, vertices[i].c.alpha() / 255.0);
                if (chunk)
                    switch (i)
                    {
                    case 0:
                        glTexCoord2d(left, top);
                        break;
                    case 1:
                        glTexCoord2d(right, top);
                        break;
                    case 2:
                        glTexCoord2d(right, bottom);
                        break;
                    case 3:
                        glTexCoord2d(left, bottom);
                        break;
                    }
                glVertex2d(vertices[i].x, vertices[i].y);
            }

            glEnd();

            if (chunk)
                glDisable(GL_TEXTURE_2D);
        }
        
        bool operator<(const DrawOp& other) const
        {
            return z < other.z; // optimization starts TODAY
        }
    };

    class DrawOpQueue
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
}

#endif


