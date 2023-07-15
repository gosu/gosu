#include <gtest/gtest.h>
#include <Gosu/Color.hpp>

namespace Gosu
{
    class ColorTests : public testing::Test
    {
    };

    TEST_F(ColorTests, ColorCreation)
    {
        EXPECT_EQ(Color::CYAN, Color{0xff'00ffff});
        EXPECT_EQ(Color::RED, Color(0xff, 0x00, 0x00).with_alpha(255));
        EXPECT_EQ(Color::FUCHSIA, Color::from_hsv(300.0, 1.0, 1.0));
        EXPECT_EQ(Color::YELLOW, Color::from_hsv(60.0, 1.0, 1.0));
        // Double input values are be adjusted/clamped.
        EXPECT_EQ(Color::YELLOW, Color::from_hsv(-300.0, 100.0, 255.0));
    }

    TEST_F(ColorTests, ColorHSV)
    {
        // Gosu offers colors constants for the three primary colors and their midpoints on the hue
        // spectrum.
        EXPECT_EQ(Color::RED.hue(), 0.0);
        EXPECT_EQ(Color::YELLOW.hue(), 60.0);
        EXPECT_EQ(Color::GREEN.hue(), 120.0);
        EXPECT_EQ(Color::AQUA.hue(), 180.0);
        EXPECT_EQ(Color::BLUE.hue(), 240.0);
        EXPECT_EQ(Color::FUCHSIA.hue(), 300.0);

        // All of these colors must be as saturated and bright as possible.
        for (const Color c :
             {Color::RED, Color::YELLOW, Color::GREEN, Color::CYAN, Color::BLUE, Color::FUCHSIA}) {
            EXPECT_EQ(c.saturation(), 1.0);
            EXPECT_EQ(c.value(), 1.0);
        }

        // The brightness (value) of the three shades of gray that Gosu offers are 0.0, ~0.5, 1.0.
        EXPECT_EQ(Color::BLACK.value(), 0.0);
        EXPECT_NEAR(Color::GRAY.value(), 0.5, 0.01);
        EXPECT_EQ(Color::WHITE.value(), 1.0);

        for (const Color c : {Color::NONE, Color::BLACK, Color::GRAY, Color::WHITE}) {
            // Shades of gray do not really have a hue, but 0.0 is the fallback value.
            EXPECT_EQ(c.hue(), 0.0);
            // Shades of gray have no saturation.
            EXPECT_EQ(c.saturation(), 0.0);
        }
    }

    TEST_F(ColorTests, ComparisonOperators)
    {
        EXPECT_EQ(Color::CYAN, Color::AQUA);
        EXPECT_NE(Color::GREEN, Color::BLACK);
        // We order by RGBA -> BLUE must come after RED. (But this is an implementation detail.)
        EXPECT_LT(Color::BLUE, Color::RED);
    }
}
