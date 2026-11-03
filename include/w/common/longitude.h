#pragma once

#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/detail/export.h>
#include <w/common/detail/geographic.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Longitude on the WGS84 ellipsoid, positive east of Greenwich, always in `[-180°, 180°)`,
 * stored in degrees as a `double`.
 * @details Every longitude names one meridian, so a longitude wraps into `[-180°, 180°)` when it
 * is created: 190° is −170°, and the antimeridian is −180°. A longitude already in range keeps its
 * exact digits. A non-finite longitude stays non-finite.
 *
 * A longitude is a position on a circle, not an amount, so it is not a quantity. Longitudes form
 * an affine space over Angle, the long way around never taken:
 *   - `Longitude ± Angle -> Longitude`, moving along a parallel and wrapping;
 *   - `Longitude − Longitude -> Angle`, the shortest signed difference, positive eastward, in
 *     `[-180°, 180°)`;
 *   - is_east_of(), is_west_of() and is_between() compare along the shorter arc instead of `<`,
 *     which has no meaning on a circle.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto const tokyo = 139.69_east;           // or 139.69_lon
 * auto const honolulu = 157.86_west;        // or -157.86_lon
 * auto const flight = honolulu - tokyo;     // Angle, 62.45 deg east across the antimeridian
 * tokyo.is_between(100_east, 170_west);     // true, the range crosses the antimeridian
 * (179_east + 3_deg).degrees();             // -178
 * tokyo.utm_zone();                         // 54
 * @endcode
 *
 * @ingroup core_geodesy
 * @see Latitude
 * @see GeodeticPosition
 */
class W_COMMON_API Longitude
{
 public:
  /**
   * @brief Side of the prime meridian. The prime meridian counts as east, the antimeridian,
   * −180°, as west.
   */
  enum class Hemisphere : std::uint8_t
  {
    East,
    West,
  };

  /**
   * @brief Sexagesimal form of a longitude: whole degrees, whole minutes, seconds and the
   * hemisphere.
   */
  struct Dms
  {
    int degrees = 0;
    int minutes = 0;
    double seconds = 0.0;
    Hemisphere hemisphere = Hemisphere::East;

    [[nodiscard]] constexpr bool operator==(Dms const&) const noexcept = default;
  };

  /**
   * @brief Creates the prime meridian.
   */
  constexpr Longitude() noexcept = default;

  /**
   * @brief Creates a longitude from degrees east, wrapped into `[-180°, 180°)`.
   */
  [[nodiscard]] static constexpr Longitude from_degrees(double const degrees) noexcept
  {
    return Longitude(degrees);
  }

  /**
   * @brief Creates a longitude from radians east, wrapped into `[-180°, 180°)`.
   */
  [[nodiscard]] static constexpr Longitude from_radians(double const radians) noexcept
  {
    return Longitude(to_degrees(radians));
  }

  /**
   * @brief Creates a longitude from its degrees, minutes and seconds; the hemisphere gives the
   * sign.
   */
  [[nodiscard]] static constexpr Longitude from_dms(Dms const& dms) noexcept
  {
    auto const magnitude = dms.degrees + dms.minutes / 60.0 + dms.seconds / 3'600.0;
    return Longitude(dms.hemisphere == Hemisphere::West ? -magnitude : magnitude);
  }

  /**
   * @brief Creates a longitude from an angle east of the prime meridian.
   */
  [[nodiscard]] static constexpr Longitude from_angle(Angle const angle) noexcept
  {
    return Longitude(_geographic_impl::degrees_of(angle));
  }

  /**
   * @brief Reads a longitude written by hand.
   * @details Accepts the forms of Latitude::parse() with the hemispheres E, W, east and west:
   * `-157.86`, `157.86W`, `W 157.86`, `157.86west`, `157°51.6'W`, `157°51'36"W`, `157 51 36 W`.
   * @return The longitude, or nothing if @p text is none of these or is outside `[-180°, 180°]`.
   */
  [[nodiscard]] static std::optional<Longitude> parse(std::string_view text) noexcept;

  [[nodiscard]] static constexpr Longitude prime_meridian() noexcept { return {}; }

  [[nodiscard]] static constexpr Longitude antimeridian() noexcept { return Longitude(-180.0); }

  /**
   * @brief Returns degrees east in `[-180, 180)`.
   */
  [[nodiscard]] constexpr double degrees() const noexcept { return this->m_degrees; }

  /**
   * @brief Returns degrees east in `[0, 360)`, as some gridded datasets count them.
   */
  [[nodiscard]] constexpr double degrees_east() const noexcept
  {
    return _geographic_impl::wrapped_positive_degrees(this->m_degrees);
  }

  [[nodiscard]] constexpr double radians() const noexcept { return to_radians(this->m_degrees); }

