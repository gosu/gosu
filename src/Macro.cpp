#include "Macro.hpp"
#include "DrawOpQueue.hpp"
#include <cmath>
#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>

struct Gosu::Macro::Impl
{
    typedef double Float;
    
    VertexArrays vertex_arrays;
    int width, height;
    
    Transform find_transform_for_target(Float x1, Float y1, Float x2, Float y2,
        Float x3, Float y3, Float x4, Float y4) const
    {
        // Transformation logic follows a discussion on the ImageMagick mailing
        // list (on which ImageMagick's perspective_transform.pl is based).
        
        // To draw a macro at an arbitrary position, we solve the following system:
        
        // 0, 0, 1, 0, 0, 0,    0,    0 | x1
        // 0, 0, 0, 0, 0, 1,    0,    0 | y1
        // w, 0, 1, 0, 0, 0, -x2w,    0 | x2
        // 0, 0, 0, w, 0, 1, -y2w,    0 | y2
        // 0, h, 1, 0, 0, 0,    0, -x3h | x3
        // 0, 0, 0, 0, h, 1,    0, -y3h | y3
        // w, h, 1, 0, 0, 0, -x4w, -x4h | x4
        // 0, 0, 0, w, h, 1, -y4w, -y4h | y4
        
        // Equivalent:
        
        // 0, 0, 1, 0, 0, 0,        0,        0 | x1
        // 0, 0, 0, 0, 0, 1,        0,        0 | y1
        // w, 0, 0, 0, 0, 0,     -x2w,        0 | x2-x1
        // 0, 0, 0, w, 0, 0,     -y2w,        0 | y2-y1
        // 0, h, 0, 0, 0, 0,        0,     -x3h | x3-x1
        // 0, 0, 0, 0, h, 0,        0,     -y3h | y3-y1
        // 0, 0, 0, 0, 0, 0, (x2-x4)w, (x3-x4)h | x1-x2-x3+x4
        // 0, 0, 0, 0, 0, 0, (y2-y4)w, (y3-y4)h | y1-y2-y3+y4
        
        // Since this matrix is relatively sparse, we unroll all three solving paths.
        
        static const Transform null_transform = {{ 0 }};
        
        // Row 7 is completely useless
        if (x2 == x4 && x3 == x4) return null_transform;
        // Row 8 is completely useless
        if (y2 == y3 && y3 == y4) return null_transform;
        // Col 7 is completely useless
        if (x2 == x4 && y2 == y4) return null_transform;
        // Col 8 is completely useless
        if (x3 == x4 && y3 == y4) return null_transform;
        
        Float c[8];
        
        // Rows 1, 2
        c[2] = x1, c[5] = y1;
        
        // The logic below assumes x2 != x4, i.e. row7 can be used to eliminate
        // the leftmost value in row 8 and afterwards the values in rows 3 & 4.
        // If x2 == x4, we need to exchange rows 7 and 8.
        
        // TODO: x2==x4 is the normal case where an image is
        // drawn upright; the code should rather swap in the rare case that x3==x4!
        
        Float left_cell7 = (x2 - x4) * width;
        Float right_cell7 = (x3 - x4) * height;
        Float orig_right_side7 = (x1 - x2 - x3 + x4);
        Float left_cell8 = (y2 - y4) * width;
        Float right_cell8 = (y3 - y4) * height;
        Float orig_right_side8 = (y1 - y2 - y3 + y4);
        
        bool swap_rows78 = x2 == x4;
        if (swap_rows78) {
            std::swap(left_cell7, left_cell8);
            std::swap(right_cell7, right_cell8);
            std::swap(orig_right_side7, orig_right_side8);
        }
        
        // 0, 0, 1, 0, 0, 0,         0,           0 | x1
        // 0, 0, 0, 0, 0, 1,         0,           0 | y1
        // w, 0, 0, 0, 0, 0,      -x2w,           0 | x2-x1
        // 0, 0, 0, w, 0, 0,      -y2w,           0 | y2-y1
        // 0, h, 0, 0, 0, 0,         0,        -x3h | x3-x1
        // 0, 0, 0, 0, h, 0,         0,        -y3h | y3-y1
        // 0, 0, 0, 0, 0, 0, left_cell7, right_cell7 | orig_right_side7
        // 0, 0, 0, 0, 0, 0, left_cell8, right_cell8 | orig_right_side8
        
        // Use row 7 to eliminate the left cell in row 8
        // Row8 = Row8 - factor78 * Row7
        Float factor78 = left_cell8 / left_cell7;
        Float rem_cell8 = right_cell8 - right_cell7 * factor78;
        Float right_side8 = orig_right_side8 - orig_right_side7 * factor78;
        c[7] = right_side8 / rem_cell8;
        
        // 0, 0, 1, 0, 0, 0,         0,          0 | x1
        // 0, 0, 0, 0, 0, 1,         0,          0 | y1
        // w, 0, 0, 0, 0, 0,      -x2w,          0 | x2-x1
        // 0, 0, 0, w, 0, 0,      -y2w,          0 | y2-y1
        // 0, h, 0, 0, 0, 0,         0,       -x3h | x3-x1
        // 0, 0, 0, 0, h, 0,         0,       -y3h | y3-y1
        // 0, 0, 0, 0, 0, 0, left_cell7, right_cell7 | orig_right_side7
        // 0, 0, 0, 0, 0, 0,         0,   rem_cell8 | right_side8
        
        // Use the remaining value in row 8 to eliminate the right value in row 7.
        // Row7 = Row7 - factor87 * Row8
        Float factor87 = right_cell7 / rem_cell8;
        Float rem_cell7 = left_cell7;
        Float right_side7 = orig_right_side7 - right_side8 * factor87;
        c[6] = right_side7 / rem_cell7;
        
        // 0, 0, 1, 0, 0, 0,        0,        0 | x1
        // 0, 0, 0, 0, 0, 1,        0,        0 | y1
        // w, 0, 0, 0, 0, 0,     -x2w,        0 | x2-x1
        // 0, 0, 0, w, 0, 0,     -y2w,        0 | y2-y1
        // 0, h, 0, 0, 0, 0,        0,     -x3h | x3-x1
        // 0, 0, 0, 0, h, 0,        0,     -y3h | y3-y1
        // 0, 0, 0, 0, 0, 0, rem_cell7,        0 | right_side7
        // 0, 0, 0, 0, 0, 0,        0, rem_cell8 | right_side8
        
        // Use the new rows 7 and 8 to calculate c0, c1, c3 & c4.
        // Row3 = Row3 - factor73 * Row7
        Float factor73 = -x2 * width / rem_cell7;
        Float rem_cell3 = width;
        Float right_side3 = (x2 - x1) - right_side7 * factor73;
        c[0] = right_side3 / rem_cell3;
        // Row4 = Row4 - factor74 * Row7
        Float factor74 = -y2 * width / rem_cell7;
        Float rem_cell4 = width;
        Float right_side4 = (y2 - y1) - right_side7 * factor74;
        c[3] = right_side4 / rem_cell4;
        // Row5 = Row5 - factor85 * Row7
        Float factor85 = -x3 * height / rem_cell8;
        Float rem_cell5 = height;
        Float right_side5 = (x3 - x1) - right_side8 * factor85;
        c[1] = right_side5 / rem_cell5;
        // Row6 = Row6 - factor86 * Row8
        Float factor86 = -y3 * height / rem_cell8;
        Float rem_cell6 = height;
        Float right_side6 = (y3 - y1) - right_side8 * factor86;
        c[4] = right_side6 / rem_cell6;
        
        if (swap_rows78) {
            std::swap(c[6], c[7]);
        }
        
        // Let's hope I never have to debug/understand this again! :D
        
        Transform result = {{
            c[0], c[3], 0, c[6],
            c[1], c[4], 0, c[7],
            0, 0, 1, 0,
            c[2], c[5], 0, 1
        }};
        return result;
    }
    
