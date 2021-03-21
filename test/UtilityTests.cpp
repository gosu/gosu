#include <gtest/gtest.h>
#include <Gosu/Utility.hpp>

namespace Gosu
{
    class UtilityTests : public testing::Test
    {
    };

    TEST_F(UtilityTests, HasExtension)
    {
        ASSERT_TRUE(has_extension("filename.bmp", ".bmp"));
        ASSERT_TRUE(has_extension("File.Png", "PNG"));
        ASSERT_FALSE(has_extension("Image.bmp", "png"));

        for (const std::string& language : Gosu::user_languages()) {
            ASSERT_EQ(language.length(), 5);
        }
    }
}
