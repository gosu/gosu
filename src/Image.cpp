#include <Gosu/Bitmap.hpp>
#include <Gosu/Buffer.hpp>
#include <Gosu/Drawable.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Math.hpp>
#include "EmptyDrawable.hpp"
#include <stdexcept>

Gosu::Image::Image()
{
    static const auto default_data_ptr = std::make_shared<EmptyDrawable>(0, 0);
    m_drawable = default_data_ptr;
}

Gosu::Image::Image(const std::string& filename, unsigned image_flags)
: Image(load_image_file(filename), image_flags)
{
}

Gosu::Image::Image(const std::string& filename, const Rect& source_rect, unsigned image_flags)
: Image(load_image_file(filename), source_rect, image_flags)
{
}

Gosu::Image::Image(const Bitmap& source, unsigned image_flags)
: Image(source, Rect::covering(source), image_flags)
{
}

Gosu::Image::Image(const Bitmap& source, const Rect& source_rect, unsigned image_flags)
: m_drawable(Graphics::create_drawable(source, source_rect, image_flags))
{
}

Gosu::Image::Image(std::unique_ptr<Drawable> data)
: m_drawable(std::move(data))
{
    if (!m_drawable) {
        throw std::invalid_argument("Gosu::Image cannot be initialized with nullptr");
    }
}

unsigned Gosu::Image::width() const
{
    return m_drawable->width();
}

unsigned Gosu::Image::height() const
{
    return m_drawable->height();
}

void Gosu::Image::draw(double x, double y, ZPos z, double scale_x, double scale_y, Color c,
                       BlendMode mode) const
{
    double x2 = x + width() * scale_x;
    double y2 = y + height() * scale_y;

    m_drawable->draw(x, y, c, x2, y, c, x, y2, c, x2, y2, c, z, mode);
}

void Gosu::Image::draw_mod(double x, double y, ZPos z, double scale_x, double scale_y, Color c1,
                           Color c2, Color c3, Color c4, BlendMode mode) const
{
    double x2 = x + width() * scale_x;
    double y2 = y + height() * scale_y;

    m_drawable->draw(x, y, c1, x2, y, c2, x, y2, c3, x2, y2, c4, z, mode);
}

void Gosu::Image::draw_rot(double x, double y, ZPos z, double angle, double center_x,
                           double center_y, double scale_x, double scale_y, Color c,
                           BlendMode mode) const
{
    double size_x = width() * scale_x;
    double size_y = height() * scale_y;
    double offs_x = offset_x(angle, 1);
    double offs_y = offset_y(angle, 1);

    // Offset to the centers of the original Image's edges after rotation.
    double dist_to_left_x = +offs_y * size_x * center_x;
    double dist_to_left_y = -offs_x * size_x * center_x;
    double dist_to_right_x = -offs_y * size_x * (1 - center_x);
    double dist_to_right_y = +offs_x * size_x * (1 - center_x);
    double dist_to_top_x = +offs_x * size_y * center_y;
    double dist_to_top_y = +offs_y * size_y * center_y;
    double dist_to_bottom_x = -offs_x * size_y * (1 - center_y);
    double dist_to_bottom_y = -offs_y * size_y * (1 - center_y);

    m_drawable->draw(x + dist_to_left_x + dist_to_top_x, y + dist_to_left_y + dist_to_top_y, c,
                 x + dist_to_right_x + dist_to_top_x, y + dist_to_right_y + dist_to_top_y, c,
                 x + dist_to_left_x + dist_to_bottom_x, y + dist_to_left_y + dist_to_bottom_y, c,
                 x + dist_to_right_x + dist_to_bottom_x, y + dist_to_right_y + dist_to_bottom_y, c,
                 z, mode);
}

Gosu::Drawable& Gosu::Image::drawable() const
{
    return *m_drawable;
}

std::vector<Gosu::Image> Gosu::load_tiles(const Bitmap& bitmap, //
                                          int tile_width, int tile_height, unsigned flags)
{
    if (tile_width == 0 || tile_height == 0)
        throw std::invalid_argument("Gosu::load_tiles does not support empty tiles");

    int tiles_x, tiles_y;
    std::vector<Image> images;

    if (tile_width > 0) {
        tiles_x = bitmap.width() / tile_width;
    }
    else {
        tiles_x = -tile_width;
        tile_width = bitmap.width() / tiles_x;
    }

    if (tile_height > 0) {
        tiles_y = bitmap.height() / tile_height;
    }
    else {
        tiles_y = -tile_height;
        tile_height = bitmap.height() / tiles_y;
    }

    for (int y = 0; y < tiles_y; ++y) {
        for (int x = 0; x < tiles_x; ++x) {
            images.emplace_back(
                bitmap, Rect { x * tile_width, y * tile_height, tile_width, tile_height }, flags);
        }
    }

    return images;
}

std::vector<Gosu::Image> Gosu::load_tiles(const std::string& filename, //
                                          int tile_width, int tile_height, unsigned flags)
{
    const Bitmap bitmap = load_image_file(filename);
    return load_tiles(bitmap, tile_width, tile_height, flags);
}
