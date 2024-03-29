#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <Gosu/Drawable.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Transform.hpp>
#include <Gosu/Utility.hpp>
#include <Gosu/Window.hpp>
#include <mutex>
#include <thread>

class DrawableTests : public testing::Test
{
};

TEST_F(DrawableTests, empty_drawables)
{
    const Gosu::Bitmap empty(0, 5);
    ASSERT_THROW(Gosu::create_drawable(empty, Gosu::Rect { -5, -2, 5, 2 }, 0),
                 std::invalid_argument);

    const auto empty_drawable = Gosu::create_drawable(empty, Gosu::Rect { 0, 2, 0, 3 }, 0);
    ASSERT_NE(empty_drawable, nullptr);
    ASSERT_EQ(empty_drawable->width(), 0);
    ASSERT_EQ(empty_drawable->height(), 3);
    ASSERT_EQ(empty_drawable->gl_tex_info(), nullptr);
}

TEST_F(DrawableTests, square_power_of_two_texture)
{
    const Gosu::Bitmap source = Gosu::load_image_file("test_image_io/no-alpha-jpg.jpg");

    const auto square_pot_drawable
        = Gosu::create_drawable(source, Gosu::Rect { 5, 3, 64, 64 }, true);
    ASSERT_EQ(square_pot_drawable->width(), 64);
    ASSERT_EQ(square_pot_drawable->height(), 64);
    const auto* square_pot_tex_info = square_pot_drawable->gl_tex_info();
    ASSERT_NE(square_pot_tex_info, nullptr);
    // Because this is the only image on the texture, it spans the full u/v range.
    ASSERT_EQ(square_pot_tex_info->left, 0);
    ASSERT_EQ(square_pot_tex_info->top, 0);
    ASSERT_EQ(square_pot_tex_info->right, 1);
    ASSERT_EQ(square_pot_tex_info->bottom, 1);
}

TEST_F(DrawableTests, texture_allocation)
{
    const Gosu::Bitmap source = Gosu::load_image_file("test_image_io/no-alpha-jpg.jpg");

    const auto normal_drawable = Gosu::create_drawable(source, Gosu::Rect::covering(source), 0);
    ASSERT_EQ(normal_drawable->width(), source.width());
    ASSERT_EQ(normal_drawable->height(), source.height());
    ASSERT_EQ(source, normal_drawable->to_bitmap());
    const auto* normal_tex_info = normal_drawable->gl_tex_info();
    // This image will be allocated as part of a larger texture atlas because its size is not good
    // for putting it onto a dedicated OpenGL texture (not a power of two).
    ASSERT_NE(normal_tex_info, nullptr);
    ASSERT_LT(normal_tex_info->bottom, 0.5);
    ASSERT_LT(normal_tex_info->right, 0.5);

    const auto retro_drawable
        = Gosu::create_drawable(source, Gosu::Rect::covering(source), Gosu::IF_RETRO);
    ASSERT_EQ(retro_drawable->width(), source.width());
    ASSERT_EQ(retro_drawable->height(), source.height());
    ASSERT_EQ(source, retro_drawable->to_bitmap());
    const auto* retro_tex_info = retro_drawable->gl_tex_info();
    ASSERT_NE(retro_tex_info, nullptr);
    ASSERT_LT(retro_tex_info->bottom, 0.5);
    ASSERT_LT(retro_tex_info->right, 0.5);
    // Because "retro" images require different OpenGL texture settings, both drawables must have
    // been allocated on different texture atlases.
    ASSERT_NE(normal_tex_info->tex_name, retro_tex_info->tex_name);

    const auto second_retro_drawable
        = Gosu::create_drawable(source, Gosu::Rect { 0, 0, 5, 2 }, Gosu::IF_RETRO);
    // A second retro drawable should end up on the same texture atlas as the first one.
    ASSERT_EQ(retro_tex_info->tex_name, second_retro_drawable->gl_tex_info()->tex_name);
}

TEST_F(DrawableTests, large_texture_allocation)
{
    // This bitmap can only be represented by a tiled drawable because it exceeds MAX_TEXTURE_SIZE.
    const Gosu::Bitmap red(Gosu::MAX_TEXTURE_SIZE + 5, Gosu::MAX_TEXTURE_SIZE * 3, 0xff'ff0000);
    auto tiled_drawable
        = Gosu::create_drawable(red, Gosu::Rect::covering(red), Gosu::IF_TILEABLE_TOP);

    const Gosu::Image render_result = Gosu::render(100, 100, [&] {
        // Scale this image by factor 10 so that we can verify that it has a tileable top (only).
        Gosu::transform(Gosu::Transform::scale(10),
                        [&] { Gosu::Image(std::move(tiled_drawable)).draw(0, 0, 0); });
    });
    const Gosu::Bitmap rendered_bitmap = render_result.drawable().to_bitmap();
    for (int x = 0; x < 100; ++x) {
        for (int y = 0; y < 100; ++y) {
            // We expect that the left side of the rendered image will fade out (not tileable), so
            // verify that the leftmost pixels are not pure red. Everything at x=10 must be fully
            // red though, especially the pixels at the top (IF_TILEABLE_TOP!).

            if (x < 5) {
                ASSERT_NE(rendered_bitmap.pixel(x, y), Gosu::Color::RED);
            }
            else if (x >= 10) {
                ASSERT_EQ(rendered_bitmap.pixel(x, y), Gosu::Color::RED);
            }
        }
    }
}

// Gosu is not actually ready for multithreading yet, but it turns out that Ruby's garbage collector
// happily tries to delete images and other objects from background threads. This test verifies that
// we don't outright crash when this happens.
TEST_F(DrawableTests, multithreaded_stress_test)
{
    // Gosu does not support directly creating images from background threads because it wants to
    // create an SDL2 window first, and at least macOS requires this to happen on the main thread.
    // -> Create a temporary window on the test thread (main thread) just to set things up.
    Gosu::Window(100, 100); // NOLINT(*-unused-raii)

    constexpr int num_threads = 5;
    constexpr int num_drawables_per_thread = 500;

    std::vector<std::unique_ptr<Gosu::Drawable>> drawables;
    std::mutex drawables_mutex;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&] {
            try {
                for (int j = 0; j < num_drawables_per_thread; ++j) {
                    const Gosu::Bitmap source(j * 11 % 1234, (j + 4) * 7 % 1234, Gosu::Color::RED);
                    auto drawable = Gosu::create_drawable(source, Gosu::Rect::covering(source), 0);

                    std::scoped_lock lock(drawables_mutex);
                    drawables.push_back(std::move(drawable));
                    if (j % 3) {
                        const int index_to_delete = j % static_cast<int>(drawables.size());
                        drawables.erase(drawables.begin() + index_to_delete);
                    }
                }
            } catch (const std::exception& e) {
                GTEST_FAIL() << e.what();
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Verify at least one of the drawables created by the other threads.
    for (const auto& drawable : drawables) {
        if (drawable->width() > 0 && drawable->height() > 0) {
            ASSERT_EQ(drawable->to_bitmap().pixel(0, 0), Gosu::Color::RED);
            return;
        }
    }
    GTEST_SKIP_("All Drawables were empty");
}
