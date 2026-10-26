#pragma once

#include <w/common/latitude.h>
#include <w/common/glaze/detail.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  inline auto constexpr latitude_reading = CoordinateReading {
    .limit_degrees = 90.0,
    .out_of_range = "a latitude must be in [-90, 90] degrees",
    .invalid_string =
      "a latitude string must be degrees with an optional hemisphere N, S, north or south, "
      "e.g. \"59.771N\", \"50north\" or \"59 46 15.6 N\"",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Latitude: its signed degrees, `59.771`.
 * @details A text format, JSON or TOML, also reads any string Latitude::parse() reads:
 * `"59.771N"`, `"50north"`, `"33.86 south"`, `"59°46'15.6\"N"`. A latitude outside `[-90°, 90°]`
 * or a string parse() rejects fails with `constraint_violated`. Binary formats such as BEVE read
 * the number only.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Latitude>
  : w::common::_glaze_impl::
      CoordinateAdapter<w::common::Latitude, w::common::_glaze_impl::latitude_reading>
{};
