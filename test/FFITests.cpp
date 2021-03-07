#include <Gosu/Utility.hpp>
#include "../ffi/Gosu.h"
#include <gtest/gtest.h>

namespace Gosu
{
    class FFITests : public testing::Test
    {
    };

    TEST_F(FFITests, UserLanguages)
    {
        std::vector<std::string> ffi_languages;

        Gosu_user_languages([](void* context, const char* language) {
            static_cast<decltype(ffi_languages)*>(context)->emplace_back(language);
        }, &ffi_languages);

        ASSERT_EQ(ffi_languages, Gosu::user_languages());
        ASSERT_EQ(nullptr, Gosu_last_error());
    }

    TEST_F(FFITests, ImageNotFoundError)
    {
        Gosu_Image* image = Gosu_Image_create("/does/not/exist", 0);
        ASSERT_EQ(nullptr, image);
        ASSERT_NE(nullptr, Gosu_last_error());
        // Confirm that the error message is helpful and contains the path that could not be opened.
        const std::string error_message = Gosu_last_error();
        ASSERT_NE(std::string::npos, error_message.find("/does/not/exist"));
    }
}