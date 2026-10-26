#pragma once

#include <cmath>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/distance.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/wgs84.h>
#include <w/common/detail/export.h>
#include <w/common/detail/geographic.h>
#include <w/common/detail/namespace.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Geodetic latitude on the WGS84 ellipsoid, positive north, stored in degrees as a
 * `double` whatever the `precision` option of the library.
 * @details A `float` resolves a coordinate on the Earth to about a metre; a `double` to
 * nanometres. Degrees are stored, not radians, so a latitude keeps the exact digits it was written
 * with, e.g. in a configuration file, and prints them back.
 *
 * A latitude is a position, not an amount, so it is not a quantity: latitudes are never added
 * together or scaled. They form an affine space over Angle instead:
 *   - `Latitude ± Angle -> Latitude`, moving along a meridian;
 *   - `Latitude − Latitude -> Angle`, the signed difference, positive northward;
 *   - `<` and `>` compare north and south.
 *
 * A latitude may hold any value, so that arithmetic and generic code stay total. A latitude
 * between the poles, `[-90°, 90°]`, is_valid(); clamped() stops one at the pole it passed, and
 * folded() continues it down the opposite meridian, which GeodeticPosition::normalized() applies
 * to a whole position. parse() and the glaze adapter accept valid latitudes only.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto const pulkovo = 59.771_north;  // or 59.771_lat
 * auto const sydney = 33.86_south;    // or -33.86_lat
 * auto const span = pulkovo - sydney;  // Angle, 93.631 deg
 * pulkovo.hemisphere();                // Latitude::Hemisphere::North
 * pulkovo.to_dms_string();             // 59°46'15.60"N
 * Latitude::parse("59°46'15.6\"N");    // 59.771_north, as is "59.771north"
 * @endcode
 *
 * @note Qt properties, QString conversion, and metatype registration are available only when wlib
 * is built with `with_qt=True`.
 * @ingroup core_geodesy
 * @see Longitude
 * @see GeodeticPosition
 */
