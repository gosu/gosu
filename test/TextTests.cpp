#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Text.hpp>
#include "TestHelper.hpp"

class TextTests : public testing::Test
{
public:
    /// Same as in test_text.rb: Makes the text in the "text" test case red so that it can be seen
    /// more clearly when viewed in a file browser with a white background.
    static Gosu::Bitmap make_red(Gosu::Bitmap bitmap)
    {
        for (int x = 0; x < bitmap.width(); ++x) {
            for (int y = 0; y < bitmap.height(); ++y) {
                bitmap.pixel(x, y).green = 0;
                bitmap.pixel(x, y).blue = 0;
            }
        }
        return bitmap;
    }
};

TEST_F(TextTests, edge_cases)
{
    ASSERT_THROW(Gosu::text_width(U"negative font_height", "Arial", -5), std::invalid_argument);
    ASSERT_THROW(Gosu::text_width(U"invalid flags", "Arial", 34, 255), std::invalid_argument);

    Gosu::Bitmap bitmap;
    ASSERT_THROW(Gosu::draw_text(bitmap, 0, 0, Gosu::Color::RED, //
                                 U"font_height == 0", "Arial", 0),
                 std::invalid_argument);
    ASSERT_THROW(Gosu::draw_text(bitmap, 0, 0, Gosu::Color::GREEN, //
                                 U"invalid flags", "Arial", 34, 17),
                 std::invalid_argument);

    ASSERT_THROW(Gosu::layout_markup("negative font_height", "Arial", -5), std::invalid_argument);
    ASSERT_THROW(Gosu::layout_markup("invalid flags", "Arial", 34, 0, -1, Gosu::AL_CENTER, 255),
                 std::invalid_argument);
    ASSERT_THROW(
        Gosu::layout_markup("line_spacing too small", "Arial", 34, -35, 100, Gosu::AL_CENTER, 255),
        std::invalid_argument);

    const Gosu::Bitmap empty_text = Gosu::layout_markup("", Gosu::default_font_name(), 20);
    ASSERT_EQ(empty_text.width(), 0);
    ASSERT_EQ(empty_text.height(), 0);
}

TEST_F(TextTests, text)
{
    const std::string text_with_whitespace = "$ ls\n  .\t..\tfoo\r\n  bar\tqux        ";

    const Gosu::Bitmap right_aligned = make_red(
        Gosu::layout_text(text_with_whitespace, "media/daniel.ttf", 41, 0, 139, Gosu::AL_RIGHT));
    const Gosu::Bitmap right_aligned_expected
        = Gosu::load_image_file("test_text/text-whitespace-0.png");
    ASSERT_TRUE(visible_pixels_are_equal(right_aligned, right_aligned_expected));

    const Gosu::Bitmap centered = make_red(
        Gosu::layout_text(text_with_whitespace, "media/daniel.otf", 41, 10, -1, Gosu::AL_CENTER));
    const Gosu::Bitmap centered_expected = Gosu::load_image_file("test_text/text-whitespace-1.png");
    ASSERT_TRUE(visible_pixels_are_equal(centered, centered_expected));
}

TEST_F(TextTests, markup)
{
    const std::string markup
        = "<c=ff0000><b>Bold, <u>underlined &amp; <i>italic. <c=4400ff>How <c=0f3>about</c> "
          "colors?</c></i></u>&lt;&gt;</b>";

    const Gosu::Bitmap right_aligned
        = Gosu::layout_markup(markup, "media/daniel.ttf", 41, 0, 139, Gosu::AL_RIGHT);
    const Gosu::Bitmap right_aligned_expected
        = Gosu::load_image_file("test_text/text-markup-0.png");
    ASSERT_TRUE(visible_pixels_are_equal(right_aligned, right_aligned_expected));

    const Gosu::Bitmap centered
        = Gosu::layout_markup(markup, "media/daniel.otf", 41, 10, -1, Gosu::AL_CENTER);
    const Gosu::Bitmap centered_expected //
        = Gosu::load_image_file("test_text/text-markup-1.png");
    ASSERT_TRUE(visible_pixels_are_equal(centered, centered_expected));
}
