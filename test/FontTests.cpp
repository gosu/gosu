#include <gtest/gtest.h>

#include <Gosu/Drawable.hpp>
#include <Gosu/Font.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include "TestHelper.hpp"

class FontTests : public testing::Test
{
};

TEST_F(FontTests, text_width)
{
    const Gosu::Font regular_font(12);
    ASSERT_EQ(regular_font.flags(), 0);
    ASSERT_EQ(regular_font.image_flags(), 0);
    const Gosu::Font bold_font(12, Gosu::default_font_name(), Gosu::FF_BOLD | Gosu::FF_ITALIC);
    ASSERT_EQ(bold_font.flags(), Gosu::FF_BOLD | Gosu::FF_ITALIC);
    ASSERT_EQ(regular_font.name(), bold_font.name());

    ASSERT_EQ(regular_font.text_width(""), 0);
    ASSERT_EQ(regular_font.text_width("\r"), 0);
    ASSERT_EQ(bold_font.text_width(""), 0);
    ASSERT_LE(regular_font.text_width("Afdslkgjd"), bold_font.text_width("Afdslkgjd"));
    ASSERT_LE(regular_font.markup_width("<b>Afdslkgjd</b>"), bold_font.text_width("Afdslkgjd"));

    // This is a character which we expect to be wider than it is high (special code path).
    // (It's possible that this will fail on other platforms in the future. At least with Arial
    // Unicode MS, it's an almost square letter.)
    ASSERT_GT(regular_font.text_width("‱"), regular_font.height());
}

TEST_F(FontTests, markup_width)
{
    const Gosu::Font bold_font(7, Gosu::default_font_name(), Gosu::FF_BOLD);
    const Gosu::Font regular_font(7, Gosu::default_font_name(), 0);
    ASSERT_NE(bold_font.text_width("Afdslkgjd"), regular_font.text_width("Afdslkgjd"));
    ASSERT_EQ(bold_font.text_width("Afdslkgjd"), regular_font.markup_width("<b>Afdslkgjd</b>"));
    ASSERT_EQ(bold_font.markup_width("</b>Afdslkgjd"), regular_font.text_width("Afdslkgjd"));
}

TEST_F(FontTests, draw_text_vs_draw_markup)
{
    const Gosu::Font font(10, "media/daniel.otf");
    const Gosu::Image white_text = Gosu::render(100, 100, [&] {
        font.draw_text_rel("Hallo Welt! 遊戲寫完了沒？", 50, 50, 0, 0.5, 0.5);
    });
    const Gosu::Image green_text = Gosu::render(100, 100, [&] {
        font.draw_markup_rel("<c=0f0>Hallo Welt! 遊戲寫完了沒？</c>", 50, 50, 0, 0.5, 0.5);
    });
    // If we strip the red and blue channels from the normal text...
    Gosu::Bitmap bitmap = white_text.drawable().to_bitmap();
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            bitmap.pixel(x, y).red = 0;
            bitmap.pixel(x, y).blue = 0;
        }
    }
    // ...it should be 100% equal to the green text.
    ASSERT_TRUE(visible_pixels_are_equal(bitmap, green_text.drawable().to_bitmap()));
}

TEST_F(FontTests, draw_markup)
{
    const Gosu::Font font(10, "media/daniel.otf");

    const Gosu::Image result = Gosu::render(200, 100, [&] {
        // Draw this string repeatedly to make up for opacity differences in OpenGL renderers.
        for (int i = 0; i < 255; ++i) {
            font.draw_markup("Hi! <c=f00>Red.\r\nNew   line! Äöß\n", 5, 5, -6, 2.0, 4.0,
                             Gosu::Color::FUCHSIA, Gosu::BM_ADD);
        }
    });
    ASSERT_TRUE(visible_pixels_are_equal(result.drawable().to_bitmap(),
                                         Gosu::load_image_file("test_font/draw_markup.png"), //
                                         0, 10));
}

TEST_F(FontTests, draw_markup_rel)
{
    const Gosu::Font font(10, "media/daniel.otf");

    const Gosu::Image result = Gosu::render(100, 100, [&] {
        // Draw this string repeatedly to make up for opacity differences in OpenGL renderers.
        for (int i = 0; i < 255; ++i) {
            font.draw_markup_rel("<c=000>I &lt;3 Ruby/Gosu!\n", 50, 50, 5, 0.4, -2);
        }
    });
    ASSERT_TRUE(visible_pixels_are_equal(result.drawable().to_bitmap(),
                                         Gosu::load_image_file("test_font/draw_markup_rel.png"), //
                                         10, 0));
}

TEST_F(FontTests, set_image)
{
    Gosu::Font font(10, "media/daniel.otf", 0, Gosu::IF_RETRO);

    ASSERT_THROW(font.set_image("Multiple codepoints", Gosu::Image()), std::invalid_argument);

    ASSERT_GT(font.text_width("ßßß"), 0);

    // Replace the glyph for "ß" by an empty image.
    font.set_image("ß", Gosu::Image());
    ASSERT_EQ(font.text_width("ßßß"), 0);
}
