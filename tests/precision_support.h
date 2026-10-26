#pragma once

#include <algorithm>

#include <w/common/math.h>

namespace test
{
  /// @brief Relative error a short computation in `float` stays within: a few units in the last
  /// place.
  inline auto constexpr real_tolerance = 1e-6;

  /**
   * @brief Absolute tolerance of a check: @p tolerance, but no tighter than @p float_floor, which
   * a `float` computation resolves.
   */
  [[nodiscard]] constexpr double tolerance(double const tolerance, double const float_floor = 1e-5)
  {
    return std::max(tolerance, float_floor);
  }

  /**
   * @brief Checks that @p actual is within @p tolerance of @p expected, relative to the magnitude
   * of @p expected but never tighter than @p tolerance in absolute terms. The tolerance never goes
   * below real_tolerance.
   */
  [[nodiscard]] constexpr bool near(
    double const actual,
    double const expected,
    double const tolerance = real_tolerance
  )
  {
    auto const scale = std::max(1.0, w::abs(expected));
    return w::abs(actual - expected) <= std::max(tolerance, real_tolerance) * scale;
  }
}  // namespace test
