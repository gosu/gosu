//! \file Math.hpp
//! Contains simple math functionality.

#pragma once

#include <cmath>

// Portability: Define M_PI, which MSVC's <cmath> does not do by default.
#if !defined(M_PI) && !defined(SWIG)
#define M_PI 3.14159265358979323846264338327950288
#endif

namespace Gosu
{
    //! Returns a real value between min (inclusive) and max (exclusive).
    //! Uses std::rand, so you should call std::srand before using it.
    double random(double min, double max);
    
    //! Translates between Gosu's angle system (where 0 is at the top)
    //! and radians (where 0 is at the right).
    inline double gosu_to_radians(double angle)
    {
        return (angle - 90) * M_PI / 180;
    }
    //! Translates between Gosu's angle system (where 0 is at the top)
    //! and radians (where 0 is at the right).
    inline double radians_to_gosu(double angle)
    {
        return angle * 180 / M_PI + 90;
    }

    //! Translates between degrees (used by Gosu) and radians, i.e. it
    //! does not change the 'origin' of the angle system.
    inline double degrees_to_radians(double angle)
    {
        return angle * M_PI / 180;
    }
    //! Translates between degrees (used by Gosu) and radians, i.e. it
    //! does not change the 'origin' of the angle system.
    inline double radians_to_degrees(double angle)
    {
        return angle * 180 / M_PI;
    }
    
    //! Returns the horizontal distance between the origin and the point to
    //! which you would get if you moved radius pixels in the direction
    //! specified by angle.
    //! \param angle Angle in degrees where 0.0 means upwards.
    double offset_x(double angle, double radius);
    //! Returns the vertical distance between the origin and the point to
    //! which you would get if you moved radius pixels in the direction
    //! specified by angle.
    //! \param angle Angle in degrees where 0.0 means upwards.
    double offset_y(double angle, double radius);
    //! Returns the angle from point 1 to point 2 in degrees, where 0.0 means
    //! upwards. Returns def if both points are equal.
    double angle(double from_x, double from_y, double to_x, double to_y, double def = 0);
    //! Returns the smallest angle that can be added to angle1 to get to
    //! angle2 (can be negative if counter-clockwise movement is shorter).
    double angle_diff(double angle1, double angle2);
    //! Normalizes an angle to fit into the range [0; 360[.
    double normalize_angle(double angle);
    
    //! Returns value * value.
    template<typename T>
    T square(T value)
    {
        return value * value;
    }
    
    //! Returns min if value is smaller than min, max if value is larger than
    //! max and value otherwise.
    template<typename T>
    T clamp(T value, T min, T max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    
    //! Returns (value-min) % (max-min) + min, where % always has a positive
    //! result for max > min. The results are undefined for max <= min.
    //! Note: This means that max is exclusive.
    int wrap(int value, int min, int max);
    //! Returns (value-min) % (max-min) + min, where % always has a positive
    //! result for max > min. The results are undefined for max <= min.
    //! Note: This means that max is exclusive.
    float wrap(float value, float min, float max);
    //! Returns (value-min) % (max-min) + min, where % always has a positive
    //! result for max > min. The results are undefined for max <= min.
    //! Note: This means that max is exclusive.
    double wrap(double value, double min, double max);
    
    //! Returns the square of the distance between two points.
    inline double distance_sqr(double x1, double y1, double x2, double y2)
    {
        return square(x1 - x2) + square(y1 - y2);
    }
    
    //! Returns the distance between two points.
    double distance(double x1, double y1, double x2, double y2);
    
    //! Interpolates a value between a and b, weight being the bias towards the second value.
    //! Examples: interpolate(0, 10, 0.5) == 5, interpolate(-10, 10, 0.25) == 5,
    //! interpolate(0, 10, -0.5) == -5.
    template<typename T>
    T interpolate(T a, T b, double weight = 0.5)
    {
        return a * (1.0 - weight) + b * weight;
    }
}
