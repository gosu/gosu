#ifndef GOSUIMPL_GRAPHICS_MACRO_HPP
#define GOSUIMPL_GRAPHICS_MACRO_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <GosuImpl/Graphics/DrawOpQueue.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include <cmath>
#include <algorithm>

class Gosu::Macro : public Gosu::ImageData
{
    VertexArray vertexArray;
    unsigned w, h;
    
public:
    Macro(const DrawOpQueue& queue)
    {
        queue.compileTo(vertexArray);
        double left = 0, right = 0, top = 0, bottom = 0;
        BOOST_FOREACH(const ArrayVertex& av, vertexArray)
        {
            left = std::min<double>(left, av.vertices[0]);
            right = std::max<double>(right, av.vertices[0]);
            top = std::min<double>(top, av.vertices[1]);
            bottom = std::max<double>(bottom, av.vertices[1]);
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
#ifndef GOSU_IS_IPHONE
        // Commented out for now on the iPhone.
        // To work, it would need to reset the VertexPointer etc. after doing its work.
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslated(x1, y1, 0);
        glScaled((x2 - x1) / width(), (y3 - y1) / height(), 1);
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 1);
        
        glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &vertexArray[0]);

        glDrawArrays(GL_QUADS, 0, vertexArray.size());
        glFlush();

        glDisable(GL_TEXTURE_2D);
        
        glPopMatrix();
#endif
    }
    
    boost::optional<Gosu::GLTexInfo> glTexInfo() const
    {
        return boost::none;
    }
    
    Gosu::Bitmap toBitmap() const
    {
        throw std::logic_error("Gosu::Macro cannot be rendered to Gosu::Bitmap");
    }
};

#endif

