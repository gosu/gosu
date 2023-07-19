#pragma once

#include <array>

namespace Gosu
{
    struct Transform
    {
        // The 4x4 transformation matrix in row-major order: matrix[row * 4 + column].
        std::array<double, 16> matrix;

        // Applies this transform to the given point.
        void apply(double& x, double& y) const;

        [[nodiscard]] static Transform translate(double x, double y);
        [[nodiscard]] static Transform rotate(double angle);
        [[nodiscard]] static Transform scale(double factor);
        [[nodiscard]] static Transform scale(double scale_x, double scale_y);
        [[nodiscard]] Transform around(double around_x, double around_y) const;

        Transform operator*(const Transform& rhs) const;
        bool operator==(const Transform&) const = default;
    };
}
