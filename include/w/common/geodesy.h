#pragma once

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <w/common/config.h>
#include <w/common/distance.h>
#include <w/common/latitude.h>
#include <w/common/longitude.h>
#include <w/common/vector3.h>
#include <w/common/wgs84.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetatype.h>
#  include <qobjectdefs.h>
#  include <qstring.h>
#endif

/**
 * @defgroup core_geodesy Geodesy
 * @ingroup core
 * @brief Positions on the WGS84 ellipsoid and the local frames tangent to it.
 */

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Quarter of the globe a position lies in, between the equator and the prime meridian or
 * the antimeridian, numbered counterclockwise from the north-east as the quadrants of a plane with
 * x east and y north.
 * @ingroup core_geodesy
 */
enum class Quadrant : std::uint8_t
{
  NorthEast,
  NorthWest,
  SouthWest,
  SouthEast,
};

/**
 * @brief Returns the quadrant of a latitude and a longitude; the equator counts as north and the
 * prime meridian as east.
 * @ingroup core_geodesy
 */
[[nodiscard]] constexpr Quadrant quadrant(
  Latitude const latitude,
  Longitude const longitude
) noexcept
{
  auto const north = latitude.hemisphere() == Latitude::Hemisphere::North;
  if(longitude.hemisphere() == Longitude::Hemisphere::East)
    return north ? Quadrant::NorthEast : Quadrant::SouthEast;
  return north ? Quadrant::NorthWest : Quadrant::SouthWest;
}

/**
 * @brief A UTM grid zone: the zone number and the latitude band, written `32V`.
 * @ingroup core_geodesy
 * @see GeodeticPosition::utm_zone()
 */
struct UtmZone
{
  /// @brief 1 to 60, each 6° wide eastward from the antimeridian.
  int number = 0;
  /// @brief `'C'` to `'X'`, without `'I'` and `'O'`.
  char band = '\0';

  /// @brief `<number><band>`, e.g. `32V`.
  [[nodiscard]] std::string to_string() const
  {
    return fmt::format("{}{}", this->number, this->band);
  }

  [[nodiscard]] constexpr bool operator==(UtmZone const&) const noexcept = default;

  /// @brief Formats the zone with `fmt` as its to_string().
  [[nodiscard]] friend std::string format_as(UtmZone const& zone) { return zone.to_string(); }
};

/**
 * @brief WGS84 latitude and longitude with the altitude above mean sea level, positive up.
 * @details An aggregate, so that it is built with designated initializers and read from
 * configuration as a table of its three members:
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto constexpr pulkovo = w::GeodeticPosition {
 *   .latitude = 59.771_north,
 *   .longitude = 30.326_east,
 *   .altitude_msl = 70_meters,
 * };
 * pulkovo.quadrant();              // Quadrant::NorthEast
 * pulkovo.utm_zone()->to_string();  // "36V"
 * @endcode
 *
 * Latitude and Longitude are always `double`, whatever the `precision` option; the altitude is a
 * Distance in that precision, which resolves it to a millimetre up to 16 km even in `float`.
 *
 * There is no `glz::meta` for it: glaze reflects the aggregate, and with the adapters of Latitude,
 * Longitude and Distance (`glaze/latitude.h`, `glaze/longitude.h`, `glaze/distance.h`) writes
 * `{"latitude": 59.771, "longitude": 30.326, "altitude_msl": {"meters": 70}}`, and in a text
 * format also reads coordinate strings, `"latitude": "59.771north"` or `"59°46'15.6\"N"`.
 * Code built on glaze reflection of aggregates keeps working with it.
 * @note Qt properties and metatype registration are available only when wlib is built with
 * `with_qt=True`; QML reads the plain numbers through `latitudeDegrees`, `longitudeDegrees` and
 * `altitudeMslMeters`.
 * @ingroup core_geodesy
 * @see LocalTangentPlane
 */
