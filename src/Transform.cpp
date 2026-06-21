// Default matrices, adapted from original Transform support contribution by erisdiscord. Thank you!

#include <Gosu/Math.hpp>
#include <Gosu/Transform.hpp>
#include <cmath>

void Gosu::Transform::apply(double& x, double& y) const
{
    double in[4] = { x, y, 0, 1 };
    double out[4] = { 0, 0, 0, 0 };
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            out[i] += in[j] * matrix[j * 4 + i];
        }
    }
    x = out[0] / out[3];
    y = out[1] / out[3];
}

Gosu::Transform Gosu::Transform::translate(double x, double y)
{
    return Transform { 1, 0, 0, 0, //
                       0, 1, 0, 0, //
                       0, 0, 1, 0, //
                       x, y, 0, 1 };
}

Gosu::Transform Gosu::Transform::rotate(double angle)
{
    double c = std::cos(degrees_to_radians(angle));
    double s = std::sin(degrees_to_radians(angle));
    return Transform { +c, +s, 0, 0, //
                       -s, +c, 0, 0, //
                       0,  0,  1, 0, //
                       0,  0,  0, 1 };
}

Gosu::Transform Gosu::Transform::scale(double factor)
{
    return Transform { factor, 0,      0, 0, //
                       0,      factor, 0, 0, //
                       0,      0,      1, 0, //
                       0,      0,      0, 1 };
}

Gosu::Transform Gosu::Transform::scale(double scale_x, double scale_y)
{
    return Transform { scale_x, 0,       0, 0, //
                       0,       scale_y, 0, 0, //
                       0,       0,       1, 0, //
                       0,       0,       0, 1 };
}

Gosu::Transform Gosu::Transform::around(double around_x, double around_y) const
{
    return translate(-around_x, -around_y) * *this * translate(around_x, around_y);
}

Gosu::Transform Gosu::Transform::operator*(const Transform& rhs) const
{
    Transform result { 0 };
    for (int i = 0; i < 16; ++i) {
        result.matrix[i] = 0;
        for (int j = 0; j < 4; ++j) {
            result.matrix[i] += matrix[i / 4 * 4 + j] * rhs.matrix[i % 4 + j * 4];
        }
    }
    return result;
}
