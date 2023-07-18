#include <gtest/gtest.h>

#include <Gosu/Math.hpp>
#include <set>

class MathTests : public testing::Test
{
public:
    static constexpr double EPSILON = 0.001;
};

TEST_F(MathTests, random)
{
    std::set<int> positive_values;
    for (int i = 0; i < 1'000'000; ++i) {
        const double random_value = Gosu::random(3.5, 7.0);
        positive_values.insert(static_cast<int>(random_value));
    }
    // 7 must not be returned because the upper bound is exclusive.
    ASSERT_EQ(positive_values, std::set<int>({ 3, 4, 5, 6 }));

    std::set<int> negative_values;
    for (int i = 0; i < 1'000'000; ++i) {
        const double random_value = Gosu::random(-1000, -1005);
        negative_values.insert(static_cast<int>(random_value));
    }
    // -1005 must not be returned because the upper bound is exclusive.
    ASSERT_EQ(negative_values, std::set<int>({ -1004, -1003, -1002, -1001, -1000 }));

    ASSERT_EQ(Gosu::random(12345678, 12345678), 12345678.0);

    ASSERT_TRUE(std::isnan(Gosu::random(0.0 / 0.0, 0.0 / 0.0)));
}

TEST_F(MathTests, conversions)
{
    ASSERT_EQ(Gosu::radians_to_degrees(0), 0);
    ASSERT_NEAR(Gosu::radians_to_degrees(-std::numbers::pi), -180, EPSILON);
    ASSERT_NEAR(Gosu::radians_to_degrees(2 * std::numbers::pi), 360, EPSILON);

    ASSERT_EQ(Gosu::degrees_to_radians(0), 0);
    ASSERT_NEAR(Gosu::degrees_to_radians(45), std::numbers::pi / 4, EPSILON);
    ASSERT_NEAR(Gosu::degrees_to_radians(360), 2 * std::numbers::pi, EPSILON);

    // Up
    ASSERT_EQ(Gosu::radians_to_angle(0), 90);
    // Left
    ASSERT_EQ(Gosu::radians_to_angle(std::numbers::pi), 270);
    // Up again
    ASSERT_EQ(Gosu::radians_to_angle(std::numbers::pi * 2), 90 + 360);

    // Upper right
    ASSERT_EQ(Gosu::angle_to_radians(45), -std::numbers::pi / 4);
    // Down
    ASSERT_EQ(Gosu::angle_to_radians(180), std::numbers::pi / 2);
}

TEST_F(MathTests, trigonmetry)
{
    // Horizontal offset based on direction:
    // Up
    ASSERT_NEAR(Gosu::offset_x(0, 123), 0, EPSILON);
    // Upper left
    ASSERT_NEAR(Gosu::offset_x(-45, 1), -std::numbers::sqrt2 / 2, EPSILON);
    // Right
    ASSERT_NEAR(Gosu::offset_x(90, 543), 543, EPSILON);

    // Vertical offset based on direction:
    // Up
    ASSERT_NEAR(Gosu::offset_y(0, 123), -123, EPSILON);
    // Upper left
    ASSERT_NEAR(Gosu::offset_y(-45, 1), -std::numbers::sqrt2 / 2, EPSILON);
    // Right
    ASSERT_NEAR(Gosu::offset_y(90, 543), 0, EPSILON);

    // Angles based on direction:
    // Up
    ASSERT_EQ(Gosu::angle(100, 100, 100, 5), 0);
    // Upper left
    ASSERT_NEAR(Gosu::angle(0, 0, -0.5, -0.5), 315, EPSILON);
    // Right
    ASSERT_EQ(Gosu::angle(111'111, -222'222, 888'888, -222'222), 90);
    // Down
    ASSERT_EQ(Gosu::angle(0, 0, 0, 1e10), 180);
    // Left
    ASSERT_EQ(Gosu::angle(0, 0, -1e-10, 0), -90);
    // Indeterminate angle, use fallback
    ASSERT_EQ(Gosu::angle(5, 5, 5, 5, 1234), 1234);
}

TEST_F(MathTests, angle_normalization)
{
    ASSERT_EQ(Gosu::normalize_angle(0), 0);
    ASSERT_EQ(Gosu::normalize_angle(360), 0);
    ASSERT_NEAR(Gosu::normalize_angle(722), 2, EPSILON);
    ASSERT_NEAR(Gosu::normalize_angle(-45), 315, EPSILON);

    ASSERT_NEAR(Gosu::angle_diff(0, 90), 90, EPSILON);
    ASSERT_NEAR(Gosu::angle_diff(90, 0), -90, EPSILON);
    ASSERT_NEAR(Gosu::angle_diff(360, 360), 0, EPSILON);
    ASSERT_NEAR(Gosu::angle_diff(45, 315), -90, EPSILON);
}

TEST_F(MathTests, wrap)
{
    ASSERT_EQ(Gosu::wrap(17, 10, 20), 17);
    ASSERT_EQ(Gosu::wrap(1, 10, 20), 11);
    ASSERT_EQ(Gosu::wrap(-3, 10, 20), 17);
    ASSERT_EQ(Gosu::wrap(1, -10, -20), -9);
}

TEST_F(MathTests, distance)
{
    ASSERT_NEAR(Gosu::distance(0, 0, 0, 0), 0, EPSILON);
    // sqrt(3^2 + 4^2) == 5
    ASSERT_NEAR(Gosu::distance(100, -1000, 103, -1004), 5, EPSILON);
}
