#ifndef GOSUIMPL_DRAWOP_HPP
#define GOSUIMPL_DRAWOP_HPP

#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Color.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <boost/cstdint.hpp>
#include <algorithm>
#include <set>

namespace Gosu
{
    const GLuint NO_TEXTURE = static_cast<GLuint>(-1);

    struct DrawOp
    {
        int clipX, clipY;
        unsigned clipWidth, clipHeight;
            
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

        DrawOp() { clipWidth = 0xffffffff; usedVertices = 0; chunk = 0; }
        
#ifndef GOSU_IS_IPHONE
        void perform(GLuint& currentTexName) const
        {
            if (clipWidth != 0xffffffff)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(clipX, clipY, clipWidth, clipHeight);
            }
        
            if (mode == amAdditive)
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            else
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (chunk)
            {
                if (currentTexName == NO_TEXTURE)
                    glEnable(GL_TEXTURE_2D);
                if (chunk->texName() != currentTexName)
                    glBindTexture(GL_TEXTURE_2D, chunk->texName());
                currentTexName = chunk->texName();
            }
            else if (currentTexName != NO_TEXTURE)
            {
                glDisable(GL_TEXTURE_2D);
                currentTexName = NO_TEXTURE;
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
            
            if (clipWidth != 0xffffffff)
                glDisable(GL_SCISSOR_TEST);
        }
#else
        void perform(unsigned& currentTexName) const
        {
            static GLfloat spriteVertices[8];
            static GLfloat spriteTexcoords[8];
            static boost::uint32_t spriteColors[4];
            
            // iPhone specific setup
            static bool isSetup = false;
            if (!isSetup)
            {
                // Sets up pointers and enables states needed for using vertex arrays and textures
                glVertexPointer(2, GL_FLOAT, 0, spriteVertices);
                glEnableClientState(GL_VERTEX_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, 0, spriteTexcoords);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glColorPointer(4, GL_UNSIGNED_BYTE, 0, spriteColors);
                glEnableClientState(GL_COLOR_ARRAY);
                
                isSetup = true;
            }

            if (clipWidth != 0xffffffff)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(clipX, clipY, clipWidth, clipHeight);
            }
            
            if (mode == amAdditive)
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            else
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            
            if (chunk)
            {
                if (currentTexName == NO_TEXTURE)
                    glEnable(GL_TEXTURE_2D);
                if (chunk->texName() != currentTexName)
                {
                    glBindTexture(GL_TEXTURE_2D, chunk->texName());
                }
                
                double left, top, right, bottom;
                chunk->getCoords(left, top, right, bottom);
                spriteTexcoords[0] = left, spriteTexcoords[1] = top;
                spriteTexcoords[2] = right, spriteTexcoords[3] = top;
                spriteTexcoords[4] = left, spriteTexcoords[5] = bottom;
                spriteTexcoords[6] = right, spriteTexcoords[7] = bottom;
                
                currentTexName = chunk->texName();
            }
            else if (currentTexName != NO_TEXTURE)
            {
                glDisable(GL_TEXTURE_2D);
                currentTexName = NO_TEXTURE;
            }
            
            for (int i = 0; i < usedVertices; ++i)
            {
                spriteVertices[i*2] = vertices[i].x;
                spriteVertices[i*2+1] = vertices[i].y;
                spriteColors[i] = vertices[i].c.abgr();
            }
                
            if (usedVertices == 2)
                glDrawArrays(GL_LINES, 0, 2);
            else if (usedVertices == 3)
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 3);
            else if (usedVertices == 4)
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                
            
            if (clipWidth != 0xffffffff)
                glDisable(GL_SCISSOR_TEST);
        }
        
        unsigned texName() const
        {
            return chunk ? chunk->texName() : NO_TEXTURE;
        }
#endif
        
        bool operator<(const DrawOp& other) const
        {
            return z < other.z;
        }
    };

    class DrawOpQueue
    {
        int clipX, clipY;
        unsigned clipWidth, clipHeight;
        std::multiset<DrawOp> set;

    public:
        DrawOpQueue()
        : clipWidth(0xffffffff)
        {
        }
        
        void swap(DrawOpQueue& other)
        {
            std::swap(clipX, other.clipX);
            std::swap(clipY, other.clipY);
            std::swap(clipWidth, other.clipWidth);
            std::swap(clipHeight, other.clipHeight);
            set.swap(other.set);
        }
        
        void addDrawOp(DrawOp op, ZPos z)
        {
            if (clipWidth != 0xffffffff)
            {
                op.clipX = clipX;
                op.clipY = clipY;
                op.clipWidth = clipWidth;
                op.clipHeight = clipHeight;
            }
            
            if (z == zImmediate)
            {
                GLuint currentTexName = NO_TEXTURE;
                op.perform(currentTexName);
                if (currentTexName != NO_TEXTURE)
                    glDisable(GL_TEXTURE_2D);
            }
            
            op.z = z;
            set.insert(op);
        }
        
        void beginClipping(int x, int y, unsigned width, unsigned height)
        {
            clipX = x;
            clipY = y;
            clipWidth = width;
            clipHeight = height;
        }
        
        void endClipping()
        {
            clipWidth = 0xffffffff;
        }

        void performDrawOps() const
        {
            GLuint currentTexName = NO_TEXTURE;
            
            std::multiset<DrawOp>::const_iterator cur = set.begin(), end = set.end();
            while (cur != end)
            {
                cur->perform(currentTexName);
                ++cur;
            }
            
            if (currentTexName != NO_TEXTURE)
                glDisable(GL_TEXTURE_2D);
        }
        
        void clear()
        {
            set.clear();
        }
    };
}

#endif