class W_COMMON_API Latitude
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double degrees READ degrees CONSTANT FINAL)
  Q_PROPERTY(double radians READ radians CONSTANT FINAL)
  Q_PROPERTY(Hemisphere hemisphere READ hemisphere CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(bool isValid READ is_valid CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
  Q_PROPERTY(QString dmsString READ to_dms_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Side of the equator. The equator itself counts as north.
   */
  enum class Hemisphere : std::uint8_t
  {
    North,
    South,
  };
#ifdef W_COMMON_WITH_QT
  Q_ENUM(Hemisphere)
#endif

  /**
   * @brief Sexagesimal form of a latitude: whole degrees, whole minutes, seconds and the
   * hemisphere.
   */
  struct Dms
  {
    int degrees = 0;
    int minutes = 0;
    double seconds = 0.0;
    Hemisphere hemisphere = Hemisphere::North;

    [[nodiscard]] constexpr bool operator==(Dms const&) const noexcept = default;
  };

  /**
   * @brief Creates the equator.
   */
  constexpr Latitude() noexcept = default;

  [[nodiscard]] static constexpr Latitude from_degrees(double const degrees) noexcept
  {
    return Latitude(degrees);
  }

  [[nodiscard]] static constexpr Latitude from_radians(double const radians) noexcept
  {
    return Latitude(to_degrees(radians));
  }

  /**
   * @brief Creates a latitude from its degrees, minutes and seconds; the hemisphere gives the
   * sign.
   */
  [[nodiscard]] static constexpr Latitude from_dms(Dms const& dms) noexcept
  {
    auto const magnitude = dms.degrees + dms.minutes / 60.0 + dms.seconds / 3'600.0;
    return Latitude(dms.hemisphere == Hemisphere::South ? -magnitude : magnitude);
  }

  /**
   * @brief Creates a latitude from an angle north of the equator.
   */
  [[nodiscard]] static constexpr Latitude from_angle(Angle const angle) noexcept
  {
    return Latitude(_geographic_impl::degrees_of(angle));
  }

  /**
   * @brief Reads a latitude written by hand.
   * @details Accepts signed decimal degrees, `-33.86`, with an optional degree sign or `deg`;
   * a hemisphere before or after the number instead of the sign, as a letter or a word, as the
   * literals spell it: `33.86S`, `S 33.86`, `50north`, `33.86 South`; degrees and decimal minutes,
   * `33°51.6'S`; degrees, minutes and seconds, `33°51'36"S`, `33 51 36 S`, `33:51:36S` or
   * `33d51m36s S`. Hemispheres are N, S, north and south in any case. Minutes and seconds are
   * below 60 and follow whole degrees and minutes.
   * @return The latitude, or nothing if @p text is none of these or is outside `[-90°, 90°]`.
   */
  [[nodiscard]] static std::optional<Latitude> parse(std::string_view text) noexcept;

  [[nodiscard]] static constexpr Latitude equator() noexcept { return {}; }

  [[nodiscard]] static constexpr Latitude north_pole() noexcept { return Latitude(90.0); }

  [[nodiscard]] static constexpr Latitude south_pole() noexcept { return Latitude(-90.0); }

  [[nodiscard]] constexpr double degrees() const noexcept { return this->m_degrees; }

  [[nodiscard]] constexpr double radians() const noexcept { return to_radians(this->m_degrees); }

  /**
   * @brief Returns the latitude as an angle north of the equator, in the precision of the
   * library.
   */
  [[nodiscard]] constexpr Angle to_angle() const noexcept
  {
    return _geographic_impl::angle_of_degrees(this->m_degrees);
  }

  /**
   * @brief Returns the angle from the north pole, `90° − latitude`.
   */
  [[nodiscard]] constexpr Angle colatitude() const noexcept
  {
    return _geographic_impl::angle_of_degrees(90.0 - this->m_degrees);
  }

  /**
   * @brief Returns the sexagesimal form of the latitude, unrounded.
   */
  [[nodiscard]] constexpr Dms to_dms() const noexcept
  {
    auto const magnitude = common::abs(this->m_degrees);
    // whole degrees beyond int are no latitude; the seconds then carry it, or NaN
    if(not(magnitude < max_dms_degrees))
      return {.seconds = magnitude * 3'600.0, .hemisphere = this->hemisphere()};
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
    return this->m_degrees < 0.0 ? Hemisphere::South : Hemisphere::North;
  }

  /**
   * @brief Returns `'N'` or `'S'`.
   */
  [[nodiscard]] constexpr char hemisphere_letter() const noexcept
  {
    return this->hemisphere() == Hemisphere::South ? 'S' : 'N';
  }

  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_degrees);
  }

  /**
   * @brief Checks whether the latitude lies between the poles, `[-90°, 90°]`.
   */
  [[nodiscard]] constexpr bool is_valid() const noexcept
  {
    return this->m_degrees >= -90.0 and this->m_degrees <= 90.0;
  }

  /**
   * @brief Checks whether following the meridian to this latitude passes a pole, so that the
   * point lies on the opposite meridian at folded().
   */
  [[nodiscard]] constexpr bool is_beyond_pole() const noexcept
  {
    return this->is_finite()
       and common::abs(_geographic_impl::wrapped_degrees(this->m_degrees)) > 90.0;
  }

  /**
   * @brief Returns the latitude limited to the pole it passed.
   */
  [[nodiscard]] constexpr Latitude clamped() const noexcept
  {
    if(this->m_degrees > 90.0)
      return north_pole();
    if(this->m_degrees < -90.0)
      return south_pole();
    return *this;
  }

  /**
   * @brief Returns the latitude reached by following the meridian past the poles, in
   * `[-90°, 90°]`: 100° is 80° on the opposite meridian, 200° is −20° on the same one.
   * @details A latitude between the poles is returned unchanged. is_beyond_pole() tells whether
   * the meridian flips, which GeodeticPosition::normalized() applies to the longitude.
   */
  [[nodiscard]] constexpr Latitude folded() const noexcept
  {
    if(this->is_valid() or not this->is_finite())
      return *this;
    auto const wrapped = _geographic_impl::wrapped_degrees(this->m_degrees);
    if(wrapped > 90.0)
      return Latitude(180.0 - wrapped);
    if(wrapped < -90.0)
      return Latitude(-180.0 - wrapped);
    return Latitude(wrapped);
  }

  /**
   * @brief Returns the distance from the equator as a northern latitude.
   */
  [[nodiscard]] constexpr Latitude abs() const noexcept
  {
    return Latitude(common::abs(this->m_degrees));
  }

  /**
   * @brief Interpolates along the meridian toward @p other.
   */
  [[nodiscard]] constexpr Latitude lerp(Latitude const other, double const t) const noexcept
  {
    return Latitude(this->m_degrees + (other.m_degrees - this->m_degrees) * t);
  }

  [[nodiscard]] constexpr Latitude midpoint(Latitude const other) const noexcept
  {
    return this->lerp(other, 0.5);
  }

  /**
   * @brief Returns the geocentric latitude, the angle between the equator and the line from the
   * centre of the Earth to the point on the ellipsoid, `tan ψ = (1 − e²) tan φ`.
   */
  [[nodiscard]] Latitude geocentric() const noexcept
  {
    return from_radians(std::atan2((1.0 - wgs84::eccentricity_squared) * this->sin(), this->cos()));
  }

  /**
   * @brief Returns the geodetic latitude of a geocentric one, the inverse of geocentric().
   */
  [[nodiscard]] static Latitude from_geocentric(Latitude const geocentric) noexcept
  {
    return from_radians(
      std::atan2(geocentric.sin(), (1.0 - wgs84::eccentricity_squared) * geocentric.cos())
    );
  }

  /**
   * @brief Returns the parametric (reduced) latitude, the latitude on the sphere circumscribing
   * the ellipsoid of the point with the same distance from the axis, `tan β = (1 − f) tan φ`.
   */
  [[nodiscard]] Latitude parametric() const noexcept
  {
    return from_radians(std::atan2((1.0 - wgs84::flattening) * this->sin(), this->cos()));
  }

  /**
   * @brief Returns the geodetic latitude of a parametric one, the inverse of parametric().
   */
  [[nodiscard]] static Latitude from_parametric(Latitude const parametric) noexcept
  {
    return from_radians(std::atan2(parametric.sin(), (1.0 - wgs84::flattening) * parametric.cos()));
  }

  /**
   * @brief Returns the WGS84 radius of curvature along the meridian, `M`: a short arc of the
   * meridian is `M dφ` long.
   */
  [[nodiscard]] Distance meridional_radius() const noexcept
  {
    auto const prime_vertical = this->prime_vertical_radius_meters();
    return Distance::from_meters(
      static_cast<Real>(
        prime_vertical * prime_vertical * prime_vertical * (1.0 - wgs84::eccentricity_squared)
        / (wgs84::semi_major_axis * wgs84::semi_major_axis)
      )
    );
  }

  /**
   * @brief Returns the WGS84 radius of curvature across the meridian, `N`, the distance along the
   * normal from the surface to the polar axis.
   */
  [[nodiscard]] Distance prime_vertical_radius() const noexcept
  {
    return Distance::from_meters(static_cast<Real>(this->prime_vertical_radius_meters()));
  }

  /**
   * @brief Returns the radius of the parallel on the WGS84 ellipsoid, `N cos φ`: a short arc of
   * the parallel is `N cos φ dλ` long.
   */
  [[nodiscard]] Distance parallel_radius() const noexcept
  {
    return Distance::from_meters(
      static_cast<Real>(this->prime_vertical_radius_meters() * this->cos())
    );
  }

  /**
   * @brief Returns the UTM latitude band, `'C'` to `'X'` without `'I'` and `'O'`, 8° tall from
   * 80°S, the last one 12°.
   * @return The band, or nothing outside `[-80°, 84°]`, where UPS replaces UTM.
   */
  [[nodiscard]] constexpr std::optional<char> utm_band() const noexcept
  {
    if(not(this->m_degrees >= -80.0 and this->m_degrees <= 84.0))
      return std::nullopt;
    auto constexpr bands = std::string_view("CDEFGHJKLMNPQRSTUVWX");
    auto const band = static_cast<std::size_t>(floor((this->m_degrees + 80.0) / 8.0));
    return bands.at(band < bands.size() ? band : bands.size() - 1);
  }

  /**
   * @brief Returns decimal degrees with the hemisphere, `59.771°N`, at full precision. The
   * equator has no hemisphere: `0°`.
   */
  [[nodiscard]] std::string to_string() const;

  /**
   * @brief Returns degrees, minutes and seconds with the hemisphere, `59°46'15.60"N`.
   * @param second_decimals Decimals of the seconds, 0 to 9; 2 resolve 0.3 m.
   */
  [[nodiscard]] std::string to_dms_string(int second_decimals = 2) const;

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QString to_qstring() const;

  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QString to_dms_qstring() const;
#endif

  /**
   * @brief Mirrors the latitude across the equator.
   */
  [[nodiscard]] constexpr Latitude operator-() const noexcept { return Latitude(-this->m_degrees); }

  constexpr Latitude& operator+=(Angle const angle) noexcept
  {
    this->m_degrees += _geographic_impl::degrees_of(angle);
    return *this;
  }

  constexpr Latitude& operator-=(Angle const angle) noexcept
  {
    this->m_degrees -= _geographic_impl::degrees_of(angle);
    return *this;
  }

  [[nodiscard]] friend constexpr Latitude operator+(Latitude lhs, Angle const rhs) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr Latitude operator+(Angle const lhs, Latitude rhs) noexcept
  {
    rhs += lhs;
    return rhs;
  }

  [[nodiscard]] friend constexpr Latitude operator-(Latitude lhs, Angle const rhs) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  /**
   * @brief Returns the angle from @p rhs to @p lhs along the meridian, positive northward.
   */
  [[nodiscard]] friend constexpr Angle operator-(Latitude const lhs, Latitude const rhs) noexcept
  {
    return _geographic_impl::angle_of_degrees(lhs.m_degrees - rhs.m_degrees);
  }

  [[nodiscard]] constexpr bool operator==(Latitude const&) const noexcept = default;

  /**
   * @brief Orders latitudes from south to north.
   */
  [[nodiscard]] constexpr std::partial_ordering operator<=>(Latitude const&) const noexcept =
    default;

  friend std::ostream& operator<<(std::ostream& os, Latitude const& value)
  {
    return os << value.to_string();
  }

  /**
   * @brief Formats the latitude with `fmt` as its to_string().
   */
  [[nodiscard]] friend std::string format_as(Latitude const& value) { return value.to_string(); }

 private:
  explicit constexpr Latitude(double const degrees) noexcept
    : m_degrees(degrees)
  {}

  [[nodiscard]] double prime_vertical_radius_meters() const noexcept
  {
    auto const sin = this->sin();
    return wgs84::semi_major_axis / std::sqrt(1.0 - wgs84::eccentricity_squared * sin * sin);
  }

  static constexpr double max_dms_degrees = 1e9;

  double m_degrees = 0.0;
};

inline namespace literals
{
  /**
   * @brief Creates a latitude from signed decimal degrees, positive north.
   * @ingroup core_geodesy
   * @relates Latitude
   */
  [[nodiscard]] constexpr Latitude operator""_lat(long double const value) noexcept
  {
    return Latitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a latitude from signed decimal degrees, positive north.
   * @ingroup core_geodesy
   * @relates Latitude
   */
  [[nodiscard]] constexpr Latitude operator""_lat(unsigned long long const value) noexcept
  {
    return Latitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a latitude from degrees north of the equator.
   * @ingroup core_geodesy
   * @relates Latitude
   */
  [[nodiscard]] constexpr Latitude operator""_north(long double const value) noexcept
  {
    return Latitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a latitude from degrees north of the equator.
   * @ingroup core_geodesy
   * @relates Latitude
   */
  [[nodiscard]] constexpr Latitude operator""_north(unsigned long long const value) noexcept
  {
    return Latitude::from_degrees(static_cast<double>(value));
  }

  /**
   * @brief Creates a latitude from degrees south of the equator.
   * @ingroup core_geodesy
   * @relates Latitude
   */
  [[nodiscard]] constexpr Latitude operator""_south(long double const value) noexcept
  {
    return Latitude::from_degrees(-static_cast<double>(value));
  }

  /**
   * @brief Creates a latitude from degrees south of the equator.
   * @ingroup core_geodesy
   * @relates Latitude
   */
  [[nodiscard]] constexpr Latitude operator""_south(unsigned long long const value) noexcept
  {
    return Latitude::from_degrees(-static_cast<double>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Latitude>
{
  [[nodiscard]] std::size_t operator()(w::common::Latitude const value) const noexcept
  {
    return std::hash<double> {}(value.degrees());
  }
};

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Latitude)
#endif
