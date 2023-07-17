#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <climits> // for INT_MAX
#include <random>
#include <stdexcept> // for std::invalid_argument

namespace Gosu
{

class BitmapTests : public testing::Test
{
public:
    static testing::AssertionResult visible_pixels_are_equal(const Bitmap& lhs, const Bitmap& rhs)
    {
        if (lhs.width() != rhs.width() || lhs.height() != rhs.height()) {
            return testing::AssertionFailure() << "different sizes";
        }

        for (int x = 0; x < lhs.width(); ++x) {
            for (int y = 0; y < lhs.height(); ++y) {
                Color lhs_pixel = lhs.pixel(x, y);
                Color rhs_pixel = rhs.pixel(x, y);

                if (lhs_pixel.alpha == 0 && rhs_pixel.alpha == 0) {
                    continue;
                }

                if (lhs_pixel != rhs_pixel) {
                    return testing::AssertionFailure() << "difference at " << x << ", " << y;
                }
            }
        }

        return testing::AssertionSuccess();
    }

    // This is a naive implementation of Bitmap::insert that can be used to verify that the
    // optimized version behaves correctly.
    static Bitmap insert_naively(const Bitmap& target, int x, int y, const Bitmap& source,
                                 const Rect& source_rect)
    {
        Bitmap result = target;

        for (int rel_y = 0; rel_y < source_rect.height; ++rel_y) {
            for (int rel_x = 0; rel_x < source_rect.width; ++rel_x) {
                const int target_x = x + rel_x;
                if (target_x < 0 || target_x >= target.width()) {
                    continue;
                }
                const int target_y = y + rel_y;
                if (target_y < 0 || target_y >= target.height()) {
                    continue;
                }
                const int source_x = source_rect.x + rel_x;
                if (source_x < 0 || source_x >= source.width()) {
                    continue;
                }
                const int source_y = source_rect.y + rel_y;
                if (source_y < 0 || source_y >= source.height()) {
                    continue;
                }

                result.pixel(target_x, target_y) = source.pixel(source_x, source_y);
            }
        }

        return result;
    }
};

TEST_F(BitmapTests, memory_management)
{
    Bitmap empty_bitmap;
    ASSERT_EQ(empty_bitmap.width(), 0);
    ASSERT_EQ(empty_bitmap.height(), 0);
    ASSERT_EQ(empty_bitmap.m_pixels.size(), 0);

    Bitmap bitmap(7, 3, Color::RED);
    ASSERT_EQ(bitmap.width(), 7);
    ASSERT_EQ(bitmap.height(), 3);
    ASSERT_EQ(bitmap.m_pixels.size(), 7 * 3);
    // Verify that everything was filled with the color constructor parameter.
    for (int x = 0; x < bitmap.width(); ++x) {
        for (int y = 0; y < bitmap.height(); ++y) {
            ASSERT_EQ(bitmap.pixel(x, y), Color::RED);
        }
    }

    bitmap.resize(11, 4, Color::BLUE);
    ASSERT_EQ(bitmap.width(), 11);
    ASSERT_EQ(bitmap.height(), 4);
    ASSERT_EQ(bitmap.m_pixels.size(), 11 * 4);
    // When resizing, pixels to the right and below the original size must use the new color.
    for (int x = 0; x < bitmap.width(); ++x) {
        for (int y = 0; y < bitmap.height(); ++y) {
            if (x >= 7 || y >= 3) {
                ASSERT_EQ(bitmap.pixel(x, y), Color::BLUE);
            } else {
                ASSERT_EQ(bitmap.pixel(x, y), Color::RED);
            }
        }
    }

    std::swap(bitmap, empty_bitmap);
    ASSERT_TRUE(bitmap.m_pixels.empty());

    ASSERT_THROW(Bitmap(-5, 3), std::invalid_argument);
    ASSERT_THROW(Bitmap(INT_MAX, INT_MAX), std::invalid_argument);
}

TEST_F(BitmapTests, blend_pixel)
{
    Bitmap bitmap(3, 3, Color::RED.with_alpha(64));

    // Pixel gets fully replaced.
    bitmap.blend_pixel(0, 0, Color::BLUE.with_alpha(255));
    ASSERT_EQ(bitmap.pixel(0, 0), 0xff'0000ff);

    // No change, blended pixel is invisible.
    bitmap.blend_pixel(1, 1, Color::BLUE.with_alpha(0));
    ASSERT_EQ(bitmap.pixel(1, 1), 0x40'ff0000);

    // Pixel is interpolated towards the new pixel based on both alpha values.
    bitmap.blend_pixel(2, 2, Color::GREEN.with_alpha(128));
    // Verify "Over" compositing operation: https://en.wikipedia.org/wiki/Alpha_compositing
    // alpha = alpha_new + alpha_old * (1 - alpha_new). 64 is ~0.25, 128 is ~0.5.
    const double alpha = (0.5 + 0.25 * (1 - 0.5));
    // This is a pretty large "epsilon", but blend_pixel uses integer math, and 64 is not really
    // a quarter of 255, so these little errors add up.
    const double epsilon = 2;
    ASSERT_NEAR(bitmap.pixel(2, 2).alpha, alpha * 255, epsilon);
    // c = (c_new * alpha_new + c_old * alpha_old * (1 - alpha_new)) / alpha.
    ASSERT_NEAR(bitmap.pixel(2, 2).red, (0 * 0.5 + 255 * 0.25 * (1 - 0.5)) / alpha, epsilon);
    ASSERT_NEAR(bitmap.pixel(2, 2).green, (255 * 0.5 + 0 * 0.25 * (1 - 0.5)) / alpha, epsilon);
    ASSERT_EQ(bitmap.pixel(2, 2).blue, 0);
}

TEST_F(BitmapTests, insert)
{
    Bitmap canvas(5, 3);
    canvas.m_pixels = {
        0x00'000000, 0x00'000011, 0x00'000022, 0x00'000033, 0x00'000044, //
        0x00'000055, 0x00'000066, 0x00'000077, 0x00'000088, 0x00'000099, //
        0x00'0000aa, 0x00'0000bb, 0x00'0000cc, 0x00'0000dd, 0x00'0000ee, //
    };

    Bitmap red(3, 2);
    red.m_pixels = {
        0xff'ff0000, 0xff'ff0011, 0xff'ff0022, //
        0xff'ff0033, 0xff'ff0044, 0xff'ff0055, //
    };
    // Overwrite 2x1 pixels in the top left corner.
    canvas.insert(-1, -1, red);
    // No-op: Too far up.
    canvas.insert(-10, 0, red);
    // No-op: Too far to the left.
    canvas.insert(0, -10, red);
    // No-op: Source area does not match input.

    Bitmap green(4, 3);
    green.m_pixels = {
        0x80'00ff00, 0x80'00ff11, 0x80'00ff22, 0x80'00ff33, //
        0x80'00ff44, 0x80'00ff55, 0x80'00ff66, 0x80'00ff77, //
        0x80'00ff88, 0x80'00ff99, 0x80'00ffaa, 0x80'00ffbb, //
    };
    // Overwrite 2x2 pixels in the bottom left corner.
    canvas.insert(3, 1, green);
    // No-op: Too far to the right.
    canvas.insert(5, 0, green);
    // No-op: Too far down.
    canvas.insert(0, 3, green);

    const std::vector<Color> expected_pixels {
        0xff'ff0044, 0xff'ff0055, 0x00'000022, 0x00'000033, 0x00'000044, //
        0x00'000055, 0x00'000066, 0x00'000077, 0x80'00ff00, 0x80'00ff11, //
        0x00'0000aa, 0x00'0000bb, 0x00'0000cc, 0x80'00ff44, 0x80'00ff55, //
    };
    ASSERT_EQ(canvas.m_pixels, expected_pixels);
}

TEST_F(BitmapTests, insert_fuzzing)
{
    std::random_device rd;
    std::mt19937_64 mt(rd());

    const auto next_size = [&mt] {
        static std::uniform_int_distribution size_distribution(0, 20);
        return size_distribution(mt);
    };

    const auto next_offset = [&mt] {
        static std::uniform_int_distribution offset_distribution(-20, 20);
        return offset_distribution(mt);
    };

    const auto next_color = [&mt] {
        static std::uniform_int_distribution<std::uint64_t> color_distribution(0);
        return color_distribution(mt);
    };

    for (int i = 0; i < 20'000; ++i) {
        Bitmap target(next_size(), next_size());
        std::generate_n(target.data(), target.width() * target.height(), next_color);

        Bitmap source(next_size(), next_size());
        std::generate_n(source.data(), source.width() * source.height(), next_color);

        const Rect source_rect { next_offset(), next_offset(), next_size(), next_size() };
        const int x = next_offset();
        const int y = next_offset();

        const Bitmap naive_result = insert_naively(target, x, y, source, source_rect);
        target.insert(x, y, source, source_rect);
        ASSERT_EQ(target, naive_result);
    }
}

TEST_F(BitmapTests, image_formats_with_alpha_channel)
{
    Bitmap bmp24 = load_image_file("test_image_io/alpha-bmp24.bmp");
    Bitmap png4 = load_image_file("test_image_io/alpha-png4.png");
    Bitmap png8 = load_image_file("test_image_io/alpha-png8.png");

    // Load one file via Gosu::IO to spice things up:
    File file { "test_image_io/alpha-png32.png" };
    Bitmap png32 = load_image_file(file.front_reader());

    // Also load one more image that we know is _not_ the same as the others to test that our
    // comparison actually does something.
    Bitmap not_equal = load_image_file("test_image_io/no-alpha-jpg.jpg");

    ASSERT_TRUE(visible_pixels_are_equal(bmp24, png4));
    ASSERT_TRUE(visible_pixels_are_equal(bmp24, png8));
    ASSERT_TRUE(visible_pixels_are_equal(bmp24, png32));
    ASSERT_FALSE(visible_pixels_are_equal(bmp24, not_equal));
}

TEST_F(BitmapTests, image_files_with_full_opacity)
{
    Bitmap jpg = load_image_file("test_image_io/no-alpha-jpg.jpg");
    Bitmap png32 = load_image_file("test_image_io/no-alpha-png32.png");
    ASSERT_TRUE(visible_pixels_are_equal(jpg, png32));
}

}
