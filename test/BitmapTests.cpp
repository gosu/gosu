#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <algorithm> // for std::copy_n
#include <climits> // for INT_MAX
#include <filesystem>
#include <numeric> // for std::iota
#include <random>
#include <stdexcept> // for std::invalid_argument

class BitmapTests : public testing::Test
{
public:
    static void assign_pixels(Gosu::Bitmap& target, std::initializer_list<Gosu::Color> pixels)
    {
        ASSERT_EQ(target.width() * target.height(), pixels.size());
        std::copy_n(std::data(pixels), pixels.size(), target.data());
    }

    static testing::AssertionResult
    visible_pixels_are_equal(const Gosu::Bitmap& lhs, const Gosu::Bitmap& rhs, int tolerance = 0)
    {
        if (lhs.width() != rhs.width() || lhs.height() != rhs.height()) {
            return testing::AssertionFailure() << "different sizes";
        }

        for (int x = 0; x < lhs.width(); ++x) {
            for (int y = 0; y < lhs.height(); ++y) {
                Gosu::Color lhs_pixel = lhs.pixel(x, y);
                Gosu::Color rhs_pixel = rhs.pixel(x, y);

                if (lhs_pixel.alpha == 0 && rhs_pixel.alpha == 0) {
                    continue;
                }

                if (lhs_pixel.alpha != rhs_pixel.alpha
                    || std::abs(lhs_pixel.red - rhs_pixel.red) > tolerance
                    || std::abs(lhs_pixel.green - rhs_pixel.green) > tolerance
                    || std::abs(lhs_pixel.blue - rhs_pixel.blue) > tolerance) {
                    return testing::AssertionFailure() << "difference at " << x << ", " << y;
                }
            }
        }

        return testing::AssertionSuccess();
    }

