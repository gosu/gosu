#pragma once

#include <array>

namespace Gosu
{
    /// Describes an affine transformation that can be temporarily applied to everything on the
    /// screen using the Gosu::Graphics::transform() method.
    struct Transform
    {
        /// The 4x4 transformation matrix in row-major order: matrix[row * 4 + column].
        std::array<double, 16> matrix;

        /// Applies this transform to the given point.
        void apply(double& x, double& y) const;

        [[nodiscard]] static Transform translate(double x, double y);
        [[nodiscard]] static Transform rotate(double angle);
        [[nodiscard]] static Transform scale(double factor);
        [[nodiscard]] static Transform scale(double scale_x, double scale_y);
        /// This helper is supposed to be used on the return value of rotate() or scale() in order
        /// to set the center around which to rotate/scale. It will not have any effect on the
        /// return value of translate().
        [[nodiscard]] Transform around(double around_x, double around_y) const;

        /// Performs a matrix multiplication that has the effect of "concatenating" transforms.
        /// The left-hand side will be applied to a point before the right-hand side.
        Transform operator*(const Transform& rhs) const;

        bool operator==(const Transform&) const = default;
    };
}