struct W_COMMON_API GeodeticPosition
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET
  Q_PROPERTY(Latitude latitude MEMBER latitude FINAL)
  Q_PROPERTY(Longitude longitude MEMBER longitude FINAL)
  Q_PROPERTY(Distance altitudeMsl MEMBER altitude_msl FINAL)
  Q_PROPERTY(double latitudeDegrees READ latitude_degrees CONSTANT FINAL)
  Q_PROPERTY(double longitudeDegrees READ longitude_degrees CONSTANT FINAL)
  Q_PROPERTY(double altitudeMslMeters READ altitude_msl_meters CONSTANT FINAL)
  Q_PROPERTY(bool isValid READ is_valid CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)

 public:
#endif
  Latitude latitude;
  Longitude longitude;
  Distance altitude_msl;

  [[nodiscard]] constexpr double latitude_degrees() const noexcept
  {
    return this->latitude.degrees();
  }

  [[nodiscard]] constexpr double longitude_degrees() const noexcept
  {
    return this->longitude.degrees();
  }

  [[nodiscard]] constexpr Real altitude_msl_meters() const noexcept
  {
    return this->altitude_msl.as_m();
  }

  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return this->latitude.is_finite() and this->longitude.is_finite()
       and this->altitude_msl.is_finite();
  }

  /**
   * @brief Checks whether the position is finite with its latitude between the poles.
   */
  [[nodiscard]] constexpr bool is_valid() const noexcept
  {
    return this->is_finite() and this->latitude.is_valid();
  }

  /**
   * @brief Returns the same point with its latitude between the poles: a latitude past a pole is
   * folded back and the longitude moves to the opposite meridian, so 100°N 30°E is 80°N 150°W.
   */
  [[nodiscard]] constexpr GeodeticPosition normalized() const noexcept
  {
    return {
      .latitude = this->latitude.folded(),
      .longitude = this->latitude.is_beyond_pole() ? this->longitude.opposite() : this->longitude,
      .altitude_msl = this->altitude_msl,
    };
  }

  /**
   * @brief Returns the point on the other side of the Earth through its centre, at the same
   * altitude.
   */
  [[nodiscard]] constexpr GeodeticPosition antipode() const noexcept
  {
    return {
      .latitude = -this->latitude,
      .longitude = this->longitude.opposite(),
      .altitude_msl = this->altitude_msl,
    };
  }

  [[nodiscard]] constexpr Quadrant quadrant() const noexcept
  {
    return common::quadrant(this->latitude, this->longitude);
  }

  /**
   * @brief Returns the UTM grid zone of the position, with the wider zones 32V around Norway and
   * 31X, 33X, 35X and 37X around Svalbard.
   * @return The zone, or nothing outside `[80°S, 84°N]`, where UPS replaces UTM.
   */
  [[nodiscard]] constexpr std::optional<UtmZone> utm_zone() const noexcept
  {
    auto const band = this->latitude.utm_band();
    auto const standard = this->longitude.utm_zone();
    if(not band.has_value() or not standard.has_value())
      return std::nullopt;
    auto const east = this->longitude.degrees();
    auto number = *standard;
    if(*band == 'V' and east >= 3.0 and east < 12.0)
      number = 32;
    else if(*band == 'X' and east >= 0.0 and east < 42.0)
      number = east < 9.0 ? 31 : east < 21.0 ? 33 : east < 33.0 ? 35 : 37;
    return UtmZone {.number = number, .band = *band};
  }

  /// @brief `<latitude>, <longitude>, <altitude> m MSL`, e.g. `59.771°N, 30.326°E, 70 m MSL`, at
  /// full precision.
  [[nodiscard]] std::string to_string() const;

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QString to_qstring() const;
#endif

  [[nodiscard]] constexpr bool operator==(GeodeticPosition const&) const noexcept = default;
};

W_COMMON_API std::ostream& operator<<(std::ostream& os, GeodeticPosition const& position);

namespace _geodesy_impl  // NOLINT(readability-identifier-naming)
{
  /// @brief A point or a direction in earth-centred, earth-fixed axes, always in `double`.
  struct Ecef
  {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
  };