  /**
   * @brief Returns the longitude as an angle east of the prime meridian, in the precision of the
   * library.
   */
  [[nodiscard]] constexpr Angle to_angle() const noexcept
  {
    return _geographic_impl::angle_of_degrees(this->m_degrees);
  }

  /**
   * @brief Returns the sexagesimal form of the longitude, unrounded.
   */
  [[nodiscard]] constexpr Dms to_dms() const noexcept
  {
    if(not this->is_finite())
      return {.seconds = this->m_degrees, .hemisphere = this->hemisphere()};
    auto const magnitude = common::abs(this->m_degrees);
    auto const whole = floor(magnitude);
    auto const minutes = (magnitude - whole) * 60.0;
    auto const whole_minutes = floor(minutes);
    return {
      .degrees = static_cast<int>(whole),
      .minutes = static_cast<int>(whole_minutes),
      .seconds = (minutes - whole_minutes) * 60.0,
      .hemisphere = this->hemisphere(),
    };
  }

  [[nodiscard]] double sin() const noexcept { return std::sin(this->radians()); }

  [[nodiscard]] double cos() const noexcept { return std::cos(this->radians()); }

  [[nodiscard]] double tan() const noexcept { return std::tan(this->radians()); }

  [[nodiscard]] constexpr Hemisphere hemisphere() const noexcept
  {
    return this->m_degrees < 0.0 ? Hemisphere::West : Hemisphere::East;
  }

  /**
   * @brief Returns `'E'` or `'W'`.
   */
  [[nodiscard]] constexpr char hemisphere_letter() const noexcept
  {
    return this->hemisphere() == Hemisphere::West ? 'W' : 'E';
  }

  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_degrees);
  }

  /**
   * @brief Returns the meridian on the other side of the Earth, 180° away.
   */
  [[nodiscard]] constexpr Longitude opposite() const noexcept
  {
    return Longitude(this->m_degrees + 180.0);
  }

  /**
   * @brief Returns the shortest turn from this longitude to @p other, positive eastward, in
   * `[-180°, 180°)`; opposite meridians are 180° west.
   */
  [[nodiscard]] constexpr Angle angle_to(Longitude const other) const noexcept
  {
    return _geographic_impl::angle_of_degrees(
      _geographic_impl::wrapped_degrees(other.m_degrees - this->m_degrees)
    );
  }

  /**
   * @brief Checks whether this longitude is east of @p other along the shorter arc.
   */
  [[nodiscard]] constexpr bool is_east_of(Longitude const other) const noexcept
  {
    return _geographic_impl::wrapped_degrees(this->m_degrees - other.m_degrees) > 0.0;
  }

  /**
   * @brief Checks whether this longitude is west of @p other along the shorter arc; of opposite
   * meridians, either is west of the other.
   */
  [[nodiscard]] constexpr bool is_west_of(Longitude const other) const noexcept
  {
    return _geographic_impl::wrapped_degrees(this->m_degrees - other.m_degrees) < 0.0;
  }

  /**
   * @brief Checks whether this longitude lies on the arc going east from @p west to @p east,
   * both included.
   * @details The arc may cross the antimeridian: 170°E to 170°W is 20° wide. Equal ends make an
   * arc of their meridian alone.
   */
  [[nodiscard]] constexpr bool is_between(Longitude const west, Longitude const east) const noexcept
  {
    return _geographic_impl::wrapped_positive_degrees(this->m_degrees - west.m_degrees)
        <= _geographic_impl::wrapped_positive_degrees(east.m_degrees - west.m_degrees);
  }

  /**
   * @brief Interpolates toward @p other along the shorter arc.
   */
  [[nodiscard]] constexpr Longitude lerp(Longitude const other, double const t) const noexcept
  {
    return Longitude(
      this->m_degrees + _geographic_impl::wrapped_degrees(other.m_degrees - this->m_degrees) * t
    );
  }

  /**
   * @brief Returns the meridian halfway along the shorter arc to @p other.
   */
  [[nodiscard]] constexpr Longitude midpoint(Longitude const other) const noexcept
  {
    return this->lerp(other, 0.5);
  }

  /**
   * @brief Returns how far local mean solar time runs ahead of Greenwich: four minutes a degree.
   */
  [[nodiscard]] constexpr std::chrono::duration<double> mean_solar_time_offset() const noexcept
  {
    return std::chrono::duration<double>(this->m_degrees * 240.0);
  }

  /**
   * @brief Returns the offset from UTC of the nautical time zone of the meridian, 15° wide and
   * centered on a multiple of 15°: −12 to +12 hours, positive east.
   * @details Boundaries belong to the zone east of them.
   */
  [[nodiscard]] constexpr std::chrono::hours nautical_time_zone() const noexcept
  {
    if(not this->is_finite())
      return std::chrono::hours::zero();
    return std::chrono::hours(static_cast<int>(floor((this->m_degrees + 7.5) / 15.0)));
  }

  /**
   * @brief Returns the standard UTM zone, 1 to 60, each 6° wide eastward from the antimeridian.
   * @details GeodeticPosition::utm_zone() also applies the exceptions around Norway and Svalbard.
   * @return The zone, or nothing for a non-finite longitude.
   */
  [[nodiscard]] constexpr std::optional<int> utm_zone() const noexcept
  {
    if(not this->is_finite())
      return std::nullopt;
    auto const zone = static_cast<int>(floor((this->m_degrees + 180.0) / 6.0)) + 1;
    return zone > 60 ? 60 : zone;
  }

  /**
   * @brief Returns decimal degrees with the hemisphere, `30.326°E`, at full precision. The prime
   * meridian and the antimeridian have no hemisphere: `0°` and `180°`.
   */
  [[nodiscard]] std::string to_string() const;

  /**
   * @brief Returns degrees, minutes and seconds with the hemisphere, `30°19'33.60"E`.
   * @param second_decimals Decimals of the seconds, 0 to 9.
   */
  [[nodiscard]] std::string to_dms_string(int second_decimals = 2) const;

  /**
   * @brief Mirrors the longitude across the prime meridian.
   */
  [[nodiscard]] constexpr Longitude operator-() const noexcept
  {
    return Longitude(-this->m_degrees);
  }

  constexpr Longitude& operator+=(Angle const angle) noexcept
  {
    *this = Longitude(this->m_degrees + _geographic_impl::degrees_of(angle));
    return *this;
  }

  constexpr Longitude& operator-=(Angle const angle) noexcept
  {
    *this = Longitude(this->m_degrees - _geographic_impl::degrees_of(angle));
    return *this;
  }

  [[nodiscard]] friend constexpr Longitude operator+(Longitude lhs, Angle const rhs) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr Longitude operator+(Angle const lhs, Longitude rhs) noexcept
  {
    rhs += lhs;
    return rhs;
  }

  [[nodiscard]] friend constexpr Longitude operator-(Longitude lhs, Angle const rhs) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  /**
   * @brief Returns the shortest turn from @p rhs to @p lhs, `rhs.angle_to(lhs)`.
   */
  [[nodiscard]] friend constexpr Angle operator-(Longitude const lhs, Longitude const rhs) noexcept
  {
    return rhs.angle_to(lhs);
  }

  [[nodiscard]] constexpr bool operator==(Longitude const&) const noexcept = default;

  friend std::ostream& operator<<(std::ostream& os, Longitude const& value)
  {
    return os << value.to_string();
  }

  /**
   * @brief Formats the longitude with `fmt` as its to_string().
   */
  [[nodiscard]] friend std::string format_as(Longitude const& value) { return value.to_string(); }

 private:
  explicit constexpr Longitude(double const degrees) noexcept
    : m_degrees(_geographic_impl::wrapped_degrees(degrees))
  {}

  double m_degrees = 0.0;
};

inline namespace literals
{
  /**
   * @brief Creates a longitude from signed decimal degrees, positive east.
   * @ingroup core_geodesy
   * @relates Longitude
   */
  [[nodiscard]] constexpr Longitude operator""_lon(long double const value) noexcept
  {
    return Longitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a longitude from signed decimal degrees, positive east.
   * @ingroup core_geodesy
   * @relates Longitude
   */
  [[nodiscard]] constexpr Longitude operator""_lon(unsigned long long const value) noexcept
  {
    return Longitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a longitude from degrees east of the prime meridian.
   * @ingroup core_geodesy
   * @relates Longitude
   */
  [[nodiscard]] constexpr Longitude operator""_east(long double const value) noexcept
  {
    return Longitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a longitude from degrees east of the prime meridian.
   * @ingroup core_geodesy
   * @relates Longitude
   */
  [[nodiscard]] constexpr Longitude operator""_east(unsigned long long const value) noexcept
  {
    return Longitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a longitude from degrees west of the prime meridian.
   * @ingroup core_geodesy
   * @relates Longitude
   */
  [[nodiscard]] constexpr Longitude operator""_west(long double const value) noexcept
  {
    return Longitude::from_degrees(-static_cast<double>(value));
  }

  /**
   * @brief Creates a longitude from degrees west of the prime meridian.
   * @ingroup core_geodesy
   * @relates Longitude
   */
  [[nodiscard]] constexpr Longitude operator""_west(unsigned long long const value) noexcept
  {
    return Longitude::from_degrees(-static_cast<double>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Longitude>
{
  [[nodiscard]] std::size_t operator()(w::common::Longitude const value) const noexcept
  {
    return std::hash<double> {}(value.degrees());
  }
};
