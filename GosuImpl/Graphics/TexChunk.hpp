#ifndef GOSUIMPL_GRAPHICS_TEXCHUNK_HPP
#define GOSUIMPL_GRAPHICS_TEXCHUNK_HPP

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/TR1.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <memory>
#include <vector>
#include <stdexcept>

class Gosu::TexChunk : public Gosu::ImageData
{
    Graphics& graphics;
    DrawOpQueueStack& queues;
    std::tr1::shared_ptr<Texture> texture;
    int x, y, w, h, padding;
    
    // Cached for faster access.
    GLTexInfo info;
    
public:
    TexChunk(Graphics& graphics, DrawOpQueueStack& queues,
             std::tr1::shared_ptr<Texture> texture, int x, int y, int w, int h, int padding);
    ~TexChunk();

    int width() const
    {
        return w;
    }
    
    int height() const
    {
        return h;
    }
    
    GLuint texName() const
    {
        return info.texName;
    }
    
    void draw(double x1, double y1, Color c1,
        double x2, double y2, Color c2,
        double x3, double y3, Color c3,
        double x4, double y4, Color c4,
        ZPos z, AlphaMode mode) const;
        
    const GLTexInfo* glTexInfo() const;
    Gosu::Bitmap toBitmap() const;
    void insert(const Bitmap& bitmap, int x, int y);
};

#endif

