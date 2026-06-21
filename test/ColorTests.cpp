#include <gtest/gtest.h>

#include <Gosu/Color.hpp>
#include <sstream>

namespace Gosu
{

class ColorTests : public testing::Test
{
};

TEST_F(ColorTests, color_creation)
{
    // Explicit construction from RGB.
    EXPECT_EQ(Color::RED, Color(0xff, 0x00, 0x00).with_alpha(255));
    // Implicit construction from ARGB literal.
    EXPECT_EQ(Color::CYAN.with_alpha(128), 0x80'00ffff);
    // Construction from HSV.
    EXPECT_EQ(Color::FUCHSIA, Color::from_hsv(300.0, 1.0, 1.0));
    EXPECT_EQ(Color::YELLOW, Color::from_hsv(60.0, 1.0, 1.0));
    // Double input values are adjusted/clamped.
    EXPECT_EQ(Color::YELLOW, Color::from_hsv(-300.0, 100.0, 255.0));
}

TEST_F(ColorTests, hsv_with_constants)
{
    // Gosu offers colors constants for the three primary colors and their midpoints on the hue
    // spectrum.
    EXPECT_EQ(Color::RED.hue(), 0.0);
    EXPECT_EQ(Color::YELLOW.hue(), 60.0);
    EXPECT_EQ(Color::GREEN.hue(), 120.0);
    EXPECT_EQ(Color::AQUA.hue(), 180.0);
    EXPECT_EQ(Color::BLUE.hue(), 240.0);
    EXPECT_EQ(Color::FUCHSIA.hue(), 300.0);

    // All of these colors are as saturated and bright as possible.
    for (const Color c :
         { Color::RED, Color::YELLOW, Color::GREEN, Color::CYAN, Color::BLUE, Color::FUCHSIA }) {
        EXPECT_EQ(c.saturation(), 1.0);
        EXPECT_EQ(c.value(), 1.0);
    }

    // The brightness (value) of the three shades of gray that Gosu offers are 0.0, ~0.5, 1.0.
    EXPECT_EQ(Color::BLACK.value(), 0.0);
    EXPECT_NEAR(Color::GRAY.value(), 0.5, 0.01);
    EXPECT_EQ(Color::WHITE.value(), 1.0);

    for (const Color c : { Color::NONE, Color::BLACK, Color::GRAY, Color::WHITE }) {
        // Shades of gray do not really have a hue, but 0.0 is the fallback value.
        EXPECT_EQ(c.hue(), 0.0);
        // Shades of gray have no saturation.
        EXPECT_EQ(c.saturation(), 0.0);
    }
}

TEST_F(ColorTests, hsv_roundtrip)
{
    for (int r = 0; r <= 255; r += 5) {
        for (int g = 0; g <= 255; g += 4) {
            for (int b = 0; b <= 255; b += 3) {
                Color color_from_rgb(r, g, b);

                // This is a bit fragile. Color::set_hue cannot really do anything when called on
                // a grayscale Color vlaue, so we need to call these setters in V-S-H order.
                Color color_from_hsv;
                color_from_hsv.set_value(color_from_rgb.value());
                color_from_hsv.set_saturation(color_from_rgb.saturation());
                color_from_hsv.set_hue(color_from_rgb.hue());

                EXPECT_EQ(color_from_rgb.red, color_from_hsv.red);
                EXPECT_EQ(color_from_rgb.green, color_from_hsv.green);
                EXPECT_EQ(color_from_rgb.blue, color_from_hsv.blue);
            }
        }
    }
}

TEST_F(ColorTests, misc_functions)
{
    EXPECT_EQ(0x00'785634, Color(0x12'345678).bgr());

    EXPECT_EQ(0x12'785634, Color(0x12'345678).abgr());

    EXPECT_EQ(0x12'345678, Color(0x12'345678).argb());

    const std::uint32_t rgba = Color(0x12345678).gl();
    const auto* components = reinterpret_cast<const std::uint8_t*>(&rgba);
    EXPECT_EQ(components[0], 0x34);
    EXPECT_EQ(components[1], 0x56);
    EXPECT_EQ(components[2], 0x78);
    EXPECT_EQ(components[3], 0x12);

    EXPECT_EQ(lerp(Color::RED, Color::AQUA), Color::GRAY);
    EXPECT_EQ(multiply(Color::RED, Color::AQUA), Color::BLACK);

    std::ostringstream stream;
    stream << Color(1, 2, 3).with_alpha(4);
    EXPECT_EQ(stream.str(), "0x04'010203");
}

TEST_F(ColorTests, comparison)
{
    EXPECT_EQ(Color::CYAN, Color::AQUA);
    EXPECT_NE(Color::GREEN, Color::BLACK);
    // We order by RGBA -> BLUE must come after RED. (But this is an implementation detail.)
    EXPECT_LT(Color::BLUE, Color::RED);
}

}
