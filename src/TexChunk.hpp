#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/Utility.hpp>
#include "GraphicsImpl.hpp"
#include <memory>
#include <stdexcept>

class Gosu::TexChunk : public Gosu::ImageData
{
    const std::shared_ptr<Texture> m_texture;
    const Rect m_rect;
    const GLTexInfo m_info;

public:
    TexChunk(const std::shared_ptr<Texture>& texture, const Rect& rect);
    ~TexChunk() override;

    int width() const override { return m_rect.width; }
    int height() const override { return m_rect.height; }

    unsigned tex_name() const { return m_info.tex_name; }

    void draw(double x1, double y1, Color c1, //
              double x2, double y2, Color c2, //
              double x3, double y3, Color c3, //
              double x4, double y4, Color c4, //
              ZPos z, BlendMode mode) const override;

    const GLTexInfo* gl_tex_info() const override { return &m_info; }

    std::unique_ptr<ImageData> subimage(const Rect& rect) const override;

    Gosu::Bitmap to_bitmap() const override;

    void insert(const Bitmap& bitmap, int x, int y) override;
};
