#pragma once

#include <Gosu/Fwd.hpp>
#include <Gosu/Drawable.hpp>
#include <Gosu/Platform.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace Gosu
{
    /// When an image file is too large to be represented by a single OpenGL texture, Gosu automatically
    /// splits it up into a rectangle of tiles instead of throwing an error.
    class TiledDrawable : public Drawable
    {
        int m_width, m_height;
        struct Tile
        {
            int x = 0, y = 0;
            std::unique_ptr<Drawable> data = nullptr;
        };
        std::vector<Tile> m_tiles;

    public:
        TiledDrawable(const Bitmap& source, const Rect& source_rect, int tile_size,
                      unsigned image_flags);
        /// This constructor is used to implement subimage().
        TiledDrawable(const TiledDrawable& parent, const Rect& rect);

        int width() const override { return m_width; }
        int height() const override { return m_height; }

        void draw(double x1, double y1, Color c1, //
                  double x2, double y2, Color c2, //
                  double x3, double y3, Color c3, //
                  double x4, double y4, Color c4, //
                  ZPos z, BlendMode mode) const override;

        const GLTexInfo* gl_tex_info() const override { return nullptr; }

        std::unique_ptr<Drawable> subimage(const Rect& rect) const override;

        Bitmap to_bitmap() const override;

        void insert(const Bitmap& bitmap, int x, int y) override;
    };
}
