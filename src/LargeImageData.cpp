#include "LargeImageData.hpp"
#include "GraphicsImpl.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Math.hpp>
#include <cmath>
using namespace std;

Gosu::LargeImageData::LargeImageData(const Bitmap& source,
    unsigned part_width, unsigned part_height, unsigned image_flags)
{
    full_width = source.width();
    full_height = source.height();
    parts_x = static_cast<unsigned>(trunc(ceil(1.0 * source.width() / part_width)));
    parts_y = static_cast<unsigned>(trunc(ceil(1.0 * source.height() / part_height)));
    this->part_width = part_width;
    this->part_height = part_height;

    parts.resize(parts_x * parts_y);

    for (unsigned y = 0; y < parts_y; ++y) {
        for (unsigned x = 0; x < parts_x; ++x) {
            // The right-most parts don't necessarily have the full width.
            unsigned src_width = part_width;
            if (x == parts_x - 1 && source.width() % part_width != 0) {
                src_width = source.width() % part_width;
            }

            // Same for the parts on the bottom.
            unsigned src_height = part_height;
            if (y == parts_y - 1 && source.height() % part_height != 0) {
                src_height = source.height() % part_height;
            }

            unsigned local_flags = IF_TILEABLE | image_flags;
            
            // Left edge, only tileable if requested in image_flags.
            if (x == 0) {
                local_flags &= ~IF_TILEABLE_LEFT;
                local_flags |= (image_flags & IF_TILEABLE_LEFT);
            }
            // Right edge, only tileable if requested in image_flags.
            if (x == parts_x - 1) {
                local_flags &= ~IF_TILEABLE_RIGHT;
                local_flags |= (image_flags & IF_TILEABLE_RIGHT);
            }
            // Top edge, only tileable if requested in image_flags.
            if (y == 0) {
                local_flags &= ~IF_TILEABLE_TOP;
                local_flags |= (image_flags & IF_TILEABLE_TOP);
            }
            // Bottom edge, only tileable if requested in image_flags.
            if (y == parts_y - 1) {
                local_flags &= ~IF_TILEABLE_BOTTOM;
                local_flags |= (image_flags & IF_TILEABLE_BOTTOM);
            }
            
            parts[y * parts_x + x].reset(Graphics::create_image(source,
                                                                x * part_width, y * part_height,
                                                                src_width, src_height,
                                                                local_flags).release());
        }
    }
}

int Gosu::LargeImageData::width() const
{
    return full_width;
}

int Gosu::LargeImageData::height() const
{
    return full_height;
}

namespace
{
    // Local interpolation helper functions. - TODO why not from Math.hpp?

    double ipl(double a, double b, double ratio)
    {
        return a + (b - a) * ratio;
    }

    Gosu::Color ipl(Gosu::Color a, Gosu::Color b, double ratio)
    {
        Gosu::Color result;
        result.set_alpha(Gosu::round(ipl(a.alpha(), b.alpha(), ratio)));
        result.set_red  (Gosu::round(ipl(a.red(),   b.red(),   ratio)));
        result.set_green(Gosu::round(ipl(a.green(), b.green(), ratio)));
        result.set_blue (Gosu::round(ipl(a.blue(),  b.blue(),  ratio)));
        return result;
    }
}

void Gosu::LargeImageData::draw(double x1, double y1, Color c1, double x2, double y2, Color c2,
    double x3, double y3, Color c3, double x4, double y4, Color c4, ZPos z, AlphaMode mode) const
{
    if (parts.empty()) return;

    normalize_coordinates(x1, y1, x2, y2, x3, y3, c3, x4, y4, c4);
    
    for (unsigned py = 0; py < parts_y; ++py) {
        for (unsigned px = 0; px < parts_x; ++px) {
            ImageData& part = *parts[py * parts_x + px];

            double rel_x_l = static_cast<double>(px * part_width) / width();
            double rel_x_r = static_cast<double>(px * part_width + part.width()) / width();
            double rel_y_t = static_cast<double>(py * part_height) / height();
            double rel_y_b = static_cast<double>(py * part_height + part.height()) / height();

            double abs_x_t_l = ipl(ipl(x1, x3, rel_y_t), ipl(x2, x4, rel_y_t), rel_x_l);
            double abs_x_t_r = ipl(ipl(x1, x3, rel_y_t), ipl(x2, x4, rel_y_t), rel_x_r);
            double abs_x_b_l = ipl(ipl(x1, x3, rel_y_b), ipl(x2, x4, rel_y_b), rel_x_l);
            double abs_x_b_r = ipl(ipl(x1, x3, rel_y_b), ipl(x2, x4, rel_y_b), rel_x_r);

            double abs_y_t_l = ipl(ipl(y1, y3, rel_y_t), ipl(y2, y4, rel_y_t), rel_x_l);
            double abs_y_t_r = ipl(ipl(y1, y3, rel_y_t), ipl(y2, y4, rel_y_t), rel_x_r);
            double abs_y_b_l = ipl(ipl(y1, y3, rel_y_b), ipl(y2, y4, rel_y_b), rel_x_l);
            double abs_y_b_r = ipl(ipl(y1, y3, rel_y_b), ipl(y2, y4, rel_y_b), rel_x_r);

            Color abs_c_t_l = ipl(ipl(c1, c3, rel_y_t), ipl(c2, c4, rel_y_t), rel_x_l);
            Color abs_c_t_r = ipl(ipl(c1, c3, rel_y_t), ipl(c2, c4, rel_y_t), rel_x_r);
            Color abs_c_b_l = ipl(ipl(c1, c3, rel_y_b), ipl(c2, c4, rel_y_b), rel_x_l);
            Color abs_c_b_r = ipl(ipl(c1, c3, rel_y_b), ipl(c2, c4, rel_y_b), rel_x_r);

            part.draw(abs_x_t_l, abs_y_t_l, abs_c_t_l, abs_x_t_r, abs_y_t_r, abs_c_t_r,
                      abs_x_b_l, abs_y_b_l, abs_c_b_l, abs_x_b_r, abs_y_b_r, abs_c_b_r, z, mode);
        }
    }
}

Gosu::Bitmap Gosu::LargeImageData::to_bitmap() const
{
    Bitmap bitmap(width(), height());
    for (int x = 0; x < parts_x; ++x) {
        for (int y = 0; y < parts_y; ++y) {
            bitmap.insert(parts[y * parts_x + x]->to_bitmap(), x * part_width, y * part_height);
        }
    }
    return bitmap;
}

void Gosu::LargeImageData::insert(const Bitmap& bitmap, int at_x, int at_y)
{
    for (int x = 0; x < parts_x; ++x) {
        for (int y = 0; y < parts_y; ++y) {
            parts[y * parts_x + x]->insert(bitmap, at_x - x * part_width, at_y - y * part_height);
        }
    }
}
