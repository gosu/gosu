#ifndef GOSUIMPL_DRAWOP_HPP
#define GOSUIMPL_DRAWOP_HPP

#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Color.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/TexChunk.hpp>
#include <GosuImpl/Graphics/RenderState.hpp>
#include <boost/cstdint.hpp>

namespace Gosu
{
    struct ArrayVertex
    {
        GLfloat texCoords[2];
        boost::uint32_t color;
        GLfloat vertices[3];
    };
    typedef std::vector<ArrayVertex> VertexArray;
    
    struct DrawOp
    {
        Gosu::Transform* transform;
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

        DrawOp(Gosu::Transform& transform)
        :   transform(&transform), clipWidth(NO_CLIPPING),
            usedVertices(0), chunk(0)
        {
        }
        
        void perform(RenderState& current, const DrawOp* next) const
        {
            #ifdef GOSU_IS_IPHONE
            static const unsigned MAX_AUTOGROUP = 24;
            
            static int spriteCounter = 0;
            static GLfloat spriteVertices[12 * MAX_AUTOGROUP];
            static GLfloat spriteTexcoords[12 * MAX_AUTOGROUP];
            static boost::uint32_t spriteColors[6 * MAX_AUTOGROUP];
            
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
            #endif
            
            current.setClipRect(clipX, clipY, clipWidth, clipHeight);
            current.setTransform(transform);
            current.setAlphaMode(mode);
            
            if (chunk)
            {
                current.setTexName(chunk->texName());
                
                #ifdef GOSU_IS_IPHONE
                double left, top, right, bottom;
                chunk->getCoords(left, top, right, bottom);
                spriteTexcoords[spriteCounter*12 + 0] = left;
                spriteTexcoords[spriteCounter*12 + 1] = top;
                spriteTexcoords[spriteCounter*12 + 2] = right;
                spriteTexcoords[spriteCounter*12 + 3] = top;
                spriteTexcoords[spriteCounter*12 + 4] = left;
                spriteTexcoords[spriteCounter*12 + 5] = bottom;
                
                spriteTexcoords[spriteCounter*12 + 6] = right;
                spriteTexcoords[spriteCounter*12 + 7] = top;
                spriteTexcoords[spriteCounter*12 + 8] = left;
                spriteTexcoords[spriteCounter*12 + 9] = bottom;
                spriteTexcoords[spriteCounter*12 + 10] = right;
                spriteTexcoords[spriteCounter*12 + 11] = bottom;
                #endif
            }
            else
                current.setTexName(NO_TEXTURE);

            #ifndef GOSU_IS_IPHONE
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
            #else
            for (int i = 0; i < 3; ++i)
            {
                spriteVertices[spriteCounter*12 + i*2] = vertices[i].x;
                spriteVertices[spriteCounter*12 + i*2+1] = vertices[i].y;
                spriteColors[spriteCounter*6 + i] = vertices[i].c.abgr();
            }
            for (int i = 0; i < 3; ++i)
            {
                spriteVertices[spriteCounter*12 + 6 + i*2] = vertices[i + 1].x;
                spriteVertices[spriteCounter*12 + 6 + i*2+1] = vertices[i + 1].y;
                spriteColors[spriteCounter*6 + 3 + i] = vertices[i + 1].c.abgr();
            }
            
            ++spriteCounter;
            if (spriteCounter == MAX_AUTOGROUP or next == 0 or
                chunk == 0 or next->chunk == 0 or next->transform != transform or
                next->chunk->texName() != chunk->texName() or next->mode != mode or
                clipWidth != NO_CLIPPING or next->clipWidth != NO_CLIPPING)
            {
                glDrawArrays(GL_TRIANGLES, 0, 6 * spriteCounter);
                //if (spriteCounter > 1)
                //    printf("grouped %d quads\n", spriteCounter);
                spriteCounter = 0;
            }
            #endif
        }
        
        void compileTo(VertexArray& va) const
        {
            ArrayVertex result[4];
            
            for (int i = 0; i < 4; ++i)
            {
                result[i].vertices[0] = vertices[i].x;
                result[i].vertices[1] = vertices[i].y;
                result[i].vertices[2] = 0;
                result[i].color = vertices[i].c.abgr();
            }

            double left, top, right, bottom;
            chunk->getCoords(left, top, right, bottom);
            result[0].texCoords[0] = left,  result[0].texCoords[1] = top;
            result[1].texCoords[0] = right, result[1].texCoords[1] = top;
            result[2].texCoords[0] = right, result[2].texCoords[1] = bottom;
            result[3].texCoords[0] = left,  result[3].texCoords[1] = bottom;

            va.insert(va.end(), result, result + 4);
        }
        
        bool operator<(const DrawOp& other) const
        {
            return z < other.z;
        }
    };
}

#endif
