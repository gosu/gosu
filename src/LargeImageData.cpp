#include "LargeImageData.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Math.hpp>
#include <cmath>
#include <stdexcept>
using namespace std;

Gosu::LargeImageData::LargeImageData(const Bitmap& source, int tile_width, int tile_height,
                                     unsigned image_flags)
{
    w = source.width();
    h = source.height();
    tiles_x = static_cast<int>(ceil(1.0 * w / tile_width));
    tiles_y = static_cast<int>(ceil(1.0 * h / tile_height));

    // When there are no tiles, set both fields to 0 to avoid entering any for () loop in this class.
    if (tiles_x == 0 || tiles_y == 0) {
        tiles_x = tiles_y = 0;
    }

    tiles.reserve(tiles_x * tiles_y);

    for (int y = 0; y < tiles_y; ++y) {
        for (int x = 0; x < tiles_x; ++x) {
            int src_width = tile_width;
            if (x == tiles_x - 1 && w % tile_width != 0) {
                // The right-most parts don't necessarily have the full width.
                src_width = w % tile_width;
            }

            int src_height = tile_height;
            if (y == tiles_y - 1 && h % tile_height != 0) {
                // Same for the parts on the bottom.
                src_height = h % tile_height;
            }

            unsigned local_flags = IF_TILEABLE | image_flags;
            
            // Left edge, only tileable if requested in image_flags.
            if (x == 0) {
                local_flags &= ~IF_TILEABLE_LEFT;
                local_flags |= (image_flags & IF_TILEABLE_LEFT);
            }
            // Right edge, only tileable if requested in image_flags.
            if (x == tiles_x - 1) {
                local_flags &= ~IF_TILEABLE_RIGHT;
                local_flags |= (image_flags & IF_TILEABLE_RIGHT);
            }
            // Top edge, only tileable if requested in image_flags.
            if (y == 0) {
                local_flags &= ~IF_TILEABLE_TOP;
                local_flags |= (image_flags & IF_TILEABLE_TOP);
            }
            // Bottom edge, only tileable if requested in image_flags.
            if (y == tiles_y - 1) {
                local_flags &= ~IF_TILEABLE_BOTTOM;
                local_flags |= (image_flags & IF_TILEABLE_BOTTOM);
            }
            
            tiles.emplace_back(Graphics::create_image(source,
                                                      x * tile_width, y * tile_height,
                                                      src_width, src_height, local_flags));
        }
    }
}

void Gosu::LargeImageData::draw(double x1, double y1, Color c1,
                                double x2, double y2, Color c2,
                                double x3, double y3, Color c3,
                                double x4, double y4, Color c4,
                                ZPos z, BlendMode mode) const
{
    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);
    
    double y = 0;
    for (int ty = 0; ty < tiles_y; ++ty) {
        double x = 0;
        for (int tx = 0; tx < tiles_x; ++tx) {
            ImageData& tile = *tiles[ty * tiles_x + tx];

            double rel_x_l = x / w;
            double rel_x_r = (x + tile.width()) / w;
            double rel_y_t = y / h;
            double rel_y_b = (y + tile.height()) / h;
            
        #define INTERPOLATE(what, x_weight, y_weight) \
            interpolate(interpolate(what##1, what##3, y_weight), \
                        interpolate(what##2, what##4, y_weight), \
                        x_weight);

            double x_t_l = INTERPOLATE(x, rel_x_l, rel_y_t);
            double x_t_r = INTERPOLATE(x, rel_x_r, rel_y_t);
            double x_b_l = INTERPOLATE(x, rel_x_l, rel_y_b);
            double x_b_r = INTERPOLATE(x, rel_x_r, rel_y_b);

            double y_t_l = INTERPOLATE(y, rel_x_l, rel_y_t);
            double y_t_r = INTERPOLATE(y, rel_x_r, rel_y_t);
            double y_b_l = INTERPOLATE(y, rel_x_l, rel_y_b);
            double y_b_r = INTERPOLATE(y, rel_x_r, rel_y_b);

            Color  c_t_l = INTERPOLATE(c, rel_x_l, rel_y_t);
            Color  c_t_r = INTERPOLATE(c, rel_x_r, rel_y_t);
            Color  c_b_l = INTERPOLATE(c, rel_x_l, rel_y_b);
            Color  c_b_r = INTERPOLATE(c, rel_x_r, rel_y_b);

            tile.draw(x_t_l, y_t_l, c_t_l,
                      x_t_r, y_t_r, c_t_r,
                      x_b_l, y_b_l, c_b_l,
                      x_b_r, y_b_r, c_b_r,
                      z, mode);
            
            x += tile.width();
        }
        y += tiles[ty * tiles_x]->height();
    }
}

unique_ptr<Gosu::ImageData>
    Gosu::LargeImageData::subimage(int left, int top, int width, int height) const
{
    if (left < 0 || top < 0 || left + width > w || top + height > h) {
        throw invalid_argument("subimage bounds exceed those of its parent");
    }
    if (width <= 0 || height <= 0) {
        throw invalid_argument("cannot create empty image");
    }
    
    int sub_tiles_y = 0;
    vector<unique_ptr<ImageData>> sub_tiles;
    
    int y = 0;
    for (int ty = 0; ty < tiles_y; ++ty) {
        int row_height = tiles[ty * tiles_x]->height();
        
        if (y + row_height <= top) {
            y += tiles[ty * tiles_x]->height();
            continue;
        }
        if (y >= top + height) break;
        
        sub_tiles_y += 1;

        int x = 0;
        for (int tx = 0; tx < tiles_x; ++tx) {
            ImageData& tile = *tiles[ty * tiles_x + tx];
            
            if (x + tile.width() <= left) {
                x += tile.width();
                continue;
            }
            if (x >= left + width) break;
            
            int sub_left   = max(0, left - x);
            int sub_top    = max(0, top  - y);
            int sub_right  = min(tile.width(),  left + width  - x);
            int sub_bottom = min(tile.height(), top  + height - y);
            
            sub_tiles.emplace_back(tile.subimage(sub_left, sub_top, sub_right - sub_left, sub_bottom - sub_top));
            
            x += tile.width();
        }
        y += tiles[ty * tiles_x]->height();
    }
    
    if (sub_tiles.size() == 1) {
        return move(sub_tiles[0]);
    }
    else {
        unique_ptr<LargeImageData> result(new LargeImageData());
        result->w = width;
        result->h = height;
        result->tiles_x = static_cast<int>(sub_tiles.size()) / sub_tiles_y;
        result->tiles_y = sub_tiles_y;
        result->tiles.swap(sub_tiles);
        return move(result);
    }
}

Gosu::Bitmap Gosu::LargeImageData::to_bitmap() const
{
    Bitmap bitmap(width(), height());
    int y = 0;
    for (int ty = 0; ty < tiles_y; ++ty) {
        int x = 0;
        for (int tx = 0; tx < tiles_x; ++tx) {
            ImageData& tile = *tiles[ty * tiles_x + tx];
            bitmap.insert(x, y, tile.to_bitmap());
            x += tile.width();
        }
        y += tiles[ty * tiles_x]->height();
    }
    return bitmap;
}

void Gosu::LargeImageData::insert(const Bitmap& bitmap, int at_x, int at_y)
{
    int y = 0;
    for (int ty = 0; ty < tiles_y; ++ty) {
        int x = 0;
        for (int tx = 0; tx < tiles_x; ++tx) {
            ImageData& tile = *tiles[ty * tiles_x + tx];
            tile.insert(bitmap, at_x - x, at_y - y);
            x += tile.width();
        }
        y += tiles[ty * tiles_x]->height();
    }
}
