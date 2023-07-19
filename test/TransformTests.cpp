#include <gtest/gtest.h>

#include <Gosu/Transform.hpp>

class TransformTests : public testing::Test
{
public:
    static constexpr double EPSILON = 0.001;
};

TEST_F(TransformTests, translation)
{
    double x = 10, y = 20;
    Gosu::Transform::translate(50, -100).apply(x, y);
    ASSERT_EQ(x, 60);
    ASSERT_EQ(y, -80);
}

TEST_F(TransformTests, rotation)
{
    // A point in the bottom left area rotated by 90° -> a point in the upper left area.
    double x = -10, y = 10;
    Gosu::Transform::rotate(90).apply(x, y);
    ASSERT_EQ(x, -10);
    ASSERT_EQ(y, -10);

    // If the result is then rotated around a point above it, it will move even further up.
    Gosu::Transform::rotate(90).around(0, -20).apply(x, y);
    ASSERT_EQ(x, -10);
    ASSERT_EQ(y, -30);

    // A point rotated around itself is still the same point.
    Gosu::Transform::rotate(123).around(x, y).apply(x, y);
    ASSERT_EQ(x, -10);
    ASSERT_EQ(x, -30);
}

TEST_F(TransformTests, scale)
{
    // Shrink a point towards the origin.
    double x = 10, y = 10;
    Gosu::Transform::scale(0.5, 0.3).apply(x, y);
    ASSERT_NEAR(x, 5, EPSILON);
    ASSERT_NEAR(y, 3, EPSILON);
    // Now expand it around an origin on the right.
    Gosu::Transform::scale(10).around(10, 3).apply(x, y);
    ASSERT_NEAR(x, -45, EPSILON);
    ASSERT_NEAR(y, 3, EPSILON);
}

TEST_F(TransformTests, concat)
{
    ASSERT_EQ(Gosu::Transform::translate(5, 0) * Gosu::Transform::translate(0, 20),
              Gosu::Transform::translate(5, 20));
}
