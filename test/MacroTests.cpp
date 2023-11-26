#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Drawable.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <stdexcept>

class MacroTests : public testing::Test
{
};

TEST_F(MacroTests, basic)
{
    const Gosu::Image macro = Gosu::record(320, 240, [] {});
    ASSERT_EQ(macro.width(), 320);
    ASSERT_EQ(macro.height(), 240);
    ASSERT_EQ(macro.drawable().gl_tex_info(), nullptr);
    ASSERT_EQ(macro.drawable().subimage(Gosu::Rect { 0, 0, 1, 1 }), nullptr);
    ASSERT_THROW(macro.drawable().insert(Gosu::Bitmap(), 3, 5), std::logic_error);
    ASSERT_THROW(macro.draw(0, 0, 0), std::logic_error);

    const Gosu::Bitmap bitmap = macro.drawable().to_bitmap();
    ASSERT_EQ(bitmap.width(), 320);
    ASSERT_EQ(bitmap.height(), 240);
    for (int x = 0; x < bitmap.width(); ++x) {
        for (int y = 0; y < bitmap.height(); ++y) {
            ASSERT_EQ(bitmap.pixel(x, y), Gosu::Color::NONE);
        }
    }
}
