#pragma once

#include <chrono>
#include <numbers>
#include <string_view>

#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/quantity.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Angular velocity value stored in `rad/s`.
 * @details AngularVelocity is a small value type with radian, degree and revolution
 * rates. It relates to Angle through `std::chrono` durations.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * using namespace std::chrono_literals;
 *
 * auto rotor = 300_rpm;
 * auto turned = 90_deg_per_s * 2s; // 180 deg
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Angle
 * @see AngularAcceleration
 */
class W_COMMON_API AngularVelocity : public QuantityBase<AngularVelocity>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double radiansPerSecond READ as_radians_per_second CONSTANT FINAL)
  Q_PROPERTY(double degreesPerSecond READ as_degrees_per_second CONSTANT FINAL)
  Q_PROPERTY(double revolutionsPerMinute READ as_rpm CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "rad/s";

  /**
   * @brief Creates a zero angular velocity.
   */
  constexpr AngularVelocity() noexcept = default;

  /**
   * @brief Creates an angular velocity from radians per second.
   */
  explicit constexpr AngularVelocity(Real const radians_per_second) noexcept
    : QuantityBase(radians_per_second)
  {}

  /**
   * @brief Creates an angular velocity from radians per second.
   */
  [[nodiscard]] static constexpr AngularVelocity from_radians_per_second(
    Real const radians_per_second
  ) noexcept
  {
    return AngularVelocity(radians_per_second);
  }

  /**
   * @brief Creates an angular velocity from degrees per second.
   */
  [[nodiscard]] static constexpr AngularVelocity from_degrees_per_second(
    Real const degrees_per_second
  ) noexcept
  {
    return AngularVelocity(to_radians(degrees_per_second));
  }

  /**
   * @brief Creates an angular velocity from revolutions per minute.
   */
  [[nodiscard]] static constexpr AngularVelocity from_rpm(Real const rpm) noexcept
  {
    return AngularVelocity(rpm * (2 * std::numbers::pi_v<Real>) / SECONDS_MINUTES_FACTOR);
  }

  /**
   * @brief Returns radians per second.
   */
  [[nodiscard]] constexpr Real as_radians_per_second() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns degrees per second.
   */
  [[nodiscard]] constexpr Real as_degrees_per_second() const noexcept
  {
    return to_degrees(this->magnitude());
  }

  /**
   * @brief Returns revolutions per minute.
   */
  [[nodiscard]] constexpr Real as_rpm() const noexcept
  {
    return this->magnitude() * SECONDS_MINUTES_FACTOR / (2 * std::numbers::pi_v<Real>);
  }
};

/**
 * @brief Returns the average angular velocity that changes an angle by @p delta in @p time.
 * @ingroup core_measurements
 * @relates AngularVelocity
 */
template <typename Rep, typename Period>
[[nodiscard]] constexpr AngularVelocity operator/(
  Angle const delta,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return AngularVelocity::from_radians_per_second(
    delta.radians() / std::chrono::duration<Real>(time).count()
  );
}

/**
 * @brief Returns the change of an angle at @p rate during @p time.
 * @ingroup core_measurements
 * @relates AngularVelocity
 */
template <typename Rep, typename Period>
[[nodiscard]] constexpr Angle operator*(
  AngularVelocity const rate,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return Angle::from_radians(
    rate.as_radians_per_second() * std::chrono::duration<Real>(time).count()
  );
}

/**
 * @brief Returns the change of an angle at @p rate during @p time.
 * @ingroup core_measurements
 * @relates AngularVelocity
 */
template <typename Rep, typename Period>
[[nodiscard]] constexpr Angle operator*(
  std::chrono::duration<Rep, Period> const time,
  AngularVelocity const rate
) noexcept
{
  return rate * time;
}

inline namespace literals
{
  /**
   * @brief Creates an angular velocity from a radian-per-second literal.
   * @ingroup core_measurements
   * @relates AngularVelocity
   */
  [[nodiscard]] constexpr AngularVelocity operator""_rad_per_s(long double const value) noexcept
  {
    return AngularVelocity::from_radians_per_second(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular velocity from a radian-per-second literal.
   * @ingroup core_measurements
   * @relates AngularVelocity
   */
  [[nodiscard]] constexpr AngularVelocity operator""_rad_per_s(
    unsigned long long const value
  ) noexcept
  {
    return AngularVelocity::from_radians_per_second(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular velocity from a degree-per-second literal.
   * @ingroup core_measurements
   * @relates AngularVelocity
   */
  [[nodiscard]] constexpr AngularVelocity operator""_deg_per_s(long double const value) noexcept
  {
    return AngularVelocity::from_degrees_per_second(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular velocity from a degree-per-second literal.
   * @ingroup core_measurements
   * @relates AngularVelocity
   */
  [[nodiscard]] constexpr AngularVelocity operator""_deg_per_s(
    unsigned long long const value
  ) noexcept
  {
    return AngularVelocity::from_degrees_per_second(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular velocity from a revolution-per-minute literal.
   * @ingroup core_measurements
   * @relates AngularVelocity
   */
  [[nodiscard]] constexpr AngularVelocity operator""_rpm(long double const value) noexcept
  {
    return AngularVelocity::from_rpm(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular velocity from a revolution-per-minute literal.
   * @ingroup core_measurements
   * @relates AngularVelocity
   */
  [[nodiscard]] constexpr AngularVelocity operator""_rpm(unsigned long long const value) noexcept
  {
    return AngularVelocity::from_rpm(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::AngularVelocity)
#endif
