// Default matrices, adapted from original Transform support
// contribution by erisdiscord. Thank you!

#include "GraphicsImpl.hpp"
#include <Gosu/GraphicsBase.hpp>
#include <Gosu/Math.hpp>
#include <cmath>
using namespace std;

Gosu::Transform Gosu::rotate(double angle, double around_x, double around_y)
{
    double c = cos(degrees_to_radians(angle));
    double s = sin(degrees_to_radians(angle));
    Transform result = {
        +c, +s, 0, 0,
        -s, +c, 0, 0,
        0,  0,  1, 0,
        0,  0,  0, 1
    };
    if (around_x != 0 || around_y != 0) {
        result = concat(concat(translate(-around_x, -around_y), result),
                        translate(around_x, around_y));
    }
    return result;
}

Gosu::Transform Gosu::translate(double x, double y)
{
    Transform result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    };
    return result;
}

Gosu::Transform Gosu::scale(double factor)
{
    Transform result = {
        factor, 0,      0, 0,
        0,      factor, 0, 0,
        0,      0,      1, 0,
        0,      0,      0, 1
    };
    return result;
}

Gosu::Transform Gosu::scale(double scale_x, double scale_y, double around_x, double around_y)
{
    Transform result = {
        scale_x, 0,       0, 0,
        0,       scale_y, 0, 0,
        0,       0,       1, 0,
        0,       0,       0, 1
    };
    if (around_x != 0 || around_y != 0) {
        result = concat(concat(translate(-around_x, -around_y), result),
                        translate(around_x, around_y));
    }
    return result;
}

Gosu::Transform Gosu::concat(const Transform& left, const Transform& right)
{
    Transform result;
    for (int i = 0; i < 16; ++i) {
        result[i] = 0;
        for (int j = 0; j < 4; ++j) {
            result[i] += left[i / 4 * 4 + j] * right[i % 4 + j * 4];
        }
    }
    return result;
}
