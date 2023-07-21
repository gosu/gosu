#include <Gosu/Bitmap.hpp>
#include <Gosu/GraphicsBase.hpp>
#include <algorithm> // for std::equal, std::fill_n
#include <cstring> // for std::memcpy
#include <stdexcept> // for std::invalid_argument
#include <utility> // for std::move, std::swap

Gosu::Bitmap::Bitmap(int width, int height, Color c)
    : m_width(width),
      m_height(height)
{
    if (width < 0 || height < 0) {
        throw std::invalid_argument("Negative Gosu::Bitmap size");
    }

    int size = width * height;
    if (width != 0 && size / width != height) {
        throw std::invalid_argument("Gosu::Bitmap size out of bounds");
    }

    m_pixels = Buffer(size * sizeof(Color));
    std::fill_n(data(), size, c);
}

Gosu::Bitmap::Bitmap(int width, int height, Gosu::Buffer&& buffer)
    : m_width(width),
      m_height(height),
      m_pixels(std::move(buffer))
{
    int pixels = width * height;
    if (static_cast<std::size_t>(pixels) * sizeof(Color) != m_pixels.size()) {
        throw std::length_error("Gosu::Bitmap given Gosu::Buffer of mismatched size, expected "
                                + std::to_string(pixels * sizeof(Color)) + ", given "
                                + std::to_string(m_pixels.size()));
    }
}

void Gosu::Bitmap::resize(int width, int height, Color c)
{
    if (width != m_width || height != m_height) {
        Bitmap temp(width, height, c);
        temp.insert(0, 0, *this);
        std::swap(*this, temp);
    }
}

void Gosu::Bitmap::blend_pixel(int x, int y, Color c)
{
    if (c.alpha == 0) {
        return;
    }

    Color out = pixel(x, y);
    if (out.alpha == 0) {
        pixel(x, y) = c;
        return;
    }

    int inv_alpha = out.alpha * (255 - c.alpha) / 255;

    out.alpha = (c.alpha + inv_alpha);
    out.red = ((c.red * c.alpha + out.red * inv_alpha) / out.alpha);
    out.green = ((c.green * c.alpha + out.green * inv_alpha) / out.alpha);
    out.blue = ((c.blue * c.alpha + out.blue * inv_alpha) / out.alpha);

    pixel(x, y) = out;
}

void Gosu::Bitmap::insert(int x, int y, const Bitmap& source)
{
    insert(x, y, source, Rect::covering(source));
}

void Gosu::Bitmap::insert(int x, int y, const Bitmap& source, Rect source_rect)
{
    if (&source == this) {
        throw std::invalid_argument("Gosu::Bitmap::insert cannot copy parts of itself");
    }

    // Make sure that the source area does not exceed the source image.
    // If we need to move the source_rect origin, then also move the target rectangle origin.
    source_rect.clip_to(Rect::covering(source), x, y);

    // Set up the target area and make sure that it does not exceed this image.
    Rect target_rect { .x = x, .y = y, .width = source_rect.width, .height = source_rect.height };
    // If we need to move the target_rect origin, then also move the source_rect origin.
    target_rect.clip_to(Rect::covering(*this), source_rect.x, source_rect.y);

    // These are the first source and first destination pixels/rows.
    Color* target_ptr = &pixel(target_rect.x, target_rect.y);
    const Color* source_ptr = &source.pixel(source_rect.x, source_rect.y);

    // target_rect might be smaller than source_rect now, so use its width/height for copying.

    if (width() == source.width() && width() == target_rect.width) {
        // If both images have the same width, and we want to copy full lines, then we can use a
        // single memcpy for the whole operation. This is especially likely if we vertically resize
        // a bitmap.
        const int size = target_rect.width * target_rect.height;
        std::memcpy(target_ptr, source_ptr, static_cast<std::size_t>(size) * sizeof(Color));
    }
    else {
        // Otherwise, we need to copy line by line.
        for (int row = 0; row < target_rect.height; ++row) {
            std::memcpy(target_ptr, source_ptr, target_rect.width * sizeof(Color));
            target_ptr += width();
            source_ptr += source.width();
        }
    }
}

