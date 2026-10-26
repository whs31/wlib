#pragma once

#include <concepts>
#include <chrono>
#include <numbers>
#include <string_view>

#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Angular velocity value stored in `rad/s`.
 * @details AngularVelocity is a small value type with radian, degree and revolution
 * rates. It relates to Angle through `std::chrono` durations.
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
template <std::floating_point T>
class BasicAngularVelocity : public QuantityBase<BasicAngularVelocity<T>, T>
{
  using Base = QuantityBase<BasicAngularVelocity, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "rad/s";

  /**
   * @brief Creates a zero angular velocity.
   */
  constexpr BasicAngularVelocity() noexcept = default;

  /**
   * @brief Creates an angular velocity from radians per second.
   */
  explicit constexpr BasicAngularVelocity(T const radians_per_second) noexcept
    : Base(radians_per_second)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicAngularVelocity(
    BasicAngularVelocity<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates an angular velocity from radians per second.
   */
  [[nodiscard]] static constexpr BasicAngularVelocity from_radians_per_second(
    T const radians_per_second
  ) noexcept
  {
    return BasicAngularVelocity(radians_per_second);
  }

  /**
   * @brief Creates an angular velocity from degrees per second.
   */
  [[nodiscard]] static constexpr BasicAngularVelocity from_degrees_per_second(
    T const degrees_per_second
  ) noexcept
  {
    return BasicAngularVelocity(to_radians(degrees_per_second));
  }

  /**
   * @brief Creates an angular velocity from revolutions per minute.
   */
  [[nodiscard]] static constexpr BasicAngularVelocity from_rpm(T const rpm) noexcept
  {
    return BasicAngularVelocity(
      rpm * (2 * std::numbers::pi_v<T>) / static_cast<T>(SECONDS_MINUTES_FACTOR)
    );
  }

  /**
   * @brief Returns radians per second.
   */
  [[nodiscard]] constexpr T as_radians_per_second() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns degrees per second.
   */
  [[nodiscard]] constexpr T as_degrees_per_second() const noexcept
  {
    return to_degrees(this->magnitude());
  }

  /**
   * @brief Returns revolutions per minute.
   */
  [[nodiscard]] constexpr T as_rpm() const noexcept
  {
    return this->magnitude() * static_cast<T>(SECONDS_MINUTES_FACTOR) / (2 * std::numbers::pi_v<T>);
  }
};

/**
 * @brief AngularVelocity in single precision, the default.
 * @ingroup core_measurements
 */
using AngularVelocity = BasicAngularVelocity<float>;

/**
 * @brief Returns the average angular velocity that changes an angle by @p delta in @p time.
 * @ingroup core_measurements
 * @relates AngularVelocity
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicAngularVelocity<T> operator/(
  BasicAngle<T> const delta,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicAngularVelocity<T>::from_radians_per_second(
    static_cast<T>(delta.radians() / std::chrono::duration<double>(time).count())
  );
}

/**
 * @brief Returns the change of an angle at @p rate during @p time.
 * @ingroup core_measurements
 * @relates AngularVelocity
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicAngle<T> operator*(
  BasicAngularVelocity<T> const rate,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicAngle<T>::from_radians(
    static_cast<T>(rate.as_radians_per_second() * std::chrono::duration<double>(time).count())
  );
}

/**
 * @brief Returns the change of an angle at @p rate during @p time.
 * @ingroup core_measurements
 * @relates AngularVelocity
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicAngle<T> operator*(
  std::chrono::duration<Rep, Period> const time,
  BasicAngularVelocity<T> const rate
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