    // This is a naive implementation of Bitmap::insert that can be used to verify that the
    // optimized version behaves correctly.
    static Gosu::Bitmap insert_naively(const Gosu::Bitmap& target, int x, int y,
                                       const Gosu::Bitmap& source, const Gosu::Rect& source_rect)
    {
        Gosu::Bitmap result = target;

        for (int rel_y = 0; rel_y < source_rect.height; ++rel_y) {
            for (int rel_x = 0; rel_x < source_rect.width; ++rel_x) {
                // Skip pixels that are outside the target bitmap.
                const int target_x = x + rel_x;
                if (target_x < 0 || target_x >= target.width()) {
                    continue;
                }
                const int target_y = y + rel_y;
                if (target_y < 0 || target_y >= target.height()) {
                    continue;
                }
                // Skip pixels that are outside the source bitmap.
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

TEST_F(BitmapTests, construct_from_buffer)
{
    int size = 3 * 5;
    Gosu::Buffer buffer(static_cast<std::size_t>(size) * sizeof(Gosu::Color));
    std::iota(buffer.data(), buffer.data() + buffer.size(), 0);

    Gosu::Bitmap bitmap(3, 5, std::move(buffer));
    ASSERT_EQ(buffer.size(), 0); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)
    ASSERT_EQ(bitmap.width(), 3);
    ASSERT_EQ(bitmap.height(), 5);
    ASSERT_EQ(bitmap.pixel(0, 0).red, 0);
    ASSERT_EQ(bitmap.pixel(0, 0).green, 1);
    ASSERT_EQ(bitmap.pixel(0, 0).blue, 2);
    ASSERT_EQ(bitmap.pixel(0, 0).alpha, 3);
    // ... more RGBA values ...
    ASSERT_EQ(bitmap.pixel(2, 4).red, 56);
    ASSERT_EQ(bitmap.pixel(2, 4).green, 57);
    ASSERT_EQ(bitmap.pixel(2, 4).blue, 58);
    ASSERT_EQ(bitmap.pixel(2, 4).alpha, 59);

    ASSERT_THROW(Gosu::Bitmap(10, 10, std::move(buffer)), std::length_error);
}

TEST_F(BitmapTests, visible_pixels_are_equal)
{
    Gosu::Bitmap red(1, 1, Gosu::Color::RED);
    Gosu::Bitmap blue(1, 1, Gosu::Color::BLUE);
    ASSERT_FALSE(visible_pixels_are_equal(red, blue));

    // A tolerance of 255 makes all colors the same.
    ASSERT_TRUE(visible_pixels_are_equal(red, blue, 255));

    Gosu::Bitmap red_but_larger(2, 2, Gosu::Color::RED);
    ASSERT_FALSE(visible_pixels_are_equal(red, red_but_larger));
}

TEST_F(BitmapTests, resize)
{
    Gosu::Bitmap bitmap(7, 3, Gosu::Color::RED);
    ASSERT_EQ(bitmap.width(), 7);
    ASSERT_EQ(bitmap.height(), 3);
    // Verify that everything was filled with the color constructor parameter.
    for (int x = 0; x < bitmap.width(); ++x) {
        for (int y = 0; y < bitmap.height(); ++y) {
            ASSERT_EQ(bitmap.pixel(x, y), Gosu::Color::RED);
        }
    }

    bitmap.resize(11, 4, Gosu::Color::BLUE);
    ASSERT_EQ(bitmap.width(), 11);
    ASSERT_EQ(bitmap.height(), 4);
    // When resizing, pixels to the right and below the original size must use the new color.
    for (int x = 0; x < bitmap.width(); ++x) {
        for (int y = 0; y < bitmap.height(); ++y) {
            if (x >= 7 || y >= 3) {
                ASSERT_EQ(bitmap.pixel(x, y), Gosu::Color::BLUE);
            }
            else {
                ASSERT_EQ(bitmap.pixel(x, y), Gosu::Color::RED);
            }
        }
    }

    ASSERT_THROW(Gosu::Bitmap(-5, 3), std::invalid_argument);
    ASSERT_THROW(Gosu::Bitmap(INT_MAX, INT_MAX), std::invalid_argument);
}

TEST_F(BitmapTests, blend_pixel)
{
    Gosu::Bitmap bitmap(3, 3, Gosu::Color::RED.with_alpha(64));

    // Pixel gets fully replaced.
    bitmap.blend_pixel(0, 0, Gosu::Color::BLUE.with_alpha(255));
    ASSERT_EQ(bitmap.pixel(0, 0), 0xff'0000ff);

    // No change, blended pixel is invisible.
    bitmap.blend_pixel(1, 1, Gosu::Color::BLUE.with_alpha(0));
    ASSERT_EQ(bitmap.pixel(1, 1), 0x40'ff0000);

    // Pixel is interpolated towards the new pixel based on both alpha values.
    bitmap.blend_pixel(2, 2, Gosu::Color::GREEN.with_alpha(128));
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
    Gosu::Bitmap canvas(5, 3);
    assign_pixels(canvas,
                  {
                      0x00'000000, 0x00'000011, 0x00'000022, 0x00'000033, 0x00'000044, //
                      0x00'000055, 0x00'000066, 0x00'000077, 0x00'000088, 0x00'000099, //
                      0x00'0000aa, 0x00'0000bb, 0x00'0000cc, 0x00'0000dd, 0x00'0000ee, //
                  });

    Gosu::Bitmap red(3, 2);
    assign_pixels(red,
                  {
                      0xff'ff0000, 0xff'ff0011, 0xff'ff0022, //
                      0xff'ff0033, 0xff'ff0044, 0xff'ff0055, //
                  });
    // Overwrite 2x1 pixels in the top left corner.
    canvas.insert(red, -1, -1);
    // No-op: Too far up.
    canvas.insert(red, -10, 0);
    // No-op: Too far to the left.
    canvas.insert(red, 0, -10);
    // No-op: Source area does not match input.

    Gosu::Bitmap green(4, 3);
    assign_pixels(green,
                  {
                      0x80'00ff00, 0x80'00ff11, 0x80'00ff22, 0x80'00ff33, //
                      0x80'00ff44, 0x80'00ff55, 0x80'00ff66, 0x80'00ff77, //
                      0x80'00ff88, 0x80'00ff99, 0x80'00ffaa, 0x80'00ffbb, //
                  });
    // Overwrite 2x2 pixels in the bottom left corner.
    canvas.insert(green, 3, 1);
    // No-op: Too far to the right.
    canvas.insert(green, 5, 0);
    // No-op: Too far down.
    canvas.insert(green, 0, 3);

    Gosu::Bitmap expected_result(5, 3);
    assign_pixels(expected_result,
                  {
                      0xff'ff0044, 0xff'ff0055, 0x00'000022, 0x00'000033, 0x00'000044, //
                      0x00'000055, 0x00'000066, 0x00'000077, 0x80'00ff00, 0x80'00ff11, //
                      0x00'0000aa, 0x00'0000bb, 0x00'0000cc, 0x80'00ff44, 0x80'00ff55, //
                  });
    ASSERT_EQ(canvas, expected_result);

    // Bitmap::insert does not support insertion into itself (yet).
    ASSERT_THROW(canvas.insert(canvas, 0, 0), std::invalid_argument);
}

TEST_F(BitmapTests, apply_color_key)
{
    Gosu::Bitmap bitmap(3, 5);
    // The input image consists of five pixels (red, green, blue, yellow, cyan).
    // Everything else is fuchsia (#ff00ff) and will be made transparent:
    assign_pixels(bitmap,
                  {
                      0xff'ff00ff, 0xff'ff0000, 0xff'ff00ff, // _ R _
                      0xff'00ff00, 0xff'ff00ff, 0xff'0000ff, // G _ B
                      0xff'ff00ff, 0xff'ffff00, 0xff'ff00ff, // _ Y _
                      0xff'ff00ff, 0xff'ff00ff, 0xff'ff00ff, // _ _ _
                      0xff'00ffff, 0xff'ff00ff, 0xff'ff00ff, // C _ _
                  });
    bitmap.apply_color_key(Gosu::Color::FUCHSIA);

    // The alpha values of all fuchsia pixels must be 0, and the color values are the average of
    // their surrounding pixels.
    Gosu::Bitmap expected_result(3, 5);
    assign_pixels(expected_result,
                  {
                      0x00'7f7f00, 0xff'ff0000, 0x00'7f007f, // _ R _
                      0xff'00ff00, 0x00'7f7f3f, 0xff'0000ff, // G _ B
                      0x00'7fff00, 0xff'ffff00, 0x00'7f7f7f, // _ Y _
                      0x00'7fff7f, 0x00'7fff7f, 0x00'ffff00, // _ _ _
                      0xff'00ffff, 0x00'00ffff, 0x00'000000, // C _ _
                  });
    ASSERT_TRUE(visible_pixels_are_equal(bitmap, expected_result));
    ASSERT_EQ(bitmap, expected_result);
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
        Gosu::Bitmap target(next_size(), next_size());
        std::generate_n(target.data(), target.width() * target.height(), next_color);

        Gosu::Bitmap source(next_size(), next_size());
        std::generate_n(source.data(), source.width() * source.height(), next_color);

        const Gosu::Rect source_rect { next_offset(), next_offset(), next_size(), next_size() };
        const int x = next_offset();
        const int y = next_offset();

        const Gosu::Bitmap naive_result = insert_naively(target, x, y, source, source_rect);
        target.insert(source, x, y, source_rect);
        ASSERT_EQ(target, naive_result);
    }
}

TEST_F(BitmapTests, image_formats_with_alpha_channel)
{
    Gosu::Bitmap bmp24 = Gosu::load_image_file("test_image_io/alpha-bmp24.bmp");
    Gosu::Bitmap gif = Gosu::load_image_file("test_image_io/alpha-gif.gif");
    Gosu::Bitmap psd = Gosu::load_image_file("test_image_io/alpha-psd.psd");
    Gosu::Bitmap png4 = Gosu::load_image_file("test_image_io/alpha-png4.png");
    Gosu::Bitmap png8 = Gosu::load_image_file("test_image_io/alpha-png8.png");

    // Load one file via Gosu::Buffer to spice things up:
    Gosu::Buffer buffer = Gosu::load_file("test_image_io/alpha-png32.png");
    Gosu::Bitmap png32 = load_image(buffer);

    ASSERT_TRUE(visible_pixels_are_equal(bmp24, gif));
    ASSERT_TRUE(visible_pixels_are_equal(bmp24, psd));
    ASSERT_TRUE(visible_pixels_are_equal(bmp24, png4));
    ASSERT_TRUE(visible_pixels_are_equal(bmp24, png8));
    ASSERT_TRUE(visible_pixels_are_equal(bmp24, png32));
}

TEST_F(BitmapTests, image_files_with_full_opacity)
{
    Gosu::Bitmap jpg = Gosu::load_image_file("test_image_io/no-alpha-jpg.jpg");
    Gosu::Bitmap png32 = Gosu::load_image_file("test_image_io/no-alpha-png32.png");
    ASSERT_TRUE(visible_pixels_are_equal(jpg, png32));
}

TEST_F(BitmapTests, image_format_roundtrips)
{
    const auto temp_dir = std::filesystem::temp_directory_path();

    for (const std::string& filename :
         { "alpha-bmp24.bmp", "alpha-png32.png", "no-alpha-jpg.jpg" }) {
        // Allow small RGB differences when saving files using the lossy JPEG format.
        int tolerance = (Gosu::has_extension(filename, "jpg") ? 10 : 0);

        Gosu::Bitmap image = Gosu::load_image_file("test_image_io/" + filename);

        // Roundtrip to file and back.
        const std::string temp_filename = temp_dir / filename;
        save_image_file(image, temp_filename);
        Gosu::Bitmap image_from_file = Gosu::load_image_file(temp_filename);
        ASSERT_TRUE(visible_pixels_are_equal(image, image_from_file, tolerance))
            << "difference in " << temp_filename;
        std::filesystem::remove(temp_filename);

        // Roundtrip to memory buffer and back.
        Gosu::Buffer buffer = Gosu::save_image(image, filename);
        Gosu::Bitmap image_from_buffer = Gosu::load_image(buffer);
        ASSERT_TRUE(visible_pixels_are_equal(image, image_from_buffer, tolerance))
            << "difference in " << temp_filename;
    }
}

TEST_F(BitmapTests, image_io_errors)
{
    ASSERT_THROW(Gosu::load_image_file(""), std::runtime_error);
    ASSERT_THROW(Gosu::load_image(Gosu::Buffer(0)), std::runtime_error);

    const Gosu::Bitmap bitmap(10, 10);
    ASSERT_THROW(Gosu::save_image_file(bitmap, ""), std::runtime_error);

    const Gosu::Bitmap empty_bitmap(0, 0);
    // The JPEG format does not support empty files.
    ASSERT_THROW(Gosu::save_image(empty_bitmap, "jpg"), std::runtime_error);
}
