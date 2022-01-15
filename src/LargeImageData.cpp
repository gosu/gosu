#include "LargeImageData.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Math.hpp>
#include <cmath>
#include <stdexcept>

const Gosu::ImageData& Gosu::LargeImageData::tile(int x, int y) const
{
    return *m_tiles.at(static_cast<std::size_t>(y * m_tiles_x + x));
}

Gosu::LargeImageData::LargeImageData(const Bitmap& source, int tile_width, int tile_height,
                                     unsigned image_flags)
{
    m_w = source.width();
    m_h = source.height();
    // Manual ceil() for integer division.
    m_tiles_x = (m_w + tile_width - 1) / tile_width;
    m_tiles_y = (m_h + tile_height - 1) / tile_height;

    // When there are no tiles, set both fields to 0 to avoid entering any for () loop in this class.
    if (m_tiles_x == 0 || m_tiles_y == 0) {
        m_tiles_x = m_tiles_y = 0;
    }

    m_tiles.reserve(m_tiles_x * m_tiles_y);

    for (int y = 0; y < m_tiles_y; ++y) {
        for (int x = 0; x < m_tiles_x; ++x) {
            int src_width = tile_width;
            if (x == m_tiles_x - 1 && m_w % tile_width != 0) {
                // The right-most parts don't necessarily have the full width.
                src_width = m_w % tile_width;
            }

            int src_height = tile_height;
            if (y == m_tiles_y - 1 && m_h % tile_height != 0) {
                // Same for the parts on the bottom.
                src_height = m_h % tile_height;
            }

            unsigned local_flags = IF_TILEABLE | image_flags;

            // Left edge, only tileable if requested in image_flags.
            if (x == 0) {
                local_flags &= ~IF_TILEABLE_LEFT;
                local_flags |= (image_flags & IF_TILEABLE_LEFT);
            }
            // Right edge, only tileable if requested in image_flags.
            if (x == m_tiles_x - 1) {
                local_flags &= ~IF_TILEABLE_RIGHT;
                local_flags |= (image_flags & IF_TILEABLE_RIGHT);
            }
            // Top edge, only tileable if requested in image_flags.
            if (y == 0) {
                local_flags &= ~IF_TILEABLE_TOP;
                local_flags |= (image_flags & IF_TILEABLE_TOP);
            }
            // Bottom edge, only tileable if requested in image_flags.
            if (y == m_tiles_y - 1) {
                local_flags &= ~IF_TILEABLE_BOTTOM;
                local_flags |= (image_flags & IF_TILEABLE_BOTTOM);
            }

            m_tiles.emplace_back(Graphics::create_image(source, x * tile_width, y * tile_height,
                                                        src_width, src_height, local_flags));
        }
    }
}

void Gosu::LargeImageData::draw(double x1, double y1, Color c1, double x2, double y2, Color c2,
                                double x3, double y3, Color c3, double x4, double y4, Color c4,
                                ZPos z, BlendMode mode) const
{
    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);

    double y = 0;
    for (int ty = 0; ty < m_tiles_y; ++ty) {
        double x = 0;
        for (int tx = 0; tx < m_tiles_x; ++tx) {
            const ImageData& image = tile(tx, ty);

            double rel_x_l = x / m_w;
            double rel_x_r = (x + image.width()) / m_w;
            double rel_y_t = y / m_h;
            double rel_y_b = (y + image.height()) / m_h;

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

            image.draw(x_t_l, y_t_l, c_t_l, x_t_r, y_t_r, c_t_r, x_b_l, y_b_l, c_b_l, x_b_r, y_b_r,
                       c_b_r, z, mode);

            x += image.width();
        }
        y += tile(0, ty).height();
    }
}

std::unique_ptr<Gosu::ImageData> Gosu::LargeImageData::subimage(int left, int top, int width,
                                                                int height) const
{
    if (left < 0 || top < 0 || left + width > m_w || top + height > m_h) {
        throw std::invalid_argument{"subimage bounds exceed those of its parent"};
    }
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument{"cannot create empty subimage"};
    }

    int sub_tiles_y = 0;
    std::vector<std::unique_ptr<ImageData>> sub_tiles;

    int y = 0;
    for (int ty = 0; ty < m_tiles_y; ++ty) {
        int row_height = tile(0, ty).height();

        if (y + row_height <= top) {
            // Skip rows until we are at the requested Y coordinate.
            y += row_height;
            continue;
        }
        if (y >= top + height) {
            // Also skip all rows after reaching the bottom requested Y coordinate.
            break;
        }

        sub_tiles_y += 1;

        int x = 0;
        for (int tx = 0; tx < m_tiles_x; ++tx) {
            const ImageData& image = tile(tx, ty);

            if (x + image.width() <= left) {
                // Skip columns until we are at the requested X coordinate.
                x += image.width();
                continue;
            }
            if (x >= left + width) {
                // Also skip all columns after reaching the right-most requested Y coordinate.
                break;
            }

            int sub_left = std::max(0, left - x);
            int sub_top = std::max(0, top - y);
            int sub_right = std::min(image.width(), left + width - x);
            int sub_bottom = std::min(image.height(), top + height - y);

            sub_tiles.emplace_back(
                    image.subimage(sub_left, sub_top, sub_right - sub_left, sub_bottom - sub_top));

            x += image.width();
        }
        y += tile(0, ty).height();
    }

    if (sub_tiles.size() == 1) {
        return move(sub_tiles[0]);
    }
    else {
        std::unique_ptr<LargeImageData> result(new LargeImageData());
        result->m_w = width;
        result->m_h = height;
        result->m_tiles_x = static_cast<int>(sub_tiles.size()) / sub_tiles_y;
        result->m_tiles_y = sub_tiles_y;
        result->m_tiles.swap(sub_tiles);
        return move(result);
    }
}

Gosu::Bitmap Gosu::LargeImageData::to_bitmap() const
{
    Bitmap bitmap(width(), height());
    int y = 0;
    for (int ty = 0; ty < m_tiles_y; ++ty) {
        int x = 0;
        for (int tx = 0; tx < m_tiles_x; ++tx) {
            const ImageData& image = tile(tx, ty);
            bitmap.insert(x, y, image.to_bitmap());
            x += image.width();
        }
        y += tile(0, ty).height();
    }
    return bitmap;
}

void Gosu::LargeImageData::insert(const Bitmap& bitmap, int at_x, int at_y)
{
    int y = 0;
    for (int ty = 0; ty < m_tiles_y; ++ty) {
        int x = 0;
        for (int tx = 0; tx < m_tiles_x; ++tx) {
            ImageData& image = *m_tiles[static_cast<std::size_t>(ty * m_tiles_x + tx)];
            image.insert(bitmap, at_x - x, at_y - y);
            x += image.width();
        }
        y += tile(0, ty).height();
    }
}
