#include <Gosu/Bitmap.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <stdexcept> // for std::invalid_argument
#include <utility>   // for std::swap

Gosu::Bitmap::Bitmap(int width, int height, Gosu::Color c)
{
    resize(width, height, c);
}

void Gosu::Bitmap::swap(Bitmap& other)
{
    std::swap(m_pixels, other.m_pixels);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
}

void Gosu::Bitmap::resize(int width, int height, Color c)
{
    if (width < 0 || height < 0) throw std::invalid_argument{"negative bitmap size"};

    if (width == m_width && height == m_height) return;

    Bitmap temp;
    temp.m_width = width;
    temp.m_height = height;
    temp.m_pixels.resize(width * height, c);
    temp.insert(0, 0, *this);
    swap(temp);
}

void Gosu::Bitmap::blend_pixel(int x, int y, Color c)
{
    if (c.alpha() == 0) return;

    Color out = get_pixel(x, y);
    if (out.alpha() == 0) {
        set_pixel(x, y, c);
        return;
    }

    int inv_alpha = out.alpha() * (255 - c.alpha()) / 255;

    out.set_alpha(c.alpha() + inv_alpha);
    out.set_red  ((c.red()   * c.alpha() + out.red()   * inv_alpha) / out.alpha());
    out.set_green((c.green() * c.alpha() + out.green() * inv_alpha) / out.alpha());
    out.set_blue ((c.blue()  * c.alpha() + out.blue()  * inv_alpha) / out.alpha());

    set_pixel(x, y, out);
}

void Gosu::Bitmap::insert(int x, int y, const Bitmap& source)
{
    insert(x, y, source, 0, 0, source.width(), source.height());
}

void Gosu::Bitmap::insert(int x, int y, const Bitmap& source,
                          int src_x, int src_y, int src_width, int src_height)
{
    // TODO: This should use memcpy if possible (x == 0 && src_width == this->width())

    if (x < 0) {
        int clip_left = -x;

        if (clip_left >= src_width) return;

        src_x += clip_left;
        src_width -= clip_left;
        x = 0;
    }

    if (y < 0) {
        int clip_top = -y;

        if (clip_top >= src_height) return;

        src_y += clip_top;
        src_height -= clip_top;
        y = 0;
    }

    if (x + src_width > m_width) {
        if (x >= m_width) return;

        src_width = m_width - x;
    }

    if (y + src_height > m_height) {
        if (y >= m_height) return;

        src_height = m_height - y;
    }

    for (int rel_y = 0; rel_y < src_height; ++rel_y) {
        for (int rel_x = 0; rel_x < src_width; ++rel_x) {
            set_pixel(x + rel_x, y + rel_y, source.get_pixel(src_x + rel_x, src_y + rel_y));
        }
    }
}

void Gosu::apply_color_key(Bitmap& bitmap, Color key)
{
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            if (bitmap.get_pixel(x, y) == key) {
                // Calculate the average R/G/B of adjacent, non-transparent pixels.
                unsigned neighbors = 0, red = 0, green = 0, blue = 0;
                auto visit = [&](Color c) {
                  if (c != key) {
                      neighbors += 1;
                      red += c.red();
                      green += c.green();
                      blue += c.blue();
                  }
                };

                if (x > 0) visit(bitmap.get_pixel(x - 1, y));
                if (x < bitmap.width() - 1) visit(bitmap.get_pixel(x + 1, y));
                if (y > 0) visit(bitmap.get_pixel(x, y - 1));
                if (y < bitmap.height() - 1) visit(bitmap.get_pixel(x, y + 1));

                Color replacement = Color::NONE;
                if (neighbors > 0) {
                    replacement.set_red(red / neighbors);
                    replacement.set_green(green / neighbors);
                    replacement.set_blue(blue / neighbors);
                }
                bitmap.set_pixel(x, y, replacement);
            }
        }
    }
}

Gosu::Bitmap Gosu::apply_border_flags(unsigned image_flags, const Bitmap& source,
                                      int src_x, int src_y, int src_width, int src_height)
{
    // By default, we add one pixel of transparent data around the whole image to so that during
    // interpolation, the image just fades out, instead of bleeding into adjacent image data on
    // whatever shared texture atlas it ends up on.
    // However, if a border is marked as "tileable", we instead repeat the outermost pixels, which
    // leads to a nice sharp/hard border with no interpolation at all.

    // TODO: Instead of using Color::NONE (transparent black) for non-tileable image, still repeat
    // the border pixel colors, but turn them fully transparent.

    // Backward compatibility: This used to be 'bool tileable'.
    if (image_flags == 1) image_flags = IF_TILEABLE;

    Gosu::Bitmap dest{src_width + 2, src_height + 2};

    // The borders are made "harder" by duplicating the original bitmap's
    // borders.

    // Top.
    if (image_flags & IF_TILEABLE_TOP) {
        dest.insert(1, 0,
                    source, src_x, src_y, src_width, 1);
    }
    // Bottom.
    if (image_flags & IF_TILEABLE_BOTTOM) {
        dest.insert(1, dest.height() - 1,
                    source, src_x, src_y + src_height - 1, src_width, 1);
    }
    // Left.
    if (image_flags & IF_TILEABLE_LEFT) {
        dest.insert(0, 1,
                    source, src_x, src_y, 1, src_height);
    }
    // Right.
    if (image_flags & IF_TILEABLE_RIGHT) {
        dest.insert(dest.width() - 1, 1,
                    source, src_x + src_width - 1, src_y, 1, src_height);
    }

    // Top left.
    if ((image_flags & IF_TILEABLE_TOP) && (image_flags & IF_TILEABLE_LEFT)) {
        dest.set_pixel(0, 0,
                       source.get_pixel(src_x, src_y));
    }
    // Top right.
    if ((image_flags & IF_TILEABLE_TOP) && (image_flags & IF_TILEABLE_RIGHT)) {
        dest.set_pixel(dest.width() - 1, 0,
                       source.get_pixel(src_x + src_width - 1, src_y));
    }
    // Bottom left.
    if ((image_flags & IF_TILEABLE_BOTTOM) && (image_flags & IF_TILEABLE_LEFT)) {
        dest.set_pixel(0, dest.height() - 1,
                       source.get_pixel(src_x, src_y + src_height - 1));
    }
    // Bottom right.
    if ((image_flags & IF_TILEABLE_BOTTOM) && (image_flags & IF_TILEABLE_RIGHT)) {
        dest.set_pixel(dest.width() - 1, dest.height() - 1,
                       source.get_pixel(src_x + src_width - 1, src_y + src_height - 1));
    }

    // Now put the final image into the prepared borders.
    dest.insert(1, 1,
                source, src_x, src_y, src_width, src_height);
    return dest;
}
