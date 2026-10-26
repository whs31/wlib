#pragma once

#include <string>
#include <string_view>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/quantity.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Distance value stored in meters.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
 * @details Distance is a small value type with common metric and imperial
 * conversions. Arithmetic combines the stored meter values directly, matching
 * the lightweight Rust type this class replaces. Dividing two distances yields
 * their dimensionless ratio.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto total = 1.2_km + 300_m;
 * fmt::print("{}\n", total); // "1.5 km"
 * auto ratio = 50_m / 10_m;  // 5.0
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Velocity
 */
class W_COMMON_API Distance : public QuantityBase<Distance>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double nanometers READ as_nm CONSTANT FINAL)
  Q_PROPERTY(double micrometers READ as_um CONSTANT FINAL)
  Q_PROPERTY(double millimeters READ as_mm CONSTANT FINAL)
  Q_PROPERTY(double centimeters READ as_cm CONSTANT FINAL)
  Q_PROPERTY(double decimeters READ as_dm CONSTANT FINAL)
  Q_PROPERTY(double meters READ as_m CONSTANT FINAL)
  Q_PROPERTY(double hectometers READ as_hm CONSTANT FINAL)
  Q_PROPERTY(double kilometers READ as_km CONSTANT FINAL)
  Q_PROPERTY(double inches READ as_inches CONSTANT FINAL)
  Q_PROPERTY(double feet READ as_feet CONSTANT FINAL)
  Q_PROPERTY(double yards READ as_yards CONSTANT FINAL)
  Q_PROPERTY(double furlongs READ as_furlongs CONSTANT FINAL)
  Q_PROPERTY(double miles READ as_miles CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "m";

  /**
   * @brief Creates a zero distance.
   */
  constexpr Distance() noexcept = default;

  /**
   * @brief Creates a distance from meters.
   */
  explicit constexpr Distance(Real const meters) noexcept
    : QuantityBase(meters)
  {}

  /**
   * @brief Creates a distance from meters.
   */
  [[nodiscard]] static constexpr Distance new_distance(Real const meters) noexcept
  {
    return Distance(meters);
  }

  /**
   * @brief Creates a distance from meters.
   */
  [[nodiscard]] static constexpr Distance from_meters(Real const meters) noexcept
  {
    return Distance(meters);
  }

  [[nodiscard]] static constexpr Distance from_nanometers(Real const nanometers) noexcept
  {
    return from_meters(nanometers / METER_NANOMETER_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_micrometers(Real const micrometers) noexcept
  {
    return from_meters(micrometers / METER_MICROMETER_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_millimeters(Real const millimeters) noexcept
  {
    return from_meters(millimeters / METER_MILLIMETER_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_centimeters(Real const centimeters) noexcept
  {
    return from_meters(centimeters / METER_CENTIMETER_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_decimeters(Real const decimeters) noexcept
  {
    return from_meters(decimeters / METER_DECIMETER_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_hectometers(Real const hectometers) noexcept
  {
    return from_meters(hectometers / METER_HECTOMETER_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_kilometers(Real const kilometers) noexcept
  {
    return from_meters(kilometers / METER_KILOMETER_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_inches(Real const inches) noexcept
  {
    return from_meters(inches / METER_INCH_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_feet(Real const feet) noexcept
  {
    return from_meters(feet / METER_FEET_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_yards(Real const yards) noexcept
  {
    return from_meters(yards / METER_YARD_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_furlongs(Real const furlongs) noexcept
  {
    return from_meters(furlongs / METER_FURLONG_FACTOR);
  }

  [[nodiscard]] static constexpr Distance from_miles(Real const miles) noexcept
  {
    return from_meters(miles / METER_MILE_FACTOR);
  }

  [[nodiscard]] constexpr Real as_nm() const noexcept
  {
    return this->magnitude() * METER_NANOMETER_FACTOR;
  }

  [[nodiscard]] constexpr Real as_um() const noexcept
  {
    return this->magnitude() * METER_MICROMETER_FACTOR;
  }

  [[nodiscard]] constexpr Real as_mm() const noexcept
  {
    return this->magnitude() * METER_MILLIMETER_FACTOR;
  }

  [[nodiscard]] constexpr Real as_cm() const noexcept
  {
    return this->magnitude() * METER_CENTIMETER_FACTOR;
  }

  [[nodiscard]] constexpr Real as_dm() const noexcept
  {
    return this->magnitude() * METER_DECIMETER_FACTOR;
  }

  [[nodiscard]] constexpr Real as_m() const noexcept { return this->magnitude(); }

  [[nodiscard]] constexpr Real as_hm() const noexcept
  {
    return this->magnitude() * METER_HECTOMETER_FACTOR;
  }

  [[nodiscard]] constexpr Real as_km() const noexcept
  {
    return this->magnitude() * METER_KILOMETER_FACTOR;
  }

  [[nodiscard]] constexpr Real as_inches() const noexcept
  {
    return this->magnitude() * METER_INCH_FACTOR;
  }

  [[nodiscard]] constexpr Real as_feet() const noexcept
  {
    return this->magnitude() * METER_FEET_FACTOR;
  }

  [[nodiscard]] constexpr Real as_yards() const noexcept
  {
    return this->magnitude() * METER_YARD_FACTOR;
  }

  [[nodiscard]] constexpr Real as_furlongs() const noexcept
  {
    return this->magnitude() * METER_FURLONG_FACTOR;
  }

  [[nodiscard]] constexpr Real as_miles() const noexcept
  {
    return this->magnitude() * METER_MILE_FACTOR;
  }

  /**
   * @brief Returns a compact string using millimeters, centimeters, meters, or kilometers.
   */
  // Replaces the SI display of QuantityBase::to_string().
  // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
  [[nodiscard]] std::string to_string() const;

  using QuantityBase::operator+=;
  using QuantityBase::operator-=;

  constexpr Distance& operator+=(Real const meters) noexcept { return *this += Distance(meters); }

  constexpr Distance& operator-=(Real const meters) noexcept { return *this -= Distance(meters); }
};

[[nodiscard]] constexpr Distance operator+(Distance lhs, Real const meters) noexcept
{
  lhs += meters;
  return lhs;
}

[[nodiscard]] constexpr Distance operator+(Real const meters, Distance const rhs) noexcept
{
  return Distance(meters) + rhs;
}

[[nodiscard]] constexpr Distance operator-(Distance lhs, Real const meters) noexcept
{
  lhs -= meters;
  return lhs;
}

[[nodiscard]] constexpr Distance operator-(Real const meters, Distance const rhs) noexcept
{
  return Distance(meters) - rhs;
}

inline namespace literals
{
  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_meters(long double const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_meters(unsigned long long const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_kilometers(long double const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_kilometers(unsigned long long const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_m(long double const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_m(unsigned long long const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_km(long double const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_km(unsigned long long const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a centimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_cm(long double const value) noexcept
  {
    return Distance::from_centimeters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a centimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_cm(unsigned long long const value) noexcept
  {
    return Distance::from_centimeters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a millimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_mm(long double const value) noexcept
  {
    return Distance::from_millimeters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a millimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_mm(unsigned long long const value) noexcept
  {
    return Distance::from_millimeters(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Distance)
#endif
