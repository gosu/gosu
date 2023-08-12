#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include "../src/TiledDrawable.hpp"
#include <numeric> // for std::iota

class TiledDrawableTests : public testing::Test
{
};

TEST_F(TiledDrawableTests, direct_creation)
{
    Gosu::Buffer buffer(180);
    std::iota(buffer.data(), buffer.data() + buffer.size(), 0);
    Gosu::Bitmap source(9, 5, std::move(buffer));
    Gosu::TiledDrawable tiled_drawable(source, Gosu::Rect::covering(source), 3, Gosu::IF_SMOOTH);
    ASSERT_EQ(tiled_drawable.width(), 9);
    ASSERT_EQ(tiled_drawable.height(), 5);
    ASSERT_EQ(source, tiled_drawable.to_bitmap());

    // A tiled image does not consist of a single area on a single texture.
    ASSERT_EQ(tiled_drawable.gl_tex_info(), nullptr);
    // Neither does a subimage that spans multiple tiles.
    ASSERT_EQ(tiled_drawable.subimage(Gosu::Rect { 2, 2, 2, 2 })->gl_tex_info(), nullptr);
    // These are the rectangles we expect to see:
    for (const auto& rect : {
             Gosu::Rect { 0, 0, 3, 3 },
             Gosu::Rect { 3, 0, 3, 3 },
             Gosu::Rect { 6, 0, 3, 3 },
             Gosu::Rect { 0, 3, 3, 2 },
             Gosu::Rect { 3, 3, 3, 2 },
             Gosu::Rect { 6, 3, 3, 2 },
         }) {
        // The individual tiles are rectangles on an OpenGL texture and can give us information.
        ASSERT_NE(tiled_drawable.subimage(rect)->gl_tex_info(), nullptr);
    }

    const Gosu::Bitmap other_bitmap = Gosu::load_image_file("test_image_io/alpha-bmp24.bmp");
    source.insert(other_bitmap, 4, 1);
    tiled_drawable.insert(other_bitmap, 4, 1);
    ASSERT_EQ(source, tiled_drawable.to_bitmap());

    ASSERT_THROW(Gosu::TiledDrawable(source, Gosu::Rect { 1, 2, 3, 4 }, 10, Gosu::IF_RETRO),
                 std::invalid_argument);
    ASSERT_THROW(Gosu::TiledDrawable(source, Gosu::Rect::covering(source), -6, Gosu::IF_RETRO),
                 std::invalid_argument);
}

TEST_F(TiledDrawableTests, drawing)
{
    const auto rotated_by_90_degrees = [](const Gosu::Image& image) {
        return Gosu::render(image.height(), image.width(), [&] {
            // Draw the image rotated clockwise by 90°, rotated around the bottom left corner.
            image.draw_rot(0, 0, 0, 90, 0.0, 1.0);
        });
    };

    const Gosu::Bitmap source_image = Gosu::load_image_file("test_image_io/no-alpha-jpg.jpg");
    Gosu::Image image(source_image);

    // Rotate the image eight times. We should arrive where we started.
    for (int i = 0; i < 8; ++i) {
        // Reload image into a TiledDrawable (split it up into tiles).
        const Gosu::Bitmap current_bitmap = image.drawable().to_bitmap();
        image = Gosu::Image(std::make_unique<Gosu::TiledDrawable>(
            current_bitmap, Gosu::Rect::covering(current_bitmap), i + 1, Gosu::IF_TILEABLE));
        // Now rotate it clockwise by 90°.
        image = rotated_by_90_degrees(image);
    }

    ASSERT_EQ(source_image, image.drawable().to_bitmap());
}
