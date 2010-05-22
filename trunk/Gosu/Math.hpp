//! \file Math.hpp
//! Contains simple math functionality.

#ifndef GOSU_MATH_HPP
#define GOSU_MATH_HPP

namespace Gosu
{
    //! Pi.
    const double pi = 3.1415926536;
    
    //! Truncates the fractional part of a real value. Equivalent to
    //! static_cast<long>.
    inline long trunc(double value)
    {
        return static_cast<long>(value);
    }
    
    //! Rounds a real value towards the next integer.
    inline long round(double value)
    {
        if (value >= 0)
            return static_cast<long>(value + 0.5);
        else
            return static_cast<long>(value - 0.5);
    }
    
    //! Returns a real value between min (inclusive) and max (exclusive).
    //! Uses std::rand, so you should call std::srand before using it.
    double random(double min, double max);
    
    //! Translates between Gosu's angle system (where 0¡ is at the top)
    //! and radians (where 0 is at the right).
    inline double gosuToRadians(double angle)
    {
        return (angle - 90) * pi / 180;
    }
    //! Translates between Gosu's angle system (where 0¡ is at the top)
    //! and radians (where 0 is at the right).
    inline double radiansToGosu(double angle)
    {
        return angle * 180 / pi + 90;
    }

    //! Translates between degrees (used by Gosu) and radians, i.e. it
    //! does not change the 'origin' of the angle system.
    inline double degreesToRadians(double angle)
    {
        return angle * pi / 180;
    }
    //! Translates between degrees (used by Gosu) and radians, i.e. it
    //! does not change the 'origin' of the angle system.
    inline double radiansToDegrees(double angle)
    {
        return angle * 180 / pi;
    }
    
    //! Returns the horizontal distance between the origin and the point to
    //! which you would get if you moved radius pixels in the direction
    //! specified by angle.
    //! \param angle Angle in degrees where 0.0 means upwards.
    double offsetX(double angle, double radius);
    //! Returns the vertical distance between the origin and the point to
    //! which you would get if you moved radius pixels in the direction
    //! specified by angle.
    //! \param angle Angle in degrees where 0.0 means upwards.
    double offsetY(double angle, double radius);
    //! Returns the angle from point 1 to point 2 in degrees, where 0.0 means
    //! upwards. Returns def if both points are equal.
    double angle(double fromX, double fromY, double toX, double toY,
        double def = 0);
    //! Returns the smallest angle that can be added to angle1 to get to
    //! angle2 (can be negative if counter-clockwise movement is shorter).
    double angleDiff(double angle1, double angle2);
    //! Normalizes an angle to fit into the range [0; 360[.
    double normalizeAngle(double angle);
    
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
        if (value < min)
            return min;
        if (value > max)
            return max;
        return value;
    }
    
    // Backward compatibility with 0.7.x
    template<typename T>
    T boundBy(T value, T min, T max)
    {
        return clamp(value, min, max);
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
    inline double distanceSqr(double x1, double y1, double x2, double y2)
    {
        return square(x1 - x2) + square(y1 - y2);
    }
    
    //! Returns the distance between two points.
    double distance(double x1, double y1, double x2, double y2);
    
    //! Interpolates a value between a and b, weight being the bias towards the second value.
    //! Examples: interpolate(0, 10, 0.5) == 5, interpolate(-10, 10, 0.25) == 5, interpolate(0, 10, -0.5) == -5.
    template<typename T>
    T interpolate(T a, T b, double weight = 0.5)
    {
        return a * (1.0 - weight) + b * weight;
    }
}

#endif
