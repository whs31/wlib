#pragma once

#include <chrono>
#include <string_view>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/distance.h>
#include <w/common/quantity.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Velocity value stored in meters per second.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
 * @details Velocity is a small value type with conversions for meters per
 * second, kilometers per hour, and miles per hour. It relates to Distance
 * through `std::chrono` durations.
 *
 * @code {.cpp}
 * using namespace std::chrono_literals;
 * using namespace w::literals;
 *
 * auto cruise = w::common::Velocity::from_kmph(90.0);
 * auto text = cruise.to_string();  // "25 m/s"
 * auto travelled = cruise * 2s;    // 50 m
 * auto speed = 100_m / 4s;         // 25 m/s
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Distance
 * @see Acceleration
 */
class W_COMMON_API Velocity : public QuantityBase<Velocity>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double metersPerSecond READ as_mps CONSTANT FINAL)
  Q_PROPERTY(double kilometersPerHour READ as_kmph CONSTANT FINAL)
  Q_PROPERTY(double milesPerHour READ as_mph CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "m/s";

  /**
   * @brief Creates a zero velocity.
   */
  constexpr Velocity() noexcept = default;

  /**
   * @brief Creates a velocity from meters per second.
   */
  explicit constexpr Velocity(Real const meters_per_second) noexcept
    : QuantityBase(meters_per_second)
  {}

  /**
   * @brief Creates a velocity from meters per second.
   */
  [[nodiscard]] static constexpr Velocity from_mps(Real const meters_per_second) noexcept
  {
    return Velocity(meters_per_second);
  }

  /**
   * @brief Creates a velocity from kilometers per hour.
   */
  [[nodiscard]] static constexpr Velocity from_kmph(Real const kilometers_per_hour) noexcept
  {
    return from_mps((kilometers_per_hour / METER_KILOMETER_FACTOR) / SECONDS_HOURS_FACTOR);
  }

  /**
   * @brief Creates a velocity from miles per hour.
   */
  [[nodiscard]] static constexpr Velocity from_mph(Real const miles_per_hour) noexcept
  {
    return from_mps((miles_per_hour / METER_MILE_FACTOR) / SECONDS_HOURS_FACTOR);
  }

  /**
   * @brief Returns meters per second.
   */
  [[nodiscard]] constexpr Real as_mps() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns kilometers per hour.
   */
  [[nodiscard]] constexpr Real as_kmph() const noexcept
  {
    return (this->magnitude() * METER_KILOMETER_FACTOR) * SECONDS_HOURS_FACTOR;
  }

  /**
   * @brief Returns miles per hour.
   */
  [[nodiscard]] constexpr Real as_mph() const noexcept
  {
    return this->magnitude() * METER_MILE_FACTOR * SECONDS_HOURS_FACTOR;
  }

  using QuantityBase::operator+=;
  using QuantityBase::operator-=;

  constexpr Velocity& operator+=(Real const meters_per_second) noexcept
  {
    return *this += Velocity(meters_per_second);
  }

  constexpr Velocity& operator-=(Real const meters_per_second) noexcept
  {
    return *this -= Velocity(meters_per_second);
  }
};

[[nodiscard]] constexpr Velocity operator+(Velocity lhs, Real const meters_per_second) noexcept
{
  lhs += meters_per_second;
  return lhs;
}

[[nodiscard]] constexpr Velocity operator+(
  Real const meters_per_second,
  Velocity const rhs
) noexcept
{
  return Velocity(meters_per_second) + rhs;
}

[[nodiscard]] constexpr Velocity operator-(Velocity lhs, Real const meters_per_second) noexcept
{
  lhs -= meters_per_second;
  return lhs;
}

[[nodiscard]] constexpr Velocity operator-(
  Real const meters_per_second,
  Velocity const rhs
) noexcept
{
  return Velocity(meters_per_second) - rhs;
}

/**
 * @brief Returns the average velocity that covers @p distance in @p time.
 * @ingroup core_measurements
 * @relates Velocity
 */
template <typename Rep, typename Period>
[[nodiscard]] constexpr Velocity operator/(
  Distance const distance,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return Velocity::from_mps(distance.as_m() / std::chrono::duration<Real>(time).count());
}

/**
 * @brief Returns the distance covered at @p velocity during @p time.
 * @ingroup core_measurements
 * @relates Velocity
 */
template <typename Rep, typename Period>
[[nodiscard]] constexpr Distance operator*(
  Velocity const velocity,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return Distance::from_meters(velocity.as_mps() * std::chrono::duration<Real>(time).count());
}

/**
 * @brief Returns the distance covered at @p velocity during @p time.
 * @ingroup core_measurements
 * @relates Velocity
 */
template <typename Rep, typename Period>
[[nodiscard]] constexpr Distance operator*(
  std::chrono::duration<Rep, Period> const time,
  Velocity const velocity
) noexcept
{
  return velocity * time;
}

/**
 * @brief Returns the time it takes to cover @p distance at @p velocity.
 * @ingroup core_measurements
 * @relates Velocity
 */
[[nodiscard]] constexpr std::chrono::duration<Real> operator/(
  Distance const distance,
  Velocity const velocity
) noexcept
{
  return std::chrono::duration<Real>(distance.as_m() / velocity.as_mps());
}

inline namespace literals
{
  /**
   * @brief Creates a velocity from a meters-per-second literal.
   * @ingroup core_measurements
   * @relates Velocity
   */
  [[nodiscard]] constexpr Velocity operator""_mps(long double const value) noexcept
  {
    return Velocity::from_mps(static_cast<Real>(value));
  }

  /**
   * @brief Creates a velocity from a meters-per-second literal.
   * @ingroup core_measurements
   * @relates Velocity
   */
  [[nodiscard]] constexpr Velocity operator""_mps(unsigned long long const value) noexcept
  {
    return Velocity::from_mps(static_cast<Real>(value));
  }

  /**
   * @brief Creates a velocity from a kilometers-per-hour literal.
   * @ingroup core_measurements
   * @relates Velocity
   */
  [[nodiscard]] constexpr Velocity operator""_kmph(long double const value) noexcept
  {
    return Velocity::from_kmph(static_cast<Real>(value));
  }

  /**
   * @brief Creates a velocity from a kilometers-per-hour literal.
   * @ingroup core_measurements
   * @relates Velocity
   */
  [[nodiscard]] constexpr Velocity operator""_kmph(unsigned long long const value) noexcept
  {
    return Velocity::from_kmph(static_cast<Real>(value));
  }

  /**
   * @brief Creates a velocity from a miles-per-hour literal.
   * @ingroup core_measurements
   * @relates Velocity
   */
  [[nodiscard]] constexpr Velocity operator""_mph(long double const value) noexcept
  {
    return Velocity::from_mph(static_cast<Real>(value));
  }

  /**
   * @brief Creates a velocity from a miles-per-hour literal.
   * @ingroup core_measurements
   * @relates Velocity
   */
  [[nodiscard]] constexpr Velocity operator""_mph(unsigned long long const value) noexcept
  {
    return Velocity::from_mph(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Velocity)
#endif
