#pragma once

#include "GraphicsImpl.hpp"
#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <memory>
#include <stdexcept>
#include <vector>

class Gosu::TexChunk : public Gosu::ImageData
{
    std::shared_ptr<Texture> texture;
    int x, y, w, h, padding;
    
    // Cached for faster access.
    GLTexInfo info;
    
    void set_tex_info();
    
public:
    TexChunk(std::shared_ptr<Texture> texture, int x, int y, int w, int h, int padding);
    TexChunk(const TexChunk& parent_chunk, int x, int y, int w, int h);
    ~TexChunk();

    int width() const
    {
        return w;
    }
    
    int height() const
    {
        return h;
    }
    
    GLuint tex_name() const
    {
        return info.tex_name;
    }
    
    void draw(double x1, double y1, Color c1,
        double x2, double y2, Color c2,
        double x3, double y3, Color c3,
        double x4, double y4, Color c4,
        ZPos z, AlphaMode mode) const;

    const GLTexInfo* gl_tex_info() const;
    Gosu::Bitmap to_bitmap() const;
    std::unique_ptr<ImageData> subimage(int x, int y, int width, int height) const;
    void insert(const Bitmap& bitmap, int x, int y);
};
