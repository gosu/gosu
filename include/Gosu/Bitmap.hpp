#pragma once

#include <Gosu/Buffer.hpp>
#include <Gosu/Color.hpp>
#include <Gosu/Utility.hpp>
#include <string>
#include <string_view>

namespace Gosu
{
    /// A two-dimensional area of pixels, each represented by a Color value.
    /// Internally, it is stored as a contiguous array of Gosu::Color, row by row.
    ///
    /// Bitmaps are typically created only as an intermediate step between loading image files, and
    /// creating Gosu::Image objects from them (i.e. transferring the image into video RAM).
    ///
    /// Bitmaps have (expensive) value semantics.
    class Bitmap
    {
        int m_width = 0, m_height = 0;
        Buffer m_pixels;

    public:
        Bitmap() = default;

        Bitmap(int width, int height, Color c = Color::NONE);

        /// Takes ownership of the given memory buffer.
        /// @throw std::length_error if the buffer does not contain (width * height * 4) bytes.
        Bitmap(int width, int height, Gosu::Buffer&& buffer);

        int width() const { return m_width; }

        int height() const { return m_height; }

        void resize(int width, int height, Color c = Color::NONE);

        /// Returns a reference to the pixel at the specified position without any bounds checking.
        /// This can be a two-argument operator[] once we require C++23.
        const Color& pixel(int x, int y) const { return data()[y * m_width + x]; }

        /// Returns a reference to the pixel at the specified position without any bounds checking.
        /// This can be a two-argument operator[] once we require C++23.
        Color& pixel(int x, int y) { return data()[y * m_width + x]; }

        /// This updates a pixel using the "over" alpha compositing operator, see:
        /// https://en.wikipedia.org/wiki/Alpha_compositing
        /// Like pixel(), it does not perform any bounds checking.
        void blend_pixel(int x, int y, Color c);

        /// Inserts a bitmap at the given position. Parts of the inserted bitmap that would be
        /// outside of the target bitmap will be clipped away.
        void insert(const Bitmap& source, int x, int y);

        /// Inserts a portion of a bitmap at the given position. Parts of the inserted bitmap that
        /// would be outside of the target bitmap will be clipped away.
        /// Parts of the source_rect that are outside of the source image will be skipped.
        void insert(const Bitmap& source, int x, int y, Rect source_rect);

        /// Set the alpha value of all pixels which are equal to the color key to zero.
        /// To reduce interpolation artifacts when stretching or rotating the resulting image, the
        /// RGB values of transparent pixels will be adjusted to the average of their neighbors.
        void apply_color_key(Color key);

        /// Direct access to the array of color values.
        const Color* data() const { return reinterpret_cast<const Color*>(m_pixels.data()); }

        /// Direct access to the array of color values.
        Color* data() { return reinterpret_cast<Color*>(m_pixels.data()); }

        bool operator==(const Bitmap&) const;
    };

    /// Loads an image file, in any supported format.
    Bitmap load_image_file(const std::string& filename);
    /// Loads an image from memory, in any supported format.
    Bitmap load_image(const Buffer& buffer);

    /// Saves a Bitmap to a file.
    void save_image_file(const Bitmap& bitmap, const std::string& filename);
    /// Saves a Bitmap to an arbitrary resource.
    Gosu::Buffer save_image(const Bitmap& bitmap, std::string_view format_hint = "png");

    Bitmap apply_border_flags(unsigned image_flags, const Bitmap& source, Rect source_rect);
}
