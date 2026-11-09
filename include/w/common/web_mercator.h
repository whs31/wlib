#pragma once

#include <cmath>

#include <algorithm>
#include <numbers>

#include <w/common/stdint.h>
#include <w/common/distance.h>
#include <w/common/latitude.h>
#include <w/common/longitude.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief The XYZ Web Mercator (EPSG:3857) tile scheme of slippy maps, elevation tiles and imagery
 * providers: global map positions in tiles of a zoom, x east from the antimeridian, y south from
 * the northern edge of the map.
 * @details A map position is fractional; its integer part is the tile, the fraction the position
 * inside it. At zoom `z` the map is tiles(z) tiles wide and as tall.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto const at = web_mercator::project(60.2_north, 30.3_east, 12);
 * auto const tile_x = static_cast<u32>(at.x);
 * auto const tile_y = static_cast<u32>(at.y);
 * auto const north_edge = web_mercator::latitude_at(std::floor(at.y), 12);
 * @endcode
 *
 * @ingroup core_geodesy
 */
namespace web_mercator
{
  /// @brief Web Mercator stops short of the poles, where its map is square.
  inline auto constexpr max_latitude = Latitude::from_degrees(85.051'128'78);

  /// @brief The sphere of Web Mercator, as large as the WGS84 equator.
  inline auto constexpr radius = 6'378'137_meters;

  /// @brief A global map position, in tiles of a zoom.
  struct Point
  {
    double x = 0.0;
    double y = 0.0;

    [[nodiscard]] constexpr bool operator==(Point const&) const noexcept = default;
  };

  /// @brief Tiles along either side of the map at `zoom`, `2^zoom`.
  [[nodiscard]] constexpr double tiles(u8 const zoom) noexcept
  {
    return static_cast<double>(1_u64 << zoom);
  }

  /// @brief The latitude in radians, clamped into the map.
  [[nodiscard]] inline double clamped_radians(Latitude const latitude) noexcept
  {
    auto constexpr limit = max_latitude.degrees();
    return std::clamp(latitude.degrees(), -limit, limit) * std::numbers::pi / 180.0;
  }

  /**
   * @brief Where a point is on the map at `zoom`, its latitude clamped into the map.
   * @param latitude Latitude, clamped to ±max_latitude.
   * @param longitude Longitude; the map starts at the antimeridian, so x is in `[0, tiles(zoom))`.
   * @param zoom Zoom level.
   */
  [[nodiscard]] inline Point project(
    Latitude const latitude,
    Longitude const longitude,
    u8 const zoom
  ) noexcept
  {
    auto const phi = clamped_radians(latitude);
    return {
      .x = (longitude.degrees() + 180.0) / 360.0 * tiles(zoom),
      .y = (1.0 - std::log(std::tan(phi) + 1.0 / std::cos(phi)) / std::numbers::pi) / 2.0
         * tiles(zoom),
    };
  }

  /// @brief The latitude of row `y` of the map at `zoom`, the inverse of project().
  [[nodiscard]] inline Latitude latitude_at(double const y, u8 const zoom) noexcept
  {
    return Latitude::from_radians(
      std::atan(std::sinh(std::numbers::pi * (1.0 - 2.0 * y / tiles(zoom))))
    );
  }

  /// @brief The longitude of column `x` of the map at `zoom`, the inverse of project(); the right
  /// edge of the map is the antimeridian again.
  [[nodiscard]] constexpr Longitude longitude_at(double const x, u8 const zoom) noexcept
  {
    return Longitude::from_degrees(x / tiles(zoom) * 360.0 - 180.0);
  }

  /**
   * @brief The ground size of a tile of `zoom` at a latitude, clamped into the map.
   * @details Web Mercator is conformal, so a tile is as tall as it is wide.
   */
  [[nodiscard]] inline Distance tile_size(Latitude const latitude, u8 const zoom) noexcept
  {
    return Distance::from_meters(
      static_cast<Real>(
        2.0 * std::numbers::pi * radius.as_m() * std::cos(clamped_radians(latitude)) / tiles(zoom)
      )
    );
  }
}  // namespace web_mercator

W_COMMON_NAMESPACE_END
