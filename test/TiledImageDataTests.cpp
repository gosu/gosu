#include <gtest/gtest.h>

#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include "../src/TiledImageData.hpp"
#include "Gosu/Bitmap.hpp"
#include <numeric> // for std::iota

class TiledImageDataTests : public testing::Test
{
};

TEST_F(TiledImageDataTests, direct_creation)
{
    Gosu::Buffer buffer(180);
    std::iota(buffer.data(), buffer.data() + buffer.size(), 0);
    Gosu::Bitmap source(9, 5, std::move(buffer));
    Gosu::TiledImageData tiled_data(source, 3, Gosu::IF_SMOOTH);
    ASSERT_EQ(tiled_data.width(), 9);
    ASSERT_EQ(tiled_data.height(), 5);
    ASSERT_EQ(source, tiled_data.to_bitmap());

    // A tiled image does not consist of a single area on a single texture.
    ASSERT_EQ(tiled_data.gl_tex_info(), nullptr);
    // Neither does a subimage that spans multiple tiles.
    ASSERT_EQ(tiled_data.subimage(Gosu::Rect { 2, 2, 2, 2 })->gl_tex_info(), nullptr);
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
        ASSERT_NE(tiled_data.subimage(rect)->gl_tex_info(), nullptr);
    }

    const Gosu::Bitmap red_bitmap(10, 10, Gosu::Color::RED);
    source.insert(red_bitmap, 4, 3);
    tiled_data.insert(red_bitmap, 4, 3);
    ASSERT_EQ(source, tiled_data.to_bitmap());

    ASSERT_THROW(Gosu::TiledImageData(source, 0, Gosu::IF_RETRO), std::invalid_argument);
}

TEST_F(TiledImageDataTests, drawing)
{
    const auto rotated_by_90_degrees = [](const Gosu::Image& image) {
        return Gosu::Graphics::render(image.height(), image.width(), [&] {
            // Draw the image rotated by 90° degrees, rotated around the bottom left corner.
            image.draw_rot(0, 0, 0, 90, 0.0, 1.0);
        });
    };

    const Gosu::Bitmap source_image = Gosu::load_image_file("test_image_io/no-alpha-jpg.jpg");
    Gosu::Image image(source_image);

    // Rotate the image eight times. We should arrive where we started.
    for (int i = 0; i < 8; ++i) {
        // Reload image into a TiledImageData (split it up into tiles).
        Gosu::Bitmap current_bitmap = image.data().to_bitmap();
        // TODO: Why are small tile_sizes (1, 2) so slow? Will a better BinPacker fix this?
        image = Gosu::Image(
            std::make_unique<Gosu::TiledImageData>(current_bitmap, i * 3 + 5, Gosu::IF_TILEABLE));
        // Now render it at a 90° angle.
        image = rotated_by_90_degrees(image);
        Gosu::save_image_file(image.data().to_bitmap(),
                              "/Users/jlnr/Desktop/test" + std::to_string(i) + ".png");
    }

    ASSERT_EQ(source_image, image.data().to_bitmap());
}
