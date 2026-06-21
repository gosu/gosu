#include <gtest/gtest.h>

#include <Gosu/Directories.hpp>
#include <filesystem>

class DirectoriesTests : public testing::Test
{
};

TEST_F(DirectoriesTests, use_resource_directory)
{
    const auto old_cwd = std::filesystem::current_path();
    std::filesystem::current_path("/");
    Gosu::use_resource_directory();
    const auto new_cwd = std::filesystem::current_path();
    ASSERT_EQ(new_cwd, std::filesystem::canonical(Gosu::resource_path()));
    // Restore whatever was the previous path.
    std::filesystem::current_path(old_cwd);
}

TEST_F(DirectoriesTests, resource_path)
{
    const std::string directory = Gosu::resource_path();
    ASSERT_GT(directory.length(), 0);
    ASSERT_EQ(directory.back(), '/');
    ASSERT_TRUE(std::filesystem::exists(directory));

    const std::string filename = Gosu::resource_path("foo/bar.png");
    ASSERT_EQ(directory + "foo/bar.png", filename);
    ASSERT_FALSE(std::filesystem::exists(filename));
}

TEST_F(DirectoriesTests, user_settings_path)
{
    const std::string directory = Gosu::user_settings_path("", "");
    ASSERT_GT(directory.length(), 0);
    ASSERT_EQ(directory.back(), '/');

    const std::string filename = Gosu::user_settings_path("", "", "foo/bar.sav");
    ASSERT_EQ(directory + "foo/bar.sav", filename);
    ASSERT_FALSE(std::filesystem::exists(filename));
}