    void draw_vertex_arrays(Float x1, Float y1, Float x2, Float y2, Float x3, Float y3,
        Float x4, Float y4) const
    {
        // TODO: Macros should not be split up just because they have different transforms.
        // They should be premultiplied and have the same transform by definition. Then the
        // transformation only has to be performed once.
        
    #ifndef GOSU_IS_OPENGLES
        glEnable(GL_BLEND);
        glMatrixMode(GL_MODELVIEW);
        
        Transform transform = find_transform_for_target(x1, y1, x2, y2, x3, y3, x4, y4);
        
        for (const auto& vertex_array : vertex_arrays) {
            glPushMatrix();
            vertex_array.render_state.apply();
            glMultMatrixd(&transform[0]);
            glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &vertex_array.vertices[0]);
            glDrawArrays(GL_QUADS, 0, (GLsizei) vertex_array.vertices.size());
            glPopMatrix();
        }
    #endif
    }
};

Gosu::Macro::Macro(DrawOpQueue& queue, int width, int height)
: pimpl(new Impl)
{
    pimpl->width = width;
    pimpl->height = height;
    queue.compile_to(pimpl->vertex_arrays);
}

int Gosu::Macro::width() const
{
    return pimpl->width;
}

int Gosu::Macro::height() const
{
    return pimpl->height;
}

void Gosu::Macro::draw(double x1, double y1, Color c1, double x2, double y2, Color c2,
    double x3, double y3, Color c3, double x4, double y4, Color c4, ZPos z, AlphaMode mode) const
{
    if (c1 != Color::WHITE || c2 != Color::WHITE || c3 != Color::WHITE || c4 != Color::WHITE) {
        throw std::invalid_argument("Macros cannot be tinted with colors");
    }
    
    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);
    
    std::function<void ()> f = [=] { pimpl->draw_vertex_arrays(x1, y1, x2, y2, x3, y3, x4, y4); };
    Gosu::Graphics::gl(f, z);
}

const Gosu::GLTexInfo* Gosu::Macro::gl_tex_info() const
{
    return nullptr;
}

Gosu::Bitmap Gosu::Macro::to_bitmap() const
{
    throw std::logic_error("Gosu::Macro cannot be rendered as Gosu::Bitmap yet");
}

std::unique_ptr<Gosu::ImageData> Gosu::Macro::subimage(int x, int y, int width, int height) const
{
    return std::unique_ptr<ImageData>();
}

void Gosu::Macro::insert(const Bitmap& bitmap, int x, int y)
{
    throw std::logic_error("Gosu::Macro cannot be updated with a Gosu::Bitmap yet");
}
