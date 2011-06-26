#ifndef GOSUIMPL_GRAPHICS_MACRO_HPP
#define GOSUIMPL_GRAPHICS_MACRO_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/TR1.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOpQueue.hpp>
#include <cmath>
#include <algorithm>
#include <memory>

class Gosu::Macro : public Gosu::ImageData
{
    VertexArray vertexArray;
    unsigned w, h;
    
    Graphics& graphics;
public:
    Macro(Graphics& graphics, DrawOpQueue& queue)
    : graphics(graphics)
    {
        queue.compileTo(vertexArray);
        double left = 0, right = 0, top = 0, bottom = 0;
        for (VertexArray::const_iterator it = vertexArray.begin(),
             end = vertexArray.end(); it != end; ++it)
        {
            left   = std::min<double>(left,   it->vertices[0]);
            right  = std::max<double>(right,  it->vertices[0]);
            top    = std::min<double>(top,    it->vertices[1]);
            bottom = std::max<double>(bottom, it->vertices[1]);
        }
        w = std::ceil(right - left);
        h = std::ceil(bottom - top);
    }
    
    unsigned int width() const
    {
        return w;
    }
    
    unsigned int height() const
    {
        return h;
    }
    
    void draw(double x1, double y1, Color c1,
              double x2, double y2, Color c2,
              double x3, double y3, Color c3,
              double x4, double y4, Color c4,
              ZPos z, AlphaMode mode) const
    {
        std::tr1::function<void()> f = std::tr1::bind(&Macro::realDraw, this, x1, y1, x2, y2, x3, y3);
        graphics.scheduleGL(f, z);
    }
    
    void realDraw(double x1, double y1,
              double x2, double y2,
              double x3, double y3) const
    {
        // TODO: Commented out for now on the iPhone.
        // To work, it would need to reset the VertexPointer etc. after doing its work.
    #ifndef GOSU_IS_IPHONE
        glEnable(GL_BLEND);
        RenderState rs;
        rs.setTexName(1);
        rs.setAlphaMode(amDefault);
        
        // TODO: We should apply current transformations either here or in draw(), or both.
        // TODO: Also, calculate the transform as a matrix and use RenderState::setTransform.
        
        glMatrixMode(GL_MODELVIEW);
        //glPushMatrix();
        glTranslated(x1, y1, 0);
        glScaled((x2 - x1) / width(), (y3 - y1) / height(), 1);
        
        glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &vertexArray[0]);

        glDrawArrays(GL_QUADS, 0, vertexArray.size());
        glFlush();

        //glPopMatrix();
    #endif
    }
    
    const Gosu::GLTexInfo* glTexInfo() const
    {
        return 0;
    }
    
    Gosu::Bitmap toBitmap() const
    {
        throw std::logic_error("Gosu::Macro cannot be rendered as Gosu::Bitmap");
    }
    
    void insert(const Bitmap& bitmap, int x, int y)
    {
        throw std::logic_error("Gosu::Macro cannot be updated with a Gosu::Bitmap");
    }
};

#endif

