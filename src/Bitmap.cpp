#include <Gosu/Bitmap.hpp>
#include <cassert>
#include <algorithm>
#include <vector>

void Gosu::Bitmap::swap(Bitmap& other)
{
    std::swap(pixels, other.pixels);
    std::swap(w, other.w);
    std::swap(h, other.h);
}

void Gosu::Bitmap::resize(unsigned width, unsigned height, Color c)
{
    if (width == w && height == h)
        return;

    Bitmap temp(width, height, c);
    temp.insert(*this, 0, 0);
    swap(temp);
}

void Gosu::Bitmap::insert(const Bitmap& source, int x, int y)
{
    insert(source, x, y, 0, 0, source.width(), source.height());
}

void Gosu::Bitmap::insert(const Bitmap& source, int x, int y, unsigned src_x, unsigned src_y,
    unsigned src_width, unsigned src_height)
{
    // TODO: This should use memcpy if possible (x == 0 && src_width == this->width())

    if (x < 0) {
        unsigned clip_left = -x;

        if (clip_left >= src_width)
            return;

        src_x += clip_left;
        src_width -= clip_left;
        x = 0;
    }

    if (y < 0) {
        unsigned clip_top = -y;

        if (clip_top >= src_height)
            return;

        src_y += clip_top;
        src_height -= clip_top;
        y = 0;
    }

    if (x + src_width > w) {
        if (static_cast<unsigned>(x) >= w)
            return;

        src_width = w - x;
    }

    if (y + src_height > h) {
        if (static_cast<unsigned>(y) >= h)
            return;

        src_height = h - y;
    }

    for (unsigned rel_y = 0; rel_y < src_height; ++rel_y)
        for (unsigned rel_x = 0; rel_x < src_width; ++rel_x)
            set_pixel(x + rel_x, y + rel_y, source.get_pixel(src_x + rel_x, src_y + rel_y));
}

void Gosu::apply_color_key(Bitmap& bitmap, Color key)
{
    std::vector<Color> surrounding_colors;
    surrounding_colors.reserve(4);

    for (unsigned y = 0; y < bitmap.height(); ++y)
        for (unsigned x = 0; x < bitmap.width(); ++x)
            if (bitmap.get_pixel(x, y) == key) {
                surrounding_colors.clear();
                if (x > 0 && bitmap.get_pixel(x - 1, y) != key)
                    surrounding_colors.push_back(bitmap.get_pixel(x - 1, y));
                if (x < bitmap.width() - 1 && bitmap.get_pixel(x + 1, y) != key)
                    surrounding_colors.push_back(bitmap.get_pixel(x + 1, y));
                if (y > 0 && bitmap.get_pixel(x, y - 1) != key)
                    surrounding_colors.push_back(bitmap.get_pixel(x, y - 1));
                if (y < bitmap.height() - 1 && bitmap.get_pixel(x, y + 1) != key)
                    surrounding_colors.push_back(bitmap.get_pixel(x, y + 1));

                if (surrounding_colors.empty()) {
                    bitmap.set_pixel(x, y, Color::NONE);
                    continue;
                }

                unsigned red = 0, green = 0, blue = 0;
                for (auto& color : surrounding_colors) {
                    red += color.red();
                    green += color.green();
                    blue += color.blue();
                }
                bitmap.set_pixel(x, y, Color(0, red / surrounding_colors.size(),
                    green / surrounding_colors.size(), blue / surrounding_colors.size()));
            }
}

void Gosu::unapply_color_key(Bitmap& bitmap, Color color)
{
    Color* p = bitmap.data();
    for (int i = bitmap.width() * bitmap.height(); i > 0; --i, ++p)
        if (p->alpha() == 0)
            *p = color;
        else
            p->set_alpha(255);
}

void Gosu::apply_border_flags(Bitmap& dest, const Bitmap& source, unsigned src_x, unsigned src_y,
    unsigned src_width, unsigned src_height, unsigned image_flags)
{
    // Backward compatibility: This used to be 'bool tileable'.
    if (image_flags == 1)
        image_flags = IF_TILEABLE;

    dest.resize(src_width + 2, src_height + 2);

    // The borders are made "harder" by duplicating the original bitmap's
    // borders.

    // Top.
    if (image_flags & IF_TILEABLE_TOP)
        dest.insert(source, 1, 0, src_x, src_y, src_width, 1);
    // Bottom.
    if (image_flags & IF_TILEABLE_BOTTOM)
        dest.insert(source, 1, dest.height() - 1, src_x, src_y + src_height - 1, src_width, 1);
    // Left.
    if (image_flags & IF_TILEABLE_LEFT)
        dest.insert(source, 0, 1, src_x, src_y, 1, src_height);
    // Right.
    if (image_flags & IF_TILEABLE_RIGHT)
        dest.insert(source, dest.width() - 1, 1, src_x + src_width - 1, src_y, 1, src_height);

    // Top left.
    if ((image_flags & IF_TILEABLE_TOP) && (image_flags & IF_TILEABLE_LEFT))
        dest.set_pixel(0, 0, source.get_pixel(src_x, src_y));
    // Top right.
    if ((image_flags & IF_TILEABLE_TOP) && (image_flags & IF_TILEABLE_RIGHT))
        dest.set_pixel(dest.width() - 1, 0, source.get_pixel(src_x + src_width - 1, src_y));
    // Bottom left.
    if ((image_flags & IF_TILEABLE_BOTTOM) && (image_flags & IF_TILEABLE_LEFT))
        dest.set_pixel(0, dest.height() - 1, source.get_pixel(src_x, src_y + src_height - 1));
    // Bottom right.
    if ((image_flags & IF_TILEABLE_BOTTOM) && (image_flags & IF_TILEABLE_RIGHT))
        dest.set_pixel(dest.width() - 1, dest.height() - 1,
            source.get_pixel(src_x + src_width - 1, src_y + src_height - 1));

    // Now put the final image into the prepared borders.
    dest.insert(source, 1, 1, src_x, src_y, src_width, src_height);
}
