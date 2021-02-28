#include <Gosu/Font.hpp>
#include <gtest/gtest.h>

namespace Gosu
{
    class FontTests : public testing::Test
    {
    };

    TEST_F(FontTests, TextWidth)
    {
        Font regular_font{23, default_font_name()};
        Font bold_font{23, default_font_name(), FF_BOLD | FF_ITALIC};
        ASSERT_NE(regular_font.text_width("Afdslkgjd"), bold_font.text_width("Afdslkgjd"));
    }
}