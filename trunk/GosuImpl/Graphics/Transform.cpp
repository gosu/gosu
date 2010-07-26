// Default matrices, adapted from original Transform support
// contribution by erisdiscord. Thank you!

#include <Gosu/Graphics.hpp>
#include <Gosu/Math.hpp>
#include <GosuImpl/Graphics/Common.hpp>
#include <cmath>

Gosu::Transform
Gosu::rotate(double angle, double aroundX, double aroundY)
{
    double c = std::cos(degreesToRadians(angle));
    double s = std::sin(degreesToRadians(angle));
    Gosu::Transform result = {
        +c, +s, 0, 0,
        -s, +c, 0, 0,
        0,  0,  1, 0,
        0,  0,  0, 1
    };
    if (aroundX != 0 || aroundY != 0)
        result = multiply(multiply(translate(-aroundX, -aroundY), result), translate(aroundX, aroundY));
    return result;
}

Gosu::Transform
Gosu::translate(double x, double y)
{
    Gosu::Transform result = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    };
    return result;
}

Gosu::Transform
Gosu::scale(double factor)
{
    Gosu::Transform result = {
        factor, 0,      0, 0,
        0,      factor, 0, 0,
        0,      0,      1, 0,
        0,      0,      0, 1
    };
    return result;
}

Gosu::Transform
Gosu::scale(double factorX, double factorY, double aroundX, double aroundY)
{
    Gosu::Transform result = {
        factorX, 0,       0, 0,
        0,       factorY, 0, 0,
        0,       0,       1, 0,
        0,       0,       0, 1
    };
    if (aroundX != 0 || aroundY != 0)
        result = multiply(multiply(translate(-aroundX, -aroundY), result), translate(aroundX, aroundY));
    return result;
}
