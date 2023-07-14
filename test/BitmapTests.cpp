#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <climits>
#include <stdexcept>

namespace Gosu
{

class BitmapTests : public testing::Test
{
};

inline testing::AssertionResult visible_pixels_are_equal(const Bitmap& lhs, const Bitmap& rhs)
{
    if (lhs.width() != rhs.width() || lhs.height() != rhs.height()) {
        return testing::AssertionFailure() << "different sizes";
    }

    for (int x = 0; x < lhs.width(); ++x) {
        for (int y = 0; y < lhs.height(); ++y) {
            Color lhs_pixel = lhs.get_pixel(x, y);
            Color rhs_pixel = rhs.get_pixel(x, y);

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
            ASSERT_EQ(bitmap.get_pixel(x, y), Color::RED);
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
                ASSERT_EQ(bitmap.get_pixel(x, y), Color::BLUE);
            } else {
                ASSERT_EQ(bitmap.get_pixel(x, y), Color::RED);
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
    ASSERT_EQ(bitmap.get_pixel(0, 0), 0xff'0000ff);

    // No change, blended pixel is invisible.
    bitmap.blend_pixel(1, 1, Color::BLUE.with_alpha(0));
    ASSERT_EQ(bitmap.get_pixel(1, 1), 0x40'ff0000);

    // Pixel is interpolated towards the new pixel based on both alpha values.
    bitmap.blend_pixel(2, 2, Color::GREEN.with_alpha(128));
    // Verify "Over" compositing operation: https://en.wikipedia.org/wiki/Alpha_compositing
    // alpha = alpha_new + alpha_old * (1 - alpha_new). 64 is ~0.25, 128 is ~0.5.
    const double alpha = (0.5 + 0.25 * (1 - 0.5));
    // This is a pretty large "epsilon", but blend_pixel uses math on bytes, and 64 is not really
    // a quarter of 255, so these little errors add up.
    const double epsilon = 2;
    ASSERT_NEAR(bitmap.get_pixel(2, 2).alpha, alpha * 255, epsilon);
    // c = (c_new * alpha_new + c_old * alpha_old * (1 - alpha_new)) / alpha.
    ASSERT_NEAR(bitmap.get_pixel(2, 2).red, (0 * 0.5 + 255 * 0.25 * (1 - 0.5)) / alpha, epsilon);
    ASSERT_NEAR(bitmap.get_pixel(2, 2).green, (255 * 0.5 + 0 * 0.25 * (1 - 0.5)) / alpha, epsilon);
    ASSERT_EQ(bitmap.get_pixel(2, 2).blue, 0);
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
