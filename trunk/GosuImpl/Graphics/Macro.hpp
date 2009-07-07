#ifndef GOSUIMPL_GRAPHICS_MACRO_HPP
#define GOSUIMPL_GRAPHICS_MACRO_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <boost/scoped_ptr.hpp>

class Gosu::Macro : public Gosu::ImageData
{
    VertexArray vertexArray;
    
public:
    Macro(const DrawOpQueue& queue)
    {
        queue.compileTo(vertexArray);
    }
    
    unsigned int width() const
    {
        return 1337; // TODO
    }
    
    unsigned int height() const
    {
        return 1337; // TODO
    }
    
    void draw(double x1, double y1, Color c1,
              double x2, double y2, Color c2,
              double x3, double y3, Color c3,
              double x4, double y4, Color c4,
              ZPos z, AlphaMode mode) const
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslated(x1, y1, 0);
        
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 1);
        
        glInterleavedArrays(GL_T2F_C4UB_V3F, 0, &vertexArray[0]);

        glDrawArrays(GL_QUADS, 0, vertexArray.size());
        glFlush();

        glDisable(GL_TEXTURE_2D);
        
        glPopMatrix();
    }
    
    boost::optional<Gosu::GLTexInfo> glTexInfo() const
    {
        return boost::none;
    }
};

#endif

