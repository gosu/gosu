#ifndef GOSUIMPL_GRAPHICS_MACRO_HPP
#define GOSUIMPL_GRAPHICS_MACRO_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <boost/scoped_ptr.hpp>

class Gosu::Macro : public Gosu::ImageData
{
    DrawOpQueue queue;
    
public:
    Macro(DrawOpQueue& swapMe)
    {
        queue.swap(swapMe);
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
        queue.performDrawOps();
    }
    
    boost::optional<Gosu::GLTexInfo> glTexInfo() const
    {
        return boost::none;
    }
};

#endif

