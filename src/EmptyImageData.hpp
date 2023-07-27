#pragma once

#include <Gosu/ImageData.hpp>
#include <memory>

namespace Gosu
{
    class EmptyImageData : public ImageData
    {
        int m_width, m_height;

    public:
        EmptyImageData(int width, int height)
            : m_width(width),
              m_height(height)
        {
        }

        int width() const override { return m_width; }

        int height() const override { return m_height; }

        void draw(double, double, Color, double, double, Color, //
                  double, double, Color, double, double, Color, //
                  ZPos, BlendMode) const override
        {
        }

        GLTexInfo* gl_tex_info() const override { return nullptr; }

        Bitmap to_bitmap() const override { return Bitmap(m_width, m_height); }

        std::unique_ptr<ImageData> subimage(const Rect& rect) const override
        {
            return std::make_unique<EmptyImageData>(rect.width, rect.height);
        }

        void insert(const Bitmap&, int, int) override { }
    };
}
