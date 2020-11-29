#pragma once

#include <Gosu/Color.hpp>
#include <Gosu/IO.hpp>
#include <string>
#include <vector>

namespace Gosu
{
    /// A two-dimensional array area of pixels, each represented by a Color value.
    /// Bitmaps are typically created only as an intermediate step between loading image files, and
    /// creating Gosu::Image objects from them (i.e. transferring the image into video RAM).
    /// Has (expensive) value semantics.
    class Bitmap
    {
        int m_width = 0, m_height = 0;
        std::vector<Color> m_pixels;

    public:
        Bitmap() = default;

        Bitmap(int width, int height, Color c = Color::NONE);

        int width() const
        {
            return m_width;
        }

        int height() const
        {
            return m_height;
        }

        void swap(Bitmap& other);

        void resize(int width, int height, Color c = Color::NONE);

        /// Returns the color at the specified position without any bounds checking.
        Color get_pixel(int x, int y) const
        {
            return m_pixels[y * m_width + x];
        }

        /// Sets the pixel at the specified position without any bounds checking.
        void set_pixel(int x, int y, Color c)
        {
            m_pixels[y * m_width + x] = c;
        }

        /// This updates a pixel using the "over" alpha compositing operator, see:
        /// https://en.wikipedia.org/wiki/Alpha_compositing
        void blend_pixel(int x, int y, Color c);

        /// Inserts a bitmap at the given position. Parts of the inserted bitmap that would be
        /// outside of the target bitmap will be clipped away.
        void insert(int x, int y, const Bitmap& source);

        /// Inserts a portion of a bitmap at the given position. Parts of the inserted bitmap that
        /// would be outside of the target bitmap will be clipped away.
        void insert(int x, int y, const Bitmap& source,
                    int src_x, int src_y, int src_width, int src_height);

        /// Direct access to the array of color values.
        /// The return value is undefined if this bitmap is empty.
        const Color* data() const
        {
            return &m_pixels[0];
        }

        /// Direct access to the array of color values.
        /// The return value is undefined if this bitmap is empty.
        Color* data()
        {
            return &m_pixels[0];
        }

        #ifdef FRIEND_TEST
        FRIEND_TEST(BitmapTests, MemoryManagement);
        #endif
    };

    /// Loads any supported image into a Bitmap.
    Bitmap load_image_file(const std::string& filename);
    /// Loads any supported image into a Bitmap.
    Bitmap load_image_file(Reader input);

    /// Saves a Bitmap to a file.
    void save_image_file(const Bitmap& bitmap, const std::string& filename);
    /// Saves a Bitmap to an arbitrary resource.
    void save_image_file(const Bitmap& bitmap, Writer writer,
                         const std::string_view& format_hint = "png");

    /// Set the alpha value of all pixels which are equal to the color key to zero.
    /// To reduce interpolation artefacts when stretching or rotating the resulting image, the color
    /// value of these pixels will also be adjusted to the average of their surrounding pixels.
    void apply_color_key(Bitmap& bitmap, Color key);

    Bitmap apply_border_flags(unsigned image_flags, const Bitmap& source,
                              int src_x, int src_y, int src_width, int src_height);
}
