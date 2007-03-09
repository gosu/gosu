#ifndef GOSUIMPL_OPENGL_TEXCHUNK_HPP
#define GOSUIMPL_OPENGL_TEXCHUNK_HPP

#include <Gosu/ImageData.hpp>
#include <GosuImpl/Graphics/Graphics.hpp>
#include <GosuImpl/OpenGL/Blit.hpp>
#include <GosuImpl/OpenGL/Texture.hpp>
#include <memory>
#include <vector>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Gosu::OpenGL::TexChunk : public Gosu::ImageData
{
    Graphics* graphics;
    BlitQueue<Blit>* queue;
    boost::shared_ptr<Texture> texture;
    int x, y, w, h, padding;

public:
    TexChunk(Graphics& graphics, BlitQueue<Blit>& queue, boost::shared_ptr<Texture> texture,
             int x, int y, int w, int h, int padding);
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
};

#endif

