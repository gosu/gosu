#pragma once

#include <Gosu/Bitmap.hpp>
#include <Gosu/Drawable.hpp>
#include <memory>

namespace Gosu
{
    class EmptyDrawable : public Drawable
    {
        int m_width, m_height;

    public:
        EmptyDrawable(int width, int height)
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

        std::unique_ptr<Drawable> subimage(const Rect&) const override { return nullptr; }

        void insert(const Bitmap&, int, int) override { }
    };
}
