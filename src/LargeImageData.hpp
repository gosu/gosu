#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/ImageData.hpp>
#include <Gosu/Platform.hpp>
#include "GraphicsImpl.hpp"
#include <memory>
#include <vector>

class Gosu::LargeImageData : public Gosu::ImageData
{
    int m_w, m_h;
    int m_tiles_x, m_tiles_y;
    std::vector<std::unique_ptr<ImageData>> m_tiles;

    LargeImageData() = default;

    const ImageData& tile(int x, int y) const;

public:
    LargeImageData(const Bitmap& source, int tile_width, int tile_height, unsigned image_flags);

    int width() const override { return m_w; }
    int height() const override { return m_h; }

    void draw(double x1, double y1, Color c1, //
              double x2, double y2, Color c2, //
              double x3, double y3, Color c3, //
              double x4, double y4, Color c4, //
              ZPos z, BlendMode mode) const override;

    const GLTexInfo* gl_tex_info() const override { return nullptr; }

    std::unique_ptr<ImageData> subimage(int x, int y, int width, int height) const override;

    Bitmap to_bitmap() const override;

    void insert(const Bitmap& bitmap, int x, int y) override;
};
