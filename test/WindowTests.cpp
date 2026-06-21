#include <gtest/gtest.h>

#include <Gosu/Window.hpp>

class WindowTests : public testing::Test{};

TEST_F(WindowTests, resolution_without_window)
{
    const int screen_width = Gosu::screen_width();
    ASSERT_GT(screen_width, 0);
    const int screen_height = Gosu::screen_height();
    ASSERT_GT(screen_height, 0);
    const int available_width = Gosu::available_width();
    ASSERT_GT(available_width, 0);
    ASSERT_LE(available_width, screen_width);
    const int available_height = Gosu::available_height();
    ASSERT_GT(available_height, 0);
    ASSERT_LE(available_height, screen_height);
}

TEST_F(WindowTests, resolution_with_window)
{
    const Gosu::Window window(100, 100, Gosu::WF_BORDERLESS | Gosu::WF_RESIZABLE);

    const int screen_width = Gosu::screen_width(&window);
    ASSERT_GT(screen_width, 0);
    const int screen_height = Gosu::screen_height(&window);
    ASSERT_GT(screen_height, 0);
    const int available_width = Gosu::available_width(&window);
    ASSERT_GT(available_width, 0);
    ASSERT_LE(available_width, screen_width);
    const int available_height = Gosu::available_height(&window);
    ASSERT_GT(available_height, 0);
    ASSERT_LE(available_height, screen_height);

}
