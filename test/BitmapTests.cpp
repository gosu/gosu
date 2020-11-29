#include <gtest/gtest.h>
#include <Gosu/Bitmap.hpp>

namespace Gosu
{
    class BitmapTests : public testing::Test
    {
    };

    static testing::AssertionResult visible_pixels_are_equal(const Bitmap& lhs, const Bitmap& rhs)
    {
        if (lhs.width() != rhs.width() || lhs.height() != rhs.height()) {
            return testing::AssertionFailure() << "different sizes";
        }

        for (int x = 0; x < lhs.width(); ++x) {
            for (int y = 0; y < lhs.height(); ++y) {
                Color lhs_pixel = lhs.get_pixel(x, y);
                Color rhs_pixel = rhs.get_pixel(x, y);

                if (lhs_pixel.alpha() == 0 && rhs_pixel.alpha() == 0) continue;

                if (lhs_pixel != rhs_pixel) {
                    return testing::AssertionFailure() << "difference at " << x << ", " << y;
                }
            }
        }

        return testing::AssertionSuccess();
    }

    TEST_F(BitmapTests, MemoryManagement)
    {
        Bitmap empty_bitmap;
        ASSERT_EQ(empty_bitmap.width(), 0);
        ASSERT_EQ(empty_bitmap.height(), 0);
        ASSERT_EQ(empty_bitmap.m_pixels.size(), 0);

        Bitmap bitmap{7, 3, Color::RED};
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
                }
                else {
                    ASSERT_EQ(bitmap.get_pixel(x, y), Color::RED);
                }
            }
        }

        bitmap.swap(empty_bitmap);
        ASSERT_TRUE(bitmap.m_pixels.empty());
    }

    TEST_F(BitmapTests, ImageFormatsWithAlphaChannel)
    {
        Bitmap bmp24 = load_image_file("test_image_io/alpha-bmp24.bmp");
        Bitmap png4 = load_image_file("test_image_io/alpha-png4.png");
        Bitmap png8 = load_image_file("test_image_io/alpha-png8.png");

        // Load one file via Gosu::IO to spice things up:
        File file{"test_image_io/alpha-png32.png"};
        Bitmap png32 = load_image_file(file.front_reader());

        // Also load one more image that we know is _not_ the same as the others to test that our
        // comparison actually does something.
        Bitmap not_equal = load_image_file("test_image_io/no-alpha-jpg.jpg");

        ASSERT_TRUE(visible_pixels_are_equal(bmp24, png4));
        ASSERT_TRUE(visible_pixels_are_equal(bmp24, png8));
        ASSERT_TRUE(visible_pixels_are_equal(bmp24, png32));
        ASSERT_FALSE(visible_pixels_are_equal(bmp24, not_equal));
    }

    TEST_F(BitmapTests, ImageFilesWithFullOpacity)
    {
        Bitmap jpg = load_image_file("test_image_io/no-alpha-jpg.jpg");
        Bitmap png32 = load_image_file("test_image_io/no-alpha-png32.png");
        ASSERT_TRUE(visible_pixels_are_equal(jpg, png32));
    }
}
