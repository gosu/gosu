#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include "GraphicsImpl.hpp"
#include <memory>
#include <stdexcept>

class Gosu::TexChunk : public Gosu::ImageData
{
    std::shared_ptr<Texture> m_texture;
    int m_x, m_y, m_w, m_h, m_padding;

    GLTexInfo m_info;

    void set_tex_info();

public:
    TexChunk(std::shared_ptr<Texture> texture, int x, int y, int w, int h, int padding);
    TexChunk(const TexChunk& parent, int x, int y, int w, int h);
    ~TexChunk() override;

    int width() const override { return m_w; }
    int height() const override { return m_h; }

    GLuint tex_name() const { return m_info.tex_name; }

    void draw(double x1, double y1, Color c1, //
              double x2, double y2, Color c2, //
              double x3, double y3, Color c3, //
              double x4, double y4, Color c4, //
              ZPos z, BlendMode mode) const override;

    const GLTexInfo* gl_tex_info() const override { return &m_info; }

    std::unique_ptr<ImageData> subimage(int x, int y, int width, int height) const override;

    Gosu::Bitmap to_bitmap() const override;

    void insert(const Bitmap& bitmap, int x, int y) override;
};
