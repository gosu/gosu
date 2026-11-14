#include <gtest/gtest.h>

#include "TestHelper.hpp"
#include <Gosu/Bitmap.hpp>
#include <Gosu/Drawable.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Transform.hpp>
#include <functional>
#include <stdexcept>

class MacroTests : public testing::Test
{
protected:
    static constexpr int W = 40, H = 30;

    // Draw operations that exercise different paths: quads, triangles, gradients, z-order, and
    // transforms.
    static void draw_scene()
    {
        Gosu::draw_rect(0, 0, W, H, Gosu::Color::GRAY, 0);
        Gosu::draw_rect(5, 5, 10, 10, Gosu::Color::RED, 2);
        Gosu::draw_rect(8, 8, 10, 10, Gosu::Color::BLUE, 1); // Drawn first despite coming later.
        Gosu::draw_triangle(20, 5, Gosu::Color::GREEN, 35, 5, Gosu::Color::BLUE, //
                            20, 25, Gosu::Color::YELLOW, 3);
        Gosu::transform(Gosu::Transform::translate(25, 18), [] {
            Gosu::draw_rect(0, 0, 8, 8, Gosu::Color::CYAN, 4);
        });
    }

    // Macros are drawn via Gosu::gl, which needs a current viewport; Gosu::render alone has none.
    static Gosu::Bitmap render_to_bitmap(int width, int height, const std::function<void()>& f)
    {
        Gosu::Viewport viewport(width, height);
        const auto handle = viewport.make_current();
        return Gosu::render(width, height, f).drawable().to_bitmap();
    }
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

TEST_F(MacroTests, looks_like_direct_drawing)
{
    const Gosu::Image macro = Gosu::record(W, H, draw_scene);
    const Gosu::Bitmap expected = render_to_bitmap(W, H, draw_scene);

    EXPECT_TRUE(visible_pixels_are_equal(render_to_bitmap(W, H, [&] { macro.draw(0, 0, 0); }),
                                         expected, 2));
    EXPECT_TRUE(visible_pixels_are_equal(macro.drawable().to_bitmap(), expected, 2));
}

TEST_F(MacroTests, scaled_and_inside_transform)
{
    const Gosu::Image macro = Gosu::record(W, H, draw_scene);
    const auto placed = [](const std::function<void()>& f) {
        return [f] {
            Gosu::transform(Gosu::Transform::translate(7, 3), [&] {
                Gosu::transform(Gosu::Transform::scale(2), f);
            });
        };
    };

    const Gosu::Bitmap expected = render_to_bitmap(2 * W + 10, 2 * H + 10, placed(draw_scene));
    // Allow a few differences along rasterized edges.
    const int max_differences = 2 * (W + H);

    const Gosu::Bitmap via_transform
        = render_to_bitmap(2 * W + 10, 2 * H + 10, placed([&] { macro.draw(0, 0, 0); }));
    EXPECT_TRUE(visible_pixels_are_equal(via_transform, expected, 2, max_differences));

    const Gosu::Bitmap via_arguments
        = render_to_bitmap(2 * W + 10, 2 * H + 10, [&] { macro.draw(7, 3, 0, 2, 2); });
    EXPECT_TRUE(visible_pixels_are_equal(via_arguments, expected, 2, max_differences));
}

TEST_F(MacroTests, corners_map_to_target_quad)
{
    // Each quadrant has its own color, so we can tell where each corner of the macro ends up.
    const Gosu::Image macro = Gosu::record(20, 20, [] {
        Gosu::draw_rect(0, 0, 10, 10, Gosu::Color::RED, 0);
        Gosu::draw_rect(10, 0, 10, 10, Gosu::Color::GREEN, 0);
        Gosu::draw_rect(0, 10, 10, 10, Gosu::Color::BLUE, 0);
        Gosu::draw_rect(10, 10, 10, 10, Gosu::Color::YELLOW, 0);
    });

    // Top edge: (0, 0) to (40, 10). Bottom edge: (0, 40) to (40, 30).
    // The right edge is vertical (x2 == x4), see https://github.com/gosu/gosu/issues/571.
    const Gosu::Bitmap result = render_to_bitmap(40, 40, [&] {
        macro.drawable().draw(0, 0, Gosu::Color::WHITE, 40, 10, Gosu::Color::WHITE, //
                              0, 40, Gosu::Color::WHITE, 40, 30, Gosu::Color::WHITE, //
                              0, Gosu::BM_DEFAULT);
    });

    EXPECT_EQ(result.pixel(1, 1), Gosu::Color::RED);
    EXPECT_EQ(result.pixel(38, 10), Gosu::Color::GREEN);
    EXPECT_EQ(result.pixel(1, 38), Gosu::Color::BLUE);
    EXPECT_EQ(result.pixel(38, 29), Gosu::Color::YELLOW);
    // Outside the trapezoid, above the top edge and below the bottom edge.
    EXPECT_EQ(result.pixel(38, 2), Gosu::Color::NONE);
    EXPECT_EQ(result.pixel(38, 35), Gosu::Color::NONE);
}

TEST_F(MacroTests, cannot_be_tinted)
{
    const Gosu::Image macro = Gosu::record(10, 10, [] {});
    ASSERT_THROW(render_to_bitmap(10, 10, [&] { macro.draw(0, 0, 0, 1, 1, Gosu::Color::RED); }),
                 std::invalid_argument);
}

TEST_F(MacroTests, no_custom_opengl)
{
    ASSERT_THROW(Gosu::record(10, 10, [] { Gosu::gl([] {}); }), std::logic_error);
}

TEST_F(MacroTests, no_lines)
{
    ASSERT_THROW(Gosu::record(10, 10, [] {
        Gosu::draw_line(0, 0, Gosu::Color::WHITE, 10, 10, Gosu::Color::WHITE, 0);
    }), std::logic_error);
}

TEST_F(MacroTests, exception_while_recording)
{
    ASSERT_THROW(Gosu::record(10, 10, [] { throw std::runtime_error("test"); }),
                 std::runtime_error);
    // The macro's queue must have been removed again.
    ASSERT_THROW(Gosu::flush(), std::logic_error);
    // ...and recording must still work.
    ASSERT_EQ(Gosu::record(10, 10, [] {}).width(), 10);
}

TEST_F(MacroTests, degenerate_targets)
{
    const Gosu::Image macro = Gosu::record(10, 10, [] {
        Gosu::draw_rect(0, 0, 10, 10, Gosu::Color::RED, 0);
    });
    const Gosu::Bitmap result = render_to_bitmap(10, 10, [&] {
        // All four corners in the same place.
        macro.drawable().draw(5, 5, Gosu::Color::WHITE, 5, 5, Gosu::Color::WHITE, //
                              5, 5, Gosu::Color::WHITE, 5, 5, Gosu::Color::WHITE, //
                              0, Gosu::BM_DEFAULT);
        Gosu::record(0, 0, [] {}).draw(0, 0, 0);
    });
    // -> Nothing is rendered.
    EXPECT_TRUE(visible_pixels_are_equal(result, Gosu::Bitmap(10, 10)));
}

TEST_F(MacroTests, temporary_macro)
{
    // The macro is destroyed long before the queue is flushed at the end of Gosu::render.
    // This is a regression test for a previous bug where the macro vertices were not being kept
    // alive by a shared_ptr.
    const Gosu::Bitmap result = render_to_bitmap(W, H, [] {
        Gosu::record(W, H, draw_scene).draw(0, 0, 0);
    });
    EXPECT_TRUE(visible_pixels_are_equal(result, render_to_bitmap(W, H, draw_scene), 2));
}
