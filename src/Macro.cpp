#include "Macro.hpp"
#include "DrawOpQueue.hpp"
#include <Gosu/Image.hpp>
#include <stdexcept>
using namespace std;

struct Gosu::Macro::Impl
{
    VertexArrays vertex_arrays;
    int width, height;

    // Solves the 2x2 linear system for x:
    // (a11 a12) (x1) = (b1)
    // (a21 a22) (x2) = (b2)
    // x1, x2 are output parameters. Returns false if the matrix is singular.
    static bool solve_2x2(double a11, double a12, double a21, double a22, double b1, double b2,
                          double& x1, double& x2)
    {
        const double det = a11 * a22 - a21 * a12;
        if (det == 0) return false;
        x1 = (a22 * b1 - a12 * b2) / det;
        x2 = (a11 * b2 - a21 * b1) / det;
        return true;
    }

    Transform find_transform_for_target(double x1, double y1, double x2, double y2,
        double x3, double y3, double x4, double y4) const
    {
        // Transformation logic follows a discussion on the ImageMagick mailing
        // list (on which ImageMagick's perspective_transform.pl is based).

        // To draw a macro at an arbitrary position, we solve the following system:

        // 0, 0, 1, 0, 0, 0,     0,     0 | x1
        // 0, 0, 0, 0, 0, 1,     0,     0 | y1
        // w, 0, 1, 0, 0, 0, -x2 w,     0 | x2
        // 0, 0, 0, w, 0, 1, -y2 w,     0 | y2
        // 0, h, 1, 0, 0, 0,     0, -x3 h | x3
        // 0, 0, 0, 0, h, 1,     0, -y3 h | y3
        // w, h, 1, 0, 0, 0, -x4 w, -x4 h | x4
        // 0, 0, 0, w, h, 1, -y4 w, -y4 h | y4

        // Equivalent:

        // 0, 0, 1, 0, 0, 0,         0,         0 | x1
        // 0, 0, 0, 0, 0, 1,         0,         0 | y1
        // w, 0, 0, 0, 0, 0,     -x2 w,         0 | x2-x1
        // 0, 0, 0, w, 0, 0,     -y2 w,         0 | y2-y1
        // 0, h, 0, 0, 0, 0,         0,     -x3 h | x3-x1
        // 0, 0, 0, 0, h, 0,         0,     -y3 h | y3-y1
        // 0, 0, 0, 0, 0, 0, (x2-x4) w, (x3-x4) h | x1-x2-x3+x4
        // 0, 0, 0, 0, 0, 0, (y2-y4) w, (y3-y4) h | y1-y2-y3+y4

        // The last two rows only involve the last two variables.
        // We can directly solve this as a separate 2x2 linear system.

        // Set up 2x2 linear system of the lower right corner entries.
        const double a11 = (x2 - x4) * width;
        const double a12 = (x3 - x4) * height;
        const double a21 = (y2 - y4) * width;
        const double a22 = (y3 - y4) * height;
        const double b1 = x1 - x2 - x3 + x4;
        const double b2 = y1 - y2 - y3 + y4;

        // Solve:
        double qx, qy;
        if (!solve_2x2(a11, a12, a21, a22, b1, b2, qx, qy)) return Transform{{0}};

        // Updating the last two rows with the computed solution yields

        // 0, 0, 1, 0, 0, 0,     0,     0 | x1
        // 0, 0, 0, 0, 0, 1,     0,     0 | y1
        // w, 0, 0, 0, 0, 0, -x2 w,     0 | x2-x1
        // 0, 0, 0, w, 0, 0, -y2 w,     0 | y2-y1
        // 0, h, 0, 0, 0, 0,     0, -x3 h | x3-x1
        // 0, 0, 0, 0, h, 0,     0, -y3 h | y3-y1
        // 0, 0, 0, 0, 0, 0,     1,     0 | qx
        // 0, 0, 0, 0, 0, 0,     0,     1 | qy

        // We can use the last two rows to eliminate entries in rows 3, 4, 5, and 6:

        // 0, 0, 1, 0, 0, 0, 0, 0 | x1
        // 0, 0, 0, 0, 0, 1, 0, 0 | y1
        // w, 0, 0, 0, 0, 0, 0, 0 | x2-x1 + qx x2 w
        // 0, 0, 0, w, 0, 0, 0, 0 | y2-y1 + qx y2 w
        // 0, h, 0, 0, 0, 0, 0, 0 | x3-x1 + qy x3 h
        // 0, 0, 0, 0, h, 0, 0, 0 | y3-y1 + qy y3 h
        // 0, 0, 0, 0, 0, 0, 1, 0 | qx
        // 0, 0, 0, 0, 0, 0, 0, 1 | qy

        // Normalize and reorder rows so we can read off the solution:

        // 1, 0, 0, 0, 0, 0, 0, 0 | (x2-x1) / w + qx x2
        // 0, 1, 0, 0, 0, 0, 0, 0 | (x3-x1) / h + qy x3
        // 0, 0, 1, 0, 0, 0, 0, 0 | x1
        // 0, 0, 0, 1, 0, 0, 0, 0 | (y2-y1) / w + qx y2
        // 0, 0, 0, 0, 1, 0, 0, 0 | (y3-y1) / h + qy y3
        // 0, 0, 0, 0, 0, 1, 0, 0 | y1
        // 0, 0, 0, 0, 0, 0, 1, 0 | qx
        // 0, 0, 0, 0, 0, 0, 0, 1 | qy

        double c[8];
        c[0] = (x2 - x1) / width  + qx * x2;
        c[1] = (x3 - x1) / height + qy * x3;
        c[2] = x1;
        c[3] = (y2 - y1) / width  + qx * y2;
        c[4] = (y3 - y1) / height + qy * y3;
        c[5] = y1;
        c[6] = qx;
        c[7] = qy;

        Transform result = {{
            c[0], c[3], 0, c[6],
            c[1], c[4], 0, c[7],
            0,    0,    1, 0,
            c[2], c[5], 0, 1
        }};
        return result;
    }

    void draw_vertex_arrays(double x1, double y1, double x2, double y2, double x3, double y3,
        double x4, double y4) const
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
    double x3, double y3, Color c3, double x4, double y4, Color c4, ZPos z, BlendMode mode) const
{
    if (c1 != Color::WHITE || c2 != Color::WHITE || c3 != Color::WHITE || c4 != Color::WHITE) {
        throw invalid_argument("Macros cannot be tinted with colors");
    }
    
    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);
    
    Gosu::Graphics::gl(z, [=] { pimpl->draw_vertex_arrays(x1, y1, x2, y2, x3, y3, x4, y4); });
}

const Gosu::GLTexInfo* Gosu::Macro::gl_tex_info() const
{
    return nullptr;
}

Gosu::Bitmap Gosu::Macro::to_bitmap() const
{
    return Gosu::Graphics::render(pimpl->width, pimpl->height, [this] {
        draw(0,            0,             Color::WHITE,
             pimpl->width, 0,             Color::WHITE,
             0,            pimpl->height, Color::WHITE,
             pimpl->width, pimpl->height, Color::WHITE,
             0, BM_DEFAULT);
    }).data().to_bitmap();
}

unique_ptr<Gosu::ImageData> Gosu::Macro::subimage(int x, int y, int width, int height) const
{
    return unique_ptr<ImageData>();
}

void Gosu::Macro::insert(const Bitmap& bitmap, int x, int y)
{
    throw logic_error("Gosu::Macro cannot be updated with a Gosu::Bitmap yet");
}
