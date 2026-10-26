#pragma once

#include <w/common/longitude.h>
#include <w/common/glaze/detail.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  inline auto constexpr longitude_reading = CoordinateReading {
    .limit_degrees = 180.0,
    .out_of_range = "a longitude must be in [-180, 180] degrees",
    .invalid_string =
      "a longitude string must be degrees with an optional hemisphere E, W, east or west, "
      "e.g. \"30.326E\", \"30east\" or \"30 19 33.6 E\"",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Longitude: its signed degrees, `30.326`.
 * @details A text format, JSON or TOML, also reads any string Longitude::parse() reads:
 * `"30.326E"`, `"157.86west"`, `"157°51'36\"W"`. A longitude outside `[-180°, 180°]` in a
 * document is more likely a mistake than a turn around the Earth, so it fails with
 * `constraint_violated` rather than wrapping, as does a string parse() rejects. Binary formats
 * such as BEVE read the number only.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Longitude>
  : w::common::_glaze_impl::
      CoordinateAdapter<w::common::Longitude, w::common::_glaze_impl::longitude_reading>
{};
