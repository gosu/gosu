#pragma once

#include "GraphicsImpl.hpp"
#include "RenderState.hpp"
#include "TexChunk.hpp"
#include <Gosu/Color.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <cassert>

namespace Gosu
{
    struct DrawOp
    {
        // For sorting before drawing the queue.
        ZPos z;
        
        RenderState render_state;
        // Only valid if render_state.tex_name != NO_TEXTURE
        GLfloat top, left, bottom, right;
        
        // TODO: Merge with Gosu::ArrayVertex.
        struct Vertex
        {
            float x, y;
            Color c;
            Vertex() {}
            Vertex(float x, float y, Color c) : x(x), y(y), c(c) {}
        };
        Vertex vertices[4];
        
        // Number of vertices used, or: complement index of code block
        int vertices_or_block_index;
        
        void perform(const DrawOp* next) const
        {
            // This should not be called on GL code ops.
            assert (vertices_or_block_index >= 2);
            assert (vertices_or_block_index <= 4);
            
            #ifdef GOSU_IS_OPENGLES
            static const unsigned MAX_AUTOGROUP = 24;
            
            static int sprite_counter = 0;
            static float sprite_vertices[12 * MAX_AUTOGROUP];
            static float sprite_texcoords[12 * MAX_AUTOGROUP];
            static unsigned sprite_colors[6 * MAX_AUTOGROUP];
            
            // iPhone specific setup
            static bool is_setup = false;
            if (!is_setup) {
                // Sets up pointers and enables states needed for using vertex arrays and textures
                glVertexPointer(2, GL_FLOAT, 0, sprite_vertices);
                glEnableClientState(GL_VERTEX_ARRAY);
                glTexCoordPointer(2, GL_FLOAT, 0, sprite_texcoords);
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                // TODO: See if I can somehow change the format of the color pointer, or maybe
                // change the internal color representation on iOS.
                glColorPointer(4, GL_UNSIGNED_BYTE, 0, sprite_colors);
                glEnableClientState(GL_COLOR_ARRAY);
                
                is_setup = true;
            }
            
            if (render_state.texture) {
                sprite_texcoords[sprite_counter * 12 +  0] = left;
                sprite_texcoords[sprite_counter * 12 +  1] = top;
                sprite_texcoords[sprite_counter * 12 +  2] = right;
                sprite_texcoords[sprite_counter * 12 +  3] = top;
                sprite_texcoords[sprite_counter * 12 +  4] = left;
                sprite_texcoords[sprite_counter * 12 +  5] = bottom;
                
                sprite_texcoords[sprite_counter * 12 +  6] = right;
                sprite_texcoords[sprite_counter * 12 +  7] = top;
                sprite_texcoords[sprite_counter * 12 +  8] = left;
                sprite_texcoords[sprite_counter * 12 +  9] = bottom;
                sprite_texcoords[sprite_counter * 12 + 10] = right;
                sprite_texcoords[sprite_counter * 12 + 11] = bottom;
            }

            for (int i = 0; i < 3; ++i) {
                sprite_vertices[sprite_counter * 12 + i * 2 + 0] = vertices[i].x;
                sprite_vertices[sprite_counter * 12 + i * 2 + 1] = vertices[i].y;
                sprite_colors[sprite_counter * 6 + i]            = vertices[i].c.abgr();
            }
            for (int i = 0; i < 3; ++i) {
                sprite_vertices[sprite_counter * 12 + 6 + i * 2 + 0] = vertices[i + 1].x;
                sprite_vertices[sprite_counter * 12 + 6 + i * 2 + 1] = vertices[i + 1].y;
                sprite_colors[sprite_counter * 6 + 3 + i]            = vertices[i + 1].c.abgr();
            }
            
            ++sprite_counter;
            if (sprite_counter == MAX_AUTOGROUP || next == 0
                || !(next->render_state == render_state)) {
                glDrawArrays(GL_TRIANGLES, 0, 6 * sprite_counter);
                sprite_counter = 0;
            }
            #else
            if (vertices_or_block_index == 2) {
                glBegin(GL_LINES);
            }
            else if (vertices_or_block_index == 3) {
                glBegin(GL_TRIANGLES);
            }
            else { // vertices_or_block_index == 4
                glBegin(GL_QUADS);
            }
            
            for (unsigned i = 0; i < vertices_or_block_index; i++) {
                glColor4ubv(reinterpret_cast<const GLubyte*>(&vertices[i].c));
                if (render_state.texture) {
                    switch (i) {
                    case 0:
                        glTexCoord2f(left, top);
                        break;
                    case 1:
                        glTexCoord2f(right, top);
                        break;
                    case 2:
                        glTexCoord2f(right, bottom);
                        break;
                    case 3:
                        glTexCoord2f(left, bottom);
                        break;
                    }
                }
                glVertex2f(vertices[i].x, vertices[i].y);
            }
            
            glEnd();
            #endif
        }
        
        void compile_to(VertexArrays& vas) const
        {
            // Copy vertex data and apply & forget about the transform.
            // This is important because the pointed-to transform will be gone by the next
            // frame anyway.
            ArrayVertex result[4];
            for (int i = 0; i < 4; ++i) {
                result[i].vertices[0] = vertices[i].x;
                result[i].vertices[1] = vertices[i].y;
                result[i].vertices[2] = 0;
                result[i].color       = vertices[i].c.abgr();
                apply_transform(*render_state.transform,
                                result[i].vertices[0], result[i].vertices[1]);
            }
            RenderState va_render_state = render_state;
            va_render_state.transform   = 0;
            
            result[0].tex_coords[0] = left;
            result[0].tex_coords[1] = top;
            result[1].tex_coords[0] = right;
            result[1].tex_coords[1] = top;
            result[2].tex_coords[0] = right;
            result[2].tex_coords[1] = bottom;
            result[3].tex_coords[0] = left;
            result[3].tex_coords[1] = bottom;
            
            if (vas.empty() || !(vas.back().render_state == va_render_state)) {
                vas.push_back(VertexArray());
                vas.back().render_state = va_render_state;
            }
            
            vas.back().vertices.insert(vas.back().vertices.end(), result, result + 4);
        }
        
        bool operator<(const DrawOp& other) const
        {
            return z < other.z;
        }
    };
}
