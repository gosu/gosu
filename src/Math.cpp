#include <Gosu/Math.hpp>
#include <cmath>
#include <limits> // for std::numeric_limits::quiet_NaN
#include <random>

double Gosu::random(double min, double max)
{
    if (std::isnan(min) || std::isnan(max)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    if (min == max) {
        return min;
    }

    thread_local std::mt19937_64 generator(std::random_device {}());
    std::uniform_real_distribution<double> distribution(min, max);
    const double result = distribution(generator);
    // Make sure that we do not return "max" even when uniform_real_distribution is buggy.
    // See "Notes": https://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
    return result == max ? min : result;
}

double Gosu::offset_x(double angle, double radius)
{
    return +std::sin(angle / 180 * std::numbers::pi) * radius;
}

double Gosu::offset_y(double angle, double radius)
{
    return -std::cos(angle / 180 * std::numbers::pi) * radius;
}

double Gosu::angle(double from_x, double from_y, double to_x, double to_y, double fallback)
{
    double dist_x = to_x - from_x;
    double dist_y = to_y - from_y;

    // Special-case the four cardinal directions values so that we don't run into subtle rounding
    // errors for these obvious values.
    if (dist_x == 0) {
        if (dist_y == 0) {
            return fallback;
        }

        return dist_y < 0 ? 0 : 180;
    }
    else if (dist_y == 0) {
        return dist_x < 0 ? -90 : 90;
    }

    return normalize_angle(radians_to_angle(std::atan2(dist_y, dist_x)));
}

double Gosu::angle_diff(double from, double to)
{
    return normalize_angle(to - from + 180) - 180;
}

double Gosu::normalize_angle(double angle)
{
    return wrap(angle, 0.0, 360.0);
}

int Gosu::wrap(int value, int min, int max)
{
    int result = (value - min) % (max - min);
    return result < 0 ? result + max : result + min;
}

double Gosu::wrap(double value, double min, double max)
{
    double result = std::fmod(value - min, max - min);
    return result < 0 ? result + max : result + min;
}

double Gosu::distance(double x1, double y1, double x2, double y2)
{
    double dist_x = (x2 - x1);
    double dist_y = (y2 - y1);
    return std::sqrt(dist_x * dist_x + dist_y * dist_y);
}
