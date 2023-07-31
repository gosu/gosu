#include "TiledImageData.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Math.hpp>
#include "GraphicsImpl.hpp"
#include <cmath>
#include <stdexcept>

Gosu::TiledImageData::TiledImageData(const Bitmap& source, int tile_size, unsigned image_flags)
    : m_width(source.width()),
      m_height(source.height())
{
    if (tile_size <= 0) {
        throw std::invalid_argument("tile_size must be greater than 0");
    }

    // Manually round up during this integer division.
    const int tiles_x = (source.width() + tile_size - 1) / tile_size;
    const int tiles_y = (source.height() + tile_size - 1) / tile_size;

    for (int ty = 0; ty < tiles_y; ++ty) {
        for (int tx = 0; tx < tiles_x; ++tx) {
            Rect source_rect { tx * tile_size, ty * tile_size, tile_size, tile_size };
            source_rect.clip_to(Rect::covering(source));

            unsigned local_flags = image_flags;

            // The left edge must always be tileable in all columns except for the first.
            if (tx > 0) {
                local_flags |= IF_TILEABLE_LEFT;
            }
            // The right edge must always be tileable in all columns except for the last.
            if (tx < tiles_x - 1) {
                local_flags |= IF_TILEABLE_RIGHT;
            }
            // The top edge must always be tileable in all rows except for the first.
            if (ty > 0) {
                local_flags |= IF_TILEABLE_TOP;
            }
            // The bottom edge must always be tileable in all rows except for the last.
            if (ty < tiles_y - 1) {
                local_flags |= IF_TILEABLE_BOTTOM;
            }

            m_tiles.push_back(Tile {
                .x = source_rect.x,
                .y = source_rect.y,
                .data = Graphics::create_drawable(source, source_rect, local_flags),
            });
        }
    }
}

Gosu::TiledImageData::TiledImageData(const TiledImageData& parent, const Rect& rect)
    : m_width(rect.width),
      m_height(rect.height)
{
    for (const Tile& tile : parent.m_tiles) {
        // rect is the source area on the parent, tile_rect is this tile's area on the parent.
        Rect tile_rect { tile.x, tile.y, tile.data->width(), tile.data->height() };
        // The x/y source position on the tile...
        int x = 0, y = 0;
        // ...must be adjusted while clipping.
        tile_rect.clip_to(rect, &x, &y);
        // If there is anything left of the tile_rect after clipping, we need to create a tile too.
        if (!tile_rect.empty()) {
            m_tiles.push_back(Tile {
                .x = tile.x - rect.x,
                .y = tile.y - rect.y,
                .data = tile.data->subimage(Rect { x, y, tile_rect.width, tile_rect.height }),
            });
        }
    }
}

void Gosu::TiledImageData::draw(double x1, double y1, Color c1, double x2, double y2, Color c2,
                                double x3, double y3, Color c3, double x4, double y4, Color c4,
                                ZPos z, BlendMode mode) const
{
    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);

    double f_width = m_width, f_height = m_height;
    for (const Tile& tile : m_tiles) {
        double rel_x_l = tile.x / f_width;
        double rel_x_r = (tile.x + tile.data->width()) / f_width;
        double rel_y_t = tile.y / f_height;
        double rel_y_b = (tile.y + tile.data->height()) / f_height;

        using std::lerp;
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LERP2D(what, x_weight, y_weight)                                                           \
    lerp(lerp(what##1, what##3, y_weight), lerp(what##2, what##4, y_weight), x_weight);

        double x_t_l = LERP2D(x, rel_x_l, rel_y_t);
        double x_t_r = LERP2D(x, rel_x_r, rel_y_t);
        double x_b_l = LERP2D(x, rel_x_l, rel_y_b);
        double x_b_r = LERP2D(x, rel_x_r, rel_y_b);

        double y_t_l = LERP2D(y, rel_x_l, rel_y_t);
        double y_t_r = LERP2D(y, rel_x_r, rel_y_t);
        double y_b_l = LERP2D(y, rel_x_l, rel_y_b);
        double y_b_r = LERP2D(y, rel_x_r, rel_y_b);

        Color c_t_l = LERP2D(c, rel_x_l, rel_y_t);
        Color c_t_r = LERP2D(c, rel_x_r, rel_y_t);
        Color c_b_l = LERP2D(c, rel_x_l, rel_y_b);
        Color c_b_r = LERP2D(c, rel_x_r, rel_y_b);

        tile.data->draw(x_t_l, y_t_l, c_t_l, x_t_r, y_t_r, c_t_r, //
                        x_b_l, y_b_l, c_b_l, x_b_r, y_b_r, c_b_r, z, mode);
    }
}

std::unique_ptr<Gosu::Drawable> Gosu::TiledImageData::subimage(const Rect& source_rect) const
{
    auto tiled_data = std::make_unique<TiledImageData>(*this, source_rect);
    if (tiled_data->m_tiles.size() == 1) {
        // Optimization: If the tiled subimage only contains a single tile, return that.
        return std::move(tiled_data->m_tiles[0].data);
    }
    return tiled_data;
}

Gosu::Bitmap Gosu::TiledImageData::to_bitmap() const
{
    Bitmap bitmap(width(), height());
    for (const Tile& tile : m_tiles) {
        bitmap.insert(tile.data->to_bitmap(), tile.x, tile.y);
    }
    return bitmap;
}

void Gosu::TiledImageData::insert(const Bitmap& bitmap, int x, int y)
{
    const Rect target_rect { x, y, bitmap.width(), bitmap.height() };
    for (const Tile& tile : m_tiles) {
        Rect tile_rect { tile.x, tile.y, tile.data->width(), tile.data->height() };
        tile_rect.clip_to(target_rect);
        if (!tile_rect.empty()) {
            tile.data->insert(bitmap, x - tile.x, y - tile.y);
        }
    }
}
