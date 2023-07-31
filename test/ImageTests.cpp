#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Drawable.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>

class ImageTests : public testing::Test
{
};

TEST_F(ImageTests, empty_image)
{
    Gosu::Image image;
    ASSERT_EQ(image.width(), 0);
    ASSERT_EQ(image.height(), 0);
    ASSERT_NO_THROW(image.drawable().insert(Gosu::Bitmap(3, 5, Gosu::Color::GREEN), -4, 5));
    ASSERT_EQ(image.drawable().subimage(Gosu::Rect { 0, -3, 3, 6 }), nullptr);
    ASSERT_EQ(image.drawable().gl_tex_info(), nullptr);
    ASSERT_NO_THROW(image.draw(53, 324.6, 0, 1.0, 1.0, Gosu::Color::RED));
    ASSERT_EQ(image.drawable().to_bitmap(), Gosu::Bitmap(0, 0));
}

TEST_F(ImageTests, render_after_color_key)
{
    // Create four yellow dots on a fuchsia background.
    Gosu::Bitmap bitmap(3, 3, Gosu::Color::FUCHSIA);
    bitmap.pixel(0, 0) = Gosu::Color::YELLOW;
    bitmap.pixel(2, 0) = Gosu::Color::YELLOW;
    bitmap.pixel(0, 2) = Gosu::Color::YELLOW;
    bitmap.pixel(2, 2) = Gosu::Color::YELLOW;
    // Remove fuchsia.
    bitmap.apply_color_key(Gosu::Color::FUCHSIA);
    // Now all previously fuchsia pixels should be yellow (like their neighbors) with alpha = 0.
    ASSERT_EQ(bitmap.pixel(1, 1), Gosu::Color::YELLOW.with_alpha(0));
    // Turn it into an image for rendering. (Use 'true' to test a backward compatibility code path.)
    Gosu::Image image(Gosu::create_drawable(bitmap, Gosu::Rect::covering(bitmap), true));

    const Gosu::Image result = Gosu::Graphics::render(300, 300, [&] {
        Gosu::Graphics::draw_rect(0, 0, 300, 300, Gosu::Color::YELLOW, 0);
        image.draw(0, 0, 1, 100, 100);
    });

    Gosu::Bitmap result_bitmap = result.drawable().to_bitmap();
    for (int y = 0; y < result_bitmap.height(); ++y) {
        for (int x = 0; x < result_bitmap.width(); ++x) {
            // TODO: Is it normal/expected that render-ed images have alpha<255 in some places?
            result_bitmap.pixel(x, y).alpha = 255;
            ASSERT_EQ(result_bitmap.pixel(x, y), Gosu::Color::YELLOW);
        }
    }
}

TEST_F(ImageTests, load_tiles_from_tile)
{
    const std::vector<Gosu::Image> tiles
        = Gosu::load_tiles("test_image_io/no-alpha-png32.png", 30, 20, Gosu::IF_RETRO);
    ASSERT_GE(tiles.size(), 1);
    for (const Gosu::Image& image : tiles) {
        ASSERT_EQ(image.width(), 30);
        ASSERT_EQ(image.height(), 20);
    }
    // Verify that the second tile looks as expected.
    const Gosu::Bitmap tilemap = Gosu::load_image_file("test_image_io/no-alpha-png32.png");
    Gosu::Bitmap second_tile(30, 20);
    second_tile.insert(tilemap, -30, 0);
    ASSERT_EQ(tiles.at(1).drawable().to_bitmap(), second_tile);
}

TEST_F(ImageTests, load_tiles_from_bitmap)
{
    const Gosu::Bitmap tilemap = Gosu::load_image_file("test_image/from_blob.png");
    const std::vector<Gosu::Image> tiles = Gosu::load_tiles(tilemap, -3, -10);
    ASSERT_EQ(tiles.size(), 3 * 10);
    for (const Gosu::Image& image : tiles) {
        ASSERT_EQ(image.width(), tilemap.width() / 3);
        ASSERT_EQ(image.height(), tilemap.height() / 10);
    }
}