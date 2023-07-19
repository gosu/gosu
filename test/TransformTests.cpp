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
    ASSERT_NEAR(x, -10, EPSILON);
    ASSERT_NEAR(y, -10, EPSILON);

    // If the result is then rotated around a point above it, it will move even further up.
    Gosu::Transform::rotate(90).around(0, -20).apply(x, y);
    ASSERT_NEAR(x, -10, EPSILON);
    ASSERT_NEAR(y, -30, EPSILON);

    // A point rotated around itself is still the same point.
    Gosu::Transform::rotate(123).around(x, y).apply(x, y);
    ASSERT_NEAR(x, -10, EPSILON);
    ASSERT_NEAR(y, -30, EPSILON);
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
    ASSERT_NEAR(x, -40, EPSILON);
    ASSERT_NEAR(y, 3, EPSILON);
}

TEST_F(TransformTests, concatenation)
{
    // "Concatenating" two translations results in the sum of both offsets.
    ASSERT_EQ(Gosu::Transform::translate(5, 0) * Gosu::Transform::translate(0, 20),
              Gosu::Transform::translate(5, 20));

    // (A * B).apply(x, y) should be the same as A.apply(x, y); B.apply(x, y); (left-to-right)
    double x = 0, y = -10;
    (Gosu::Transform::rotate(90) * Gosu::Transform::translate(10, 0)).apply(x, y);
    ASSERT_NEAR(x, 20, EPSILON);
    ASSERT_NEAR(y, 0, EPSILON);
}
