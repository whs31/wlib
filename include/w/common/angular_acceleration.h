#pragma once

#include <concepts>
#include <chrono>
#include <string_view>

#include <w/common/angular_velocity.h>
#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Angular acceleration value stored in `rad/s^2`.
 * @details AngularAcceleration is a small value type with radian and degree rates. It
 * relates to AngularVelocity through `std::chrono` durations.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * using namespace std::chrono_literals;
 *
 * auto spin_up = 10_rad_per_s2 * 3s; // 30 rad/s
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see AngularVelocity
 * @see Torque
 * @see MomentOfInertia
 */
template <std::floating_point T>
class BasicAngularAcceleration : public QuantityBase<BasicAngularAcceleration<T>, T>
{
  using Base = QuantityBase<BasicAngularAcceleration, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "rad/s^2";

  /**
   * @brief Creates a zero angular acceleration.
   */
  constexpr BasicAngularAcceleration() noexcept = default;

  /**
   * @brief Creates an angular acceleration from radians per second squared.
   */
  explicit constexpr BasicAngularAcceleration(T const radians_per_second_squared) noexcept
    : Base(radians_per_second_squared)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicAngularAcceleration(
    BasicAngularAcceleration<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates an angular acceleration from radians per second squared.
   */
  [[nodiscard]] static constexpr BasicAngularAcceleration from_radians_per_second_squared(
    T const radians_per_second_squared
  ) noexcept
  {
    return BasicAngularAcceleration(radians_per_second_squared);
  }

  /**
   * @brief Creates an angular acceleration from degrees per second squared.
   */
  [[nodiscard]] static constexpr BasicAngularAcceleration from_degrees_per_second_squared(
    T const degrees_per_second_squared
  ) noexcept
  {
    return BasicAngularAcceleration(to_radians(degrees_per_second_squared));
  }

  /**
   * @brief Returns radians per second squared.
   */
  [[nodiscard]] constexpr T as_radians_per_second_squared() const noexcept
  {
    return this->magnitude();
  }

  /**
   * @brief Returns degrees per second squared.
   */
  [[nodiscard]] constexpr T as_degrees_per_second_squared() const noexcept
  {
    return to_degrees(this->magnitude());
  }
};

/**
 * @brief AngularAcceleration in single precision, the default.
 * @ingroup core_measurements
 */
using AngularAcceleration = BasicAngularAcceleration<float>;

/**
 * @brief Returns the average angular acceleration that changes an angular velocity by @p delta in
 * @p time.
 * @ingroup core_measurements
 * @relates AngularAcceleration
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicAngularAcceleration<T> operator/(
  BasicAngularVelocity<T> const delta,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicAngularAcceleration<T>::from_radians_per_second_squared(
    static_cast<T>(delta.as_radians_per_second() / std::chrono::duration<double>(time).count())
  );
}

/**
 * @brief Returns the change of an angular velocity at @p rate during @p time.
 * @ingroup core_measurements
 * @relates AngularAcceleration
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicAngularVelocity<T> operator*(
  BasicAngularAcceleration<T> const rate,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicAngularVelocity<T>::from_radians_per_second(
    static_cast<T>(
      rate.as_radians_per_second_squared() * std::chrono::duration<double>(time).count()
    )
  );
}

/**
 * @brief Returns the change of an angular velocity at @p rate during @p time.
 * @ingroup core_measurements
 * @relates AngularAcceleration
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicAngularVelocity<T> operator*(
  std::chrono::duration<Rep, Period> const time,
  BasicAngularAcceleration<T> const rate
) noexcept
{
  return rate * time;
}

inline namespace literals
{
  /**
   * @brief Creates an angular acceleration from a radian-per-second-squared literal.
   * @ingroup core_measurements
   * @relates AngularAcceleration
   */
  [[nodiscard]] constexpr AngularAcceleration operator""_rad_per_s2(
    long double const value
  ) noexcept
  {
    return AngularAcceleration::from_radians_per_second_squared(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular acceleration from a radian-per-second-squared literal.
   * @ingroup core_measurements
   * @relates AngularAcceleration
   */
  [[nodiscard]] constexpr AngularAcceleration operator""_rad_per_s2(
    unsigned long long const value
  ) noexcept
  {
    return AngularAcceleration::from_radians_per_second_squared(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular acceleration from a degree-per-second-squared literal.
   * @ingroup core_measurements
   * @relates AngularAcceleration
   */
  [[nodiscard]] constexpr AngularAcceleration operator""_deg_per_s2(
    long double const value
  ) noexcept
  {
    return AngularAcceleration::from_degrees_per_second_squared(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angular acceleration from a degree-per-second-squared literal.
   * @ingroup core_measurements
   * @relates AngularAcceleration
   */
  [[nodiscard]] constexpr AngularAcceleration operator""_deg_per_s2(
    unsigned long long const value
  ) noexcept
  {
    return AngularAcceleration::from_degrees_per_second_squared(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