  /// @brief The axes north, east and down of a local frame, in earth-centred, earth-fixed axes.
  struct NedAxes
  {
    Ecef north;
    Ecef east;
    Ecef down;
  };
}  // namespace _geodesy_impl

/**
 * @brief A local frame on the Earth: the plane tangent to the WGS84 ellipsoid at an origin, x
 * north, y east, z down along the normal of the origin (NED).
 * @details Horizontally the geometry is exact: a point of the plane has the latitude and
 * longitude of the ellipsoid normal passing through it. Against WGS84 geodesics from the origin,
 * the horizontal error grows as s³/3R²: 8 mm at 10 km, 1 m at 50 km, 8 m at 100 km, at any
 * latitude.
 *
 * Vertically the frame is flat: every vertical line of the plane keeps its latitude and
 * longitude, and the altitude is `origin.altitude_msl − z` everywhere. The plane rises above the
 * ellipsoid away from the origin (by s²/2R, 7.8 m at 10 km), which this ignores, so that
 * altitudes, terrain heights and a constant gravity along z agree with each other. The plane
 * passes through the origin at its altitude, so horizontal distances scale with the radius at
 * that altitude. Altitudes above mean sea level stand in for heights above the ellipsoid, which
 * only moves the plane by the geoid undulation (tens of metres, 10⁻⁵ of the scale).
 *
 * lla_from_ned() keeps the latitude and longitude of the positions it was asked for last, per
 * thread, so asking again for the same horizontal position is a lookup.
 *
 * Positions in the frame are `float` Vector3; the frame itself, its origin on the Earth and its
 * axes, is held and applied in `double`, so a position keeps its own resolution (half a
 * millimetre at 10 km) wherever the origin is.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto const plane = LocalTangentPlane({.latitude = 60.2_north, .longitude = 30.3_east});
 * auto const position = plane.lla_from_ned({10'000.0, 0.0, -150.0});  // 10 km north, 150 m up
 * auto const back = plane.ned_from_lla(position);
 * @endcode
 *
 * @ingroup core_geodesy
 * @see GeodeticPosition
 */
class W_COMMON_API LocalTangentPlane
{
 public:
  /// @brief The plane at latitude, longitude and altitude zero.
  LocalTangentPlane();

  /**
   * @throws InvalidArgumentError If the origin is not finite or its latitude is not strictly
   * between the poles.
   */
  explicit LocalTangentPlane(GeodeticPosition const& origin);

  [[nodiscard]] GeodeticPosition const& origin() const noexcept { return this->m_origin; }

  /**
   * @brief Latitude, longitude and altitude of a position in the plane's frame.
   * @param position_ned Metres north, east and down of the origin.
   * @throws InvalidArgumentError If the position is not finite.
   */
  [[nodiscard]] GeodeticPosition lla_from_ned(Vector3 const& position_ned) const;

  /**
   * @brief Position in the plane's frame of a geodetic position, the inverse of lla_from_ned().
   * @return Metres north, east and down of the origin.
   * @throws InvalidArgumentError If the position is not valid: not finite, or its latitude is
   * outside [-90°, 90°].
   * @throws NumericError If the position is a quarter of the globe or more away from the origin,
   * where its normal never meets the plane.
   */
  [[nodiscard]] Vector3 ned_from_lla(GeodeticPosition const& position) const;

  /// @brief Altitude above mean sea level of a position in the plane's frame.
  [[nodiscard]] Distance altitude_msl(Vector3 const& position_ned) const noexcept
  {
    return this->m_origin.altitude_msl - Distance::from_meters(position_ned.z());
  }

 private:
  GeodeticPosition m_origin;
  _geodesy_impl::Ecef m_origin_ecef;
  _geodesy_impl::NedAxes m_axes;
};

W_COMMON_NAMESPACE_END

#ifndef __DOXYGEN__
template <>
struct fmt::formatter<w::common::GeodeticPosition> : fmt::ostream_formatter
{};
#endif

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::GeodeticPosition)
#endif
