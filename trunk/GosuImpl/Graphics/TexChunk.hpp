#ifndef GOSUIMPL_GRAPHICS_TEXCHUNK_HPP
#define GOSUIMPL_GRAPHICS_TEXCHUNK_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <memory>
#include <vector>
#include <stdexcept>
#include <boost/shared_ptr.hpp>

class Gosu::TexChunk : public Gosu::ImageData
{
    Graphics* graphics;
    Transforms* transforms;
    DrawOpQueueStack* queues;
    boost::shared_ptr<Texture> texture;
    int x, y, w, h, padding;

public:
    TexChunk(Graphics& graphics, Transforms& transforms, DrawOpQueueStack& queues,
             boost::shared_ptr<Texture> texture, int x, int y, int w, int h, int padding);
    ~TexChunk();

    unsigned int width() const;
    unsigned int height() const;
    
    GLuint texName() const;
    void getCoords(double& left, double& top, double& right, double& bottom) const;

    void draw(double x1, double y1, Color c1,
        double x2, double y2, Color c2,
        double x3, double y3, Color c3,
        double x4, double y4, Color c4,
        ZPos z, AlphaMode mode) const;
        
    boost::optional<GLTexInfo> glTexInfo() const;
    Gosu::Bitmap toBitmap() const;
};

#endif

