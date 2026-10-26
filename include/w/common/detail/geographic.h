#pragma once

#include <w/common/angle.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

namespace _geographic_impl  // NOLINT(readability-identifier-naming)
{
  /// @brief The degree sign in UTF-8.
  inline auto constexpr degree_sign = "\xc2\xb0";

  /**
   * @brief @p degrees wrapped into `[-180, 180)`, unchanged if already there, so that a value read
   * from a document keeps its exact digits.
   */
  [[nodiscard]] constexpr double wrapped_degrees(double const degrees) noexcept
  {
    if(not(degrees < -180.0 or degrees >= 180.0))
      return degrees;
    auto const wrapped = fmod(degrees + 180.0, 360.0) - 180.0;
    // a negative value a few ulps below a multiple of the turn rounds up to the full turn
    return wrapped >= 180.0 ? -180.0 : wrapped;
  }

  /// @brief @p degrees wrapped into `[0, 360)`.
  [[nodiscard]] constexpr double wrapped_positive_degrees(double const degrees) noexcept
  {
    auto const wrapped = wrapped_degrees(degrees);
    return wrapped < 0.0 ? wrapped + 360.0 : wrapped;
  }

  /**
   * @brief An angular difference in degrees as an Angle, converted to radians in `double` before
   * rounding to Real.
   */
  [[nodiscard]] constexpr Angle angle_of_degrees(double const degrees) noexcept
  {
    return Angle::from_radians(static_cast<Real>(to_radians(degrees)));
  }

  /// @brief An Angle in degrees, converted from radians in `double`.
  [[nodiscard]] constexpr double degrees_of(Angle const angle) noexcept
  {
    return to_degrees(static_cast<double>(angle.radians()));
  }
}  // namespace _geographic_impl

W_COMMON_NAMESPACE_END
