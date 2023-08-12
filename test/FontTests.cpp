#include <gtest/gtest.h>

#include <Gosu/Font.hpp>

class FontTests : public testing::Test
{
};

TEST_F(FontTests, text_width)
{
    const Gosu::Font regular_font(23);
    const Gosu::Font bold_font(23, Gosu::default_font_name(), Gosu::FF_BOLD | Gosu::FF_ITALIC);
    ASSERT_EQ(regular_font.text_width(""), 0);
    ASSERT_EQ(regular_font.text_width("\r"), 0);
    ASSERT_EQ(bold_font.text_width(""), 0);
    ASSERT_LE(regular_font.text_width("Afdslkgjd"), bold_font.text_width("Afdslkgjd"));
    ASSERT_LE(regular_font.markup_width("<b>Afdslkgjd</b>"), bold_font.text_width("Afdslkgjd"));
}

TEST_F(FontTests, markup_width)
{
    const Gosu::Font bold_font(7, Gosu::default_font_name(), Gosu::FF_BOLD);
    const Gosu::Font regular_font(7, Gosu::default_font_name(), 0);
    ASSERT_NE(bold_font.text_width("Afdslkgjd"), regular_font.text_width("Afdslkgjd"));
    ASSERT_EQ(bold_font.text_width("Afdslkgjd"), regular_font.markup_width("<b>Afdslkgjd</b>"));
    ASSERT_EQ(bold_font.markup_width("</b>Afdslkgjd"), regular_font.text_width("Afdslkgjd"));
}
