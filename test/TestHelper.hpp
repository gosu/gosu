#pragma once

#include <gtest/gtest.h>

#include <Gosu/Bitmap.hpp>
#include <numeric>

/// Compares two bitmaps, with parameters allowing for different kinds of tolerances.
/// This is a bit more specific than the Gosu::Bitmap#similar? method from test_helper.rb.
///
/// @param tolerance The tolerance of how much each RGBA channel is allowed to differ per pixel
///                  before the comparison fails.
/// @param max_differences How many pixels are allowed to fail the comparison (after applying
///                        tolerance) before the whole bitmap comparison fails.
inline testing::AssertionResult visible_pixels_are_equal(const Gosu::Bitmap& lhs,
                                                         const Gosu::Bitmap& rhs,
                                                         int tolerance = 0,
                                                         int max_differences = 0)
{
    if (lhs.width() != rhs.width() || lhs.height() != rhs.height()) {
        return testing::AssertionFailure() << "different sizes";
    }

    int differences = 0;

    for (int x = 0; x < lhs.width(); ++x) {
        for (int y = 0; y < lhs.height(); ++y) {
            const Gosu::Color lhs_pixel = lhs.pixel(x, y);
            const Gosu::Color rhs_pixel = rhs.pixel(x, y);

            // Ignore RGB of fully transparent pixels.
            if (lhs_pixel.alpha == 0 && rhs_pixel.alpha == 0) {
                continue;
            }

            if (std::abs(lhs_pixel.alpha - rhs_pixel.alpha) > tolerance
                || std::abs(lhs_pixel.red - rhs_pixel.red) > tolerance
                || std::abs(lhs_pixel.green - rhs_pixel.green) > tolerance
                || std::abs(lhs_pixel.blue - rhs_pixel.blue) > tolerance) {

                ++differences;
                if (differences > max_differences) {
                    return testing::AssertionFailure() << "difference at " << x << ", " << y;
                }
            }
        }
    }

    return testing::AssertionSuccess();
}
