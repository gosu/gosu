#pragma once

#include <Gosu/Color.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <string>
#include <vector>

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
        int m_width = 0;
        int m_height = 0;
        std::vector<Color> m_pixels;

    public:
        Bitmap() = default;

        Bitmap(int width, int height, Color c = Color::NONE);

        int width() const { return m_width; }

        int height() const { return m_height; }

        void resize(int width, int height, Color c = Color::NONE);

        /// Returns a reference to the pixel at the specified position without any bounds checking.
        /// This can be a two-argument operator[] once we require C++23.
        const Color& pixel(int x, int y) const { return m_pixels[y * m_width + x]; }

        /// Returns a reference to the pixel at the specified position without any bounds checking.
        /// This can be a two-argument operator[] once we require C++23.
        Color& pixel(int x, int y) { return m_pixels[y * m_width + x]; }

        /// This updates a pixel using the "over" alpha compositing operator, see:
        /// https://en.wikipedia.org/wiki/Alpha_compositing
        /// Like pixel(), it does not perform any bounds checking.
        void blend_pixel(int x, int y, Color c);

        /// Inserts a bitmap at the given position. Parts of the inserted bitmap that would be
        /// outside of the target bitmap will be clipped away.
        void insert(int x, int y, const Bitmap& source);

        /// Inserts a portion of a bitmap at the given position. Parts of the inserted bitmap that
        /// would be outside of the target bitmap will be clipped away.
        /// Parts of the source_rect that are outside of the source image will be skipped.
        void insert(int x, int y, const Bitmap& source, Rect source_rect);

        /// Direct access to the array of color values.
        const Color* data() const { return m_pixels.data(); }

        /// Direct access to the array of color values.
        Color* data() { return m_pixels.data(); }

        bool operator==(const Bitmap&) const = default;

#ifdef FRIEND_TEST
        FRIEND_TEST(BitmapTests, memory_management);
        FRIEND_TEST(BitmapTests, insert);
#endif
    };

    /// Loads any supported image into a Bitmap.
    Bitmap load_image_file(const std::string& filename);
    /// Loads any supported image into a Bitmap.
    Bitmap load_image_file(Reader input);

    /// Saves a Bitmap to a file.
    void save_image_file(const Bitmap& bitmap, const std::string& filename);
    /// Saves a Bitmap to an arbitrary resource.
    void save_image_file(const Bitmap& bitmap, Writer writer, std::string_view format_hint = "png");

    /// Set the alpha value of all pixels which are equal to the color key to zero.
    /// To reduce interpolation artefacts when stretching or rotating the resulting image, the color
    /// value of these transparent pixels will be adjusted to the average of their neighbors.
    void apply_color_key(Bitmap& bitmap, Color key);

    Bitmap apply_border_flags(unsigned image_flags, const Bitmap& source, Rect source_rect);
}
