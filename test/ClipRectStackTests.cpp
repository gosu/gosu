#include <gtest/gtest.h>

#include <Gosu/Drawable.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Image.hpp>
#include <Gosu/Transform.hpp>
#include "../src/ClipRectStack.hpp"

class ClipRectStackTests : public testing::Test
{
};

TEST_F(ClipRectStackTests, direct_use)
{
    Gosu::ClipRectStack stack;
    ASSERT_EQ(stack.effective_rect(), std::nullopt);
    ASSERT_THROW(stack.pop(), std::logic_error);

    stack.push(Gosu::Rect { 10, 10, 40, 40 });
    ASSERT_EQ(stack.effective_rect(), Gosu::Rect({ 10, 10, 40, 40 }));

    stack.push(Gosu::Rect { -5, -5, 20, 20 });
    ASSERT_EQ(stack.effective_rect(), Gosu::Rect({ 10, 10, 5, 5 }));

    stack.push(Gosu::Rect { 0, 0, 1, 2 });
    ASSERT_TRUE(stack.effective_rect().value().empty());

    stack.pop();
    ASSERT_EQ(stack.effective_rect(), Gosu::Rect({ 10, 10, 5, 5 }));
}

TEST_F(ClipRectStackTests, not_in_macros)
{
    // Gosu::record does not support clipping, only Gosu::render does.
    ASSERT_THROW(Gosu::record(10, 10, [] { Gosu::clip_to(1, 2, 3, 4, [] {}); }), std::logic_error);
}

TEST_F(ClipRectStackTests, rendering)
{
    Gosu::Bitmap expected(320, 240, Gosu::Color::RED);
    expected.insert(Gosu::Bitmap(100, 80, Gosu::Color::BLACK), 20, 10);

    const Gosu::Image actual = Gosu::render(320, 240, [] {
        Gosu::draw_rect(-100, -100, 1000, 1000, Gosu::Color::RED, 0, Gosu::BM_ADD);
        Gosu::clip_to(20, 10, 100, 80, [] {
            Gosu::draw_rect(-100, -100, 1000, 1000, Gosu::Color::BLUE, 0, Gosu::BM_MULTIPLY);
        });
    });

    ASSERT_EQ(actual.drawable().to_bitmap(), expected);
}

TEST_F(ClipRectStackTests, transforms_and_images)
{
    const Gosu::Image expected = Gosu::Image("test_image_io/no-alpha-png32.png", Gosu::IF_RETRO);

    // Draw an image rotated by -90° within a transform that rotates everything by +90° (a no-op).
    // This tests that the clip_to() call within transform() takes the current transformations into
    // account.
    const Gosu::Image actual = Gosu::render(expected.width(), expected.height(), [&] {
        Gosu::transform(Gosu::Transform::translate(expected.width(), 0), [&] {
            Gosu::transform(Gosu::Transform::rotate(90), [&] {
                Gosu::clip_to(0, 0, expected.height(), expected.width(),
                              [&] { expected.draw_rot(0, expected.width(), 0, -90, 0, 0); });
            });
        });
    });

    ASSERT_EQ(actual.drawable().to_bitmap(), expected.drawable().to_bitmap());
}
