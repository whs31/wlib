#pragma once

#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief The WGS84 ellipsoid.
 * @details Plain `double` constants in metres.
 * @ingroup core_geodesy
 */
namespace wgs84
{
  /// @brief Equatorial radius in metres.
  inline auto constexpr semi_major_axis = 6'378'137.0;

  /// @brief Flattening `f = (a − b) / a`.
  inline auto constexpr flattening = 1.0 / 298.257'223'563;

  /// @brief Polar radius `b = a (1 − f)` in metres.
  inline auto constexpr semi_minor_axis = semi_major_axis * (1.0 - flattening);

  /// @brief First eccentricity squared, `e² = f (2 − f)`.
  inline auto constexpr eccentricity_squared = flattening * (2.0 - flattening);

  /// @brief Second eccentricity squared, `e'² = e² / (1 − e²)`.
  inline auto constexpr second_eccentricity_squared = eccentricity_squared
                                                    / (1.0 - eccentricity_squared);
}  // namespace wgs84

W_COMMON_NAMESPACE_END
