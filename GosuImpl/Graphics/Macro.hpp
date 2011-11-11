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
    VertexArrays vertexArrays;
    unsigned w, h;
    
    Graphics& graphics;
public:
    Macro(Graphics& graphics, DrawOpQueue& queue)
    : graphics(graphics)
    {
        queue.compileTo(vertexArrays);
        // TODO: Not sure what is the most intuitive behavior here.
        // Also, shouldn't this respect each render state's transform? Ugh.
        float left = 0, right = 0, top = 0, bottom = 0;
        for (VertexArrays::const_iterator it = vertexArrays.begin(), end = vertexArrays.end(); it != end; ++it)
            for (int i = 0, num = it->vertices.size(); i < num; ++i)
            {
                // Don't consider anything drawn left and top of the origin.
                //left = std::min(left, it->vertices[i].vertices[0]);
                //top = std::min(top, it->vertices[i].vertices[1]);
                right = std::max(right, it->vertices[i].vertices[0]);
                bottom = std::max(bottom, it->vertices[i].vertices[1]);
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
        std::tr1::function<void()> f = std::tr1::bind(&Macro::realDraw, this, x1, y1, x2, y3);
        graphics.scheduleGL(f, z);
    }
    
    void realDraw(double x1, double y1, double x2, double y3) const
    {
        // TODO: Macros should not be split up because they have different transforms! This is insane.
        // They should be premultiplied and have the same transform by definition. Then, the transformation
        // only had to be performed here once.
        
        #ifndef GOSU_IS_IPHONE
        glEnable(GL_BLEND);
        glMatrixMode(GL_MODELVIEW);
        RenderState rs;
        
        for (VertexArrays::const_iterator it = vertexArrays.begin(), end = vertexArrays.end(); it != end; ++it)
        {
            rs.setRenderState(it->renderState);
            
            // TODO: Calculate the transform as a matrix and use RenderState::setTransform.
            glPushMatrix();
            glTranslated(x1, y1, 0);
            glScaled((x2 - x1) / width(), (y3 - y1) / height(), 1);

            glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &it->vertices[0]);
            glDrawArrays(GL_QUADS, 0, it->vertices.size());
            //glFlush();
            glPopMatrix();
        }
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

