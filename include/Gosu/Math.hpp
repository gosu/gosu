#pragma once

#include <numbers>

namespace Gosu
{
    /// Returns a double value between min (inclusive) and max (exclusive).
    /// This function returns random values based on a uniform distribution.
    /// Special case: If min and max are the same value, then that value is returned.
    double random(double min, double max);

    /// Translates between 360° degrees (as used by Gosu) and radians, but does not change the
    /// origin from the top to the right.
    inline double degrees_to_radians(double angle)
    {
        return angle * std::numbers::pi / 180;
    }

    /// Translates between 360° degrees (as used by Gosu) and radians, but does not change the
    /// origin from the right to the top.
    inline double radians_to_degrees(double angle)
    {
        return angle * 180 / std::numbers::pi;
    }

    /// Translates between Gosu's 360° angle system, where 0 is at the top, and radians, where 0 is
    /// to the right.
    inline double radians_to_angle(double angle)
    {
        return radians_to_degrees(angle) + 90;
    }

    /// Translates between Gosu's 360° angle system, where 0 is at the top, and radians, where 0 is
    /// to the right.
    inline double angle_to_radians(double angle)
    {
        return degrees_to_radians(angle - 90);
    }

    /// Returns the horizontal distance between the origin and the point to
    /// which you would get if you moved radius pixels in the direction
    /// specified by angle.
    /// @param angle Angle in degrees where 0.0 means upwards.
    double offset_x(double angle, double radius);

    /// Returns the vertical distance between the origin and the point to
    /// which you would get if you moved radius pixels in the direction
    /// specified by angle.
    /// @param angle Angle in degrees where 0.0 means upwards.
    double offset_y(double angle, double radius);

    /// Returns the angle from the first point to the second point in degrees, where 0.0 points up.
    /// Returns "fallback" if both points are equal.
    double angle(double from_x, double from_y, double to_x, double to_y, double fallback = 0);

    /// Returns the smallest angle that can be added to angle1 to get to angle2.
    /// The result can be negative if counter-clockwise movement is shorter.
    double angle_diff(double angle1, double angle2);

    /// Normalizes an angle to fit into the range [0; 360[.
    double normalize_angle(double angle);

    /// Returns (value-min) % (max-min) + min, where % always has a positive
    /// result for max > min. The results are undefined for max <= min.
    /// Note: This means that max is exclusive.
    int wrap(int value, int min, int max);
    /// Returns (value-min) % (max-min) + min, where % always has a positive
    /// result for max > min. The results are undefined for max <= min.
    /// Note: This means that max is exclusive.
    double wrap(double value, double min, double max);

    /// Returns the distance between two points.
    double distance(double x1, double y1, double x2, double y2);
}
