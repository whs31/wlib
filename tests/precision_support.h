#pragma once

#include <algorithm>
#include <concepts>

#include <w/common/math.h>
#include <w/common/precision.h>

namespace test
{
  /// @brief Whether the library is built with `precision=double`.
  inline auto constexpr double_precision = std::same_as<w::Real, double>;

  /// @brief Relative error a short computation in Real stays within: a few units in the last
  /// place of a `double`, or of a `float`.
  inline auto constexpr real_tolerance = double_precision ? 1e-9 : 1e-6;

  /**
   * @brief Absolute tolerance of a check: @p for_double in double precision, @p for_float in
   * single precision.
   */
  [[nodiscard]] constexpr double tolerance(double const for_double, double const for_float = 1e-5)
  {
    return double_precision ? for_double : std::max(for_double, for_float);
  }

  /**
   * @brief Checks that @p actual is within @p tolerance of @p expected, relative to the magnitude
   * of @p expected but never tighter than @p tolerance in absolute terms. The tolerance never goes
   * below real_tolerance, so a check written for `double` still holds in `float`.
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
