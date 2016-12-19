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
    
    VertexArrays vertexArrays;
    int width, height;
    
    Transform findTransformForTarget(Float x1, Float y1, Float x2, Float y2, Float x3, Float y3, Float x4, Float y4) const
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
        
        static const Transform nullTransform = {{ 0 }};
        
        // Row 7 is completely useless
        if (x2 == x4 && x3 == x4)
            return nullTransform;
        // Row 8 is completely useless
        if (y2 == y3 && y3 == y4)
            return nullTransform;
        // Col 7 is completely useless
        if (x2 == x4 && y2 == y4)
            return nullTransform;
        // Col 8 is completely useless
        if (x3 == x4 && y3 == y4)
            return nullTransform;
        
        Float c[8];
        
        // Rows 1, 2
        c[2] = x1, c[5] = y1;
        
        // The logic below assumes x2 != x4, i.e. row7 can be used to eliminate
        // the leftmost value in row 8 and afterwards the values in rows 3 & 4.
        // If x2 == x4, we need to exchange rows 7 and 8.
        
        // TODO: x2==x4 is the normal case where an image is
        // drawn upright; the code should rather swap in the rare case that x3==x4!
        
        Float leftCell7 = (x2-x4)*width, rightCell7 = (x3-x4)*height, origRightSide7 = (x1-x2-x3+x4);
        Float leftCell8 = (y2-y4)*width, rightCell8 = (y3-y4)*height, origRightSide8 = (y1-y2-y3+y4);
        
        bool swapRows78 = x2 == x4;
        if (swapRows78)
        {
            std::swap(leftCell7, leftCell8);
            std::swap(rightCell7, rightCell8);
            std::swap(origRightSide7, origRightSide8);
        }
        
        // 0, 0, 1, 0, 0, 0,         0,           0 | x1
        // 0, 0, 0, 0, 0, 1,         0,           0 | y1
        // w, 0, 0, 0, 0, 0,      -x2w,           0 | x2-x1
        // 0, 0, 0, w, 0, 0,      -y2w,           0 | y2-y1
        // 0, h, 0, 0, 0, 0,         0,        -x3h | x3-x1
        // 0, 0, 0, 0, h, 0,         0,        -y3h | y3-y1
        // 0, 0, 0, 0, 0, 0, leftCell7, rightCell7 | origRightSide7
        // 0, 0, 0, 0, 0, 0, leftCell8, rightCell8 | origRightSide8
        
        // Use row 7 to eliminate the left cell in row 8
        // Row8 = Row8 - factor78 * Row7
        Float factor78 = leftCell8 / leftCell7;
        Float remCell8 = rightCell8 - rightCell7 * factor78;
        Float rightSide8 = origRightSide8 - origRightSide7 * factor78;
        c[7] = rightSide8 / remCell8;
        
        // 0, 0, 1, 0, 0, 0,         0,          0 | x1
        // 0, 0, 0, 0, 0, 1,         0,          0 | y1
        // w, 0, 0, 0, 0, 0,      -x2w,          0 | x2-x1
        // 0, 0, 0, w, 0, 0,      -y2w,          0 | y2-y1
        // 0, h, 0, 0, 0, 0,         0,       -x3h | x3-x1
        // 0, 0, 0, 0, h, 0,         0,       -y3h | y3-y1
        // 0, 0, 0, 0, 0, 0, leftCell7, rightCell7 | origRightSide7
        // 0, 0, 0, 0, 0, 0,         0,   remCell8 | rightSide8
        
        // Use the remaining value in row 8 to eliminate the right value in row 7.
        // Row7 = Row7 - factor87 * Row8
        Float factor87 = rightCell7 / remCell8;
        Float remCell7 = leftCell7;
        Float rightSide7 = origRightSide7 - rightSide8 * factor87;
        c[6] = rightSide7 / remCell7;
        
        // 0, 0, 1, 0, 0, 0,        0,        0 | x1
        // 0, 0, 0, 0, 0, 1,        0,        0 | y1
        // w, 0, 0, 0, 0, 0,     -x2w,        0 | x2-x1
        // 0, 0, 0, w, 0, 0,     -y2w,        0 | y2-y1
        // 0, h, 0, 0, 0, 0,        0,     -x3h | x3-x1
        // 0, 0, 0, 0, h, 0,        0,     -y3h | y3-y1
        // 0, 0, 0, 0, 0, 0, remCell7,        0 | rightSide7
        // 0, 0, 0, 0, 0, 0,        0, remCell8 | rightSide8
        
        // Use the new rows 7 and 8 to calculate c0, c1, c3 & c4.
        // Row3 = Row3 - factor73 * Row7
        Float factor73 = -x2*width / remCell7;
        Float remCell3 = width;
        Float rightSide3 = (x2-x1) - rightSide7 * factor73;
        c[0] = rightSide3 / remCell3;
        // Row4 = Row4 - factor74 * Row7
        Float factor74 = -y2*width / remCell7;
        Float remCell4 = width;
        Float rightSide4 = (y2-y1) - rightSide7 * factor74;
        c[3] = rightSide4 / remCell4;
        // Row5 = Row5 - factor85 * Row7
        Float factor85 = -x3*height / remCell8;
        Float remCell5 = height;
        Float rightSide5 = (x3-x1) - rightSide8 * factor85;
        c[1] = rightSide5 / remCell5;
        // Row6 = Row6 - factor86 * Row8
        Float factor86 = -y3*height / remCell8;
        Float remCell6 = height;
        Float rightSide6 = (y3-y1) - rightSide8 * factor86;
        c[4] = rightSide6 / remCell6;
        
        if (swapRows78)
            std::swap(c[6], c[7]);
        
        // Let's hope I never have to debug/understand this again! :D
        
        Transform result = {{
            c[0], c[3], 0, c[6],
            c[1], c[4], 0, c[7],
            0, 0, 1, 0,
            c[2], c[5], 0, 1
        }};
        return result;
    }
    
    void drawVertexArrays(Float x1, Float y1, Float x2, Float y2, Float x3, Float y3, Float x4, Float y4) const
    {
        // TODO: Macros should not be split up just because they have different transforms! This is insane.
        // They should be premultiplied and have the same transform by definition. Then the transformation
        // only has to be performed once.
        
    #ifndef GOSU_IS_OPENGLES
        glEnable(GL_BLEND);
        glMatrixMode(GL_MODELVIEW);
        
        Transform transform =
        findTransformForTarget(x1, y1, x2, y2, x3, y3, x4, y4);
        
        for (const auto& vertexArray : vertexArrays)
        {
            glPushMatrix();
            vertexArray.renderState.apply();
            glMultMatrixd(&transform[0]);
            glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &vertexArray.vertices[0]);
            glDrawArrays(GL_QUADS, 0, (GLsizei)vertexArray.vertices.size());
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
    queue.compileTo(pimpl->vertexArrays);
}

int Gosu::Macro::width() const
{
    return pimpl->width;
}

int Gosu::Macro::height() const
{
    return pimpl->height;
}

void Gosu::Macro::draw(double x1, double y1, Color c1,
    double x2, double y2, Color c2,
    double x3, double y3, Color c3,
    double x4, double y4, Color c4,
    ZPos z, AlphaMode mode) const
{
    if (c1 != Color::WHITE || c2 != Color::WHITE || c3 != Color::WHITE || c4 != Color::WHITE)
        throw std::invalid_argument("Macros cannot be tinted with colors yet");
    
    reorderCoordinatesIfNecessary(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);
    
    std::function<void()> f = std::bind(&Impl::drawVertexArrays, pimpl, x1, y1, x2, y2, x3, y3, x4, y4);
    Gosu::Graphics::scheduleGL(f, z);
}

const Gosu::GLTexInfo* Gosu::Macro::glTexInfo() const
{
    return nullptr;
}

Gosu::Bitmap Gosu::Macro::toBitmap() const
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