bool Gosu::Bitmap::operator==(const Gosu::Bitmap& other) const
{
    int pixels = width() * height();
    return pixels == other.width() * other.height()
        && std::equal(data(), data() + pixels, other.data());
}

void Gosu::apply_color_key(Bitmap& bitmap, Color key)
{
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            if (bitmap.pixel(x, y) == key) {
                // Calculate the average R/G/B of adjacent, non-transparent pixels.
                unsigned neighbors = 0, red = 0, green = 0, blue = 0;
                const auto visit = [&](Color c) {
                    if (c != key) {
                        neighbors += 1;
                        red += c.red;
                        green += c.green;
                        blue += c.blue;
                    }
                };

                if (x > 0) {
                    visit(bitmap.pixel(x - 1, y));
                }
                if (x < bitmap.width() - 1) {
                    visit(bitmap.pixel(x + 1, y));
                }
                if (y > 0) {
                    visit(bitmap.pixel(x, y - 1));
                }
                if (y < bitmap.height() - 1) {
                    visit(bitmap.pixel(x, y + 1));
                }

                Color replacement = Color::NONE;
                if (neighbors > 0) {
                    replacement.red = red / neighbors;
                    replacement.green = green / neighbors;
                    replacement.blue = blue / neighbors;
                }
                bitmap.pixel(x, y) = replacement;
            }
        }
    }
}

Gosu::Bitmap Gosu::apply_border_flags(unsigned image_flags, const Bitmap& source, Rect source_rect)
{
    // Backward compatibility: This used to be 'bool tileable'. Make sure that anyone who still
    // passes "true" (implicitly converted to 1) automatically gets upgraded to IF_TILEABLE.
    if (image_flags == 1) {
        image_flags = IF_TILEABLE;
    }

    // Add one extra pixel around all four sides of the image.
    Gosu::Bitmap result(source_rect.width + 2, source_rect.height + 2);
    result.insert(1, 1, source, source_rect);

    // Now duplicate the edges of the image on all four sides.
    const Rect top { source_rect.x, source_rect.y, source_rect.width, 1 };
    result.insert(1, 0, source, top);
    const Rect bottom { source_rect.x, source_rect.bottom(), source_rect.width, 1 };
    result.insert(1, result.height() - 1, source, bottom);
    const Rect left { source_rect.x, source_rect.y, 1, source_rect.height };
    result.insert(0, 1, source, left);
    const Rect right { source_rect.right(), source_rect.y, 1, source_rect.height };
    result.insert(result.width() - 1, 1, source, right);
    // Also duplicate the corners of each size.
    const Rect top_left { source_rect.x, source_rect.y, 1, 1 };
    result.insert(0, 0, source, top_left);
    const Rect top_right { source_rect.right(), source_rect.y, 1, 1 };
    result.insert(result.width() - 1, 0, source, top_right);
    const Rect bottom_left { source_rect.x, source_rect.bottom(), 1, 1 };
    result.insert(0, result.height() - 1, source, bottom_left);
    const Rect bottom_right { source_rect.right(), source_rect.bottom(), 1, 1 };
    result.insert(result.width() - 1, result.height() - 1, source, bottom_right);

    // On edges which are supposed to have tileable borders, we are now finished.
    // Where soft borders are desired, we need to make all pixels on a side fully transparent.
    if ((image_flags & IF_TILEABLE_TOP) == 0) {
        for (int x = 0; x < result.width(); ++x) {
            result.pixel(x, 0).alpha = 0;
        }
    }
    if ((image_flags & IF_TILEABLE_BOTTOM) == 0) {
        const int y = result.height() - 1;
        for (int x = 0; x < result.width(); ++x) {
            result.pixel(x, y).alpha = 0;
        }
    }
    if ((image_flags & IF_TILEABLE_LEFT) == 0) {
        for (int y = 0; y < result.height(); ++y) {
            result.pixel(0, y).alpha = 0;
        }
    }
    if ((image_flags & IF_TILEABLE_RIGHT) == 0) {
        const int x = result.width() - 1;
        for (int y = 0; y < result.height(); ++y) {
            result.pixel(x, y).alpha = 0;
        }
    }

    return result;
}
