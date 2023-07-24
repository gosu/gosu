#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/ImageData.hpp>

class ImageTests : public testing::Test
{
};

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
    Gosu::Image image(Gosu::Graphics::create_image(bitmap, Gosu::Rect::covering(bitmap), true));

    const Gosu::Image result = Gosu::Graphics::render(300, 300, [&] {
        Gosu::Graphics::draw_rect(0, 0, 300, 300, Gosu::Color::YELLOW, 0);
        image.draw(0, 0, 1, 100, 100);
    });

    Gosu::Bitmap result_bitmap = result.data().to_bitmap();
    for (int y = 0; y < result_bitmap.height(); ++y) {
        for (int x = 0; x < result_bitmap.width(); ++x) {
            // TODO: Is it normal/expected that render-ed images have alpha<255 in some places?
            result_bitmap.pixel(x, y).alpha = 255;
            ASSERT_EQ(result_bitmap.pixel(x, y), Gosu::Color::YELLOW);
        }
    }
}
