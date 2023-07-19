#include <gtest/gtest.h>

#include <Gosu/Utility.hpp>

class UtilityTests : public testing::Test
{
};

TEST_F(UtilityTests, utf8_to_composed_utc4)
{
    ASSERT_EQ(Gosu::utf8_to_composed_utc4(""), std::u32string());
    // Make sure that this decomposed 'Ä' is converted to a single codepoint.
    ASSERT_EQ(Gosu::utf8_to_composed_utc4("BA\xcc\x88R"), std::u32string({ 'B', 0xc4, 'R' }));
    // Verify that invalid UTF-8 is ignored as expected.
    ASSERT_EQ(Gosu::utf8_to_composed_utc4("BA\xccR"), std::u32string({'B', 'A', 'R'}));
}

TEST_F(UtilityTests, has_extension)
{
    ASSERT_TRUE(Gosu::has_extension("filename.bmp", ".bmp"));
    ASSERT_TRUE(Gosu::has_extension("File.Png", "PNG"));
    ASSERT_FALSE(Gosu::has_extension("Image.bmp", "png"));
    ASSERT_FALSE(Gosu::has_extension("ng", ".png"));
}

TEST_F(UtilityTests, user_languages)
{
    for (const std::string& language : Gosu::user_languages()) {
        ASSERT_GE(language.length(), 2);
    }
}
