#pragma once

#include <concepts>
#include <chrono>
#include <string_view>
#include <type_traits>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/namespace.h>
#include <w/common/distance.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Velocity value stored in meters per second.
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
template <std::floating_point T>
class BasicVelocity : public QuantityBase<BasicVelocity<T>, T>
{
  using Base = QuantityBase<BasicVelocity, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "m/s";

  /**
   * @brief Creates a zero velocity.
   */
  constexpr BasicVelocity() noexcept = default;

  /**
   * @brief Creates a velocity from meters per second.
   */
  explicit constexpr BasicVelocity(T const meters_per_second) noexcept
    : Base(meters_per_second)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicVelocity(
    BasicVelocity<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a velocity from meters per second.
   */
  [[nodiscard]] static constexpr BasicVelocity from_mps(T const meters_per_second) noexcept
  {
    return BasicVelocity(meters_per_second);
  }

  /**
   * @brief Creates a velocity from kilometers per hour.
   */
  [[nodiscard]] static constexpr BasicVelocity from_kmph(T const kilometers_per_hour) noexcept
  {
    return from_mps(
      (kilometers_per_hour / static_cast<T>(METER_KILOMETER_FACTOR))
      / static_cast<T>(SECONDS_HOURS_FACTOR)
    );
  }

  /**
   * @brief Creates a velocity from miles per hour.
   */
  [[nodiscard]] static constexpr BasicVelocity from_mph(T const miles_per_hour) noexcept
  {
    return from_mps(
      (miles_per_hour / static_cast<T>(METER_MILE_FACTOR)) / static_cast<T>(SECONDS_HOURS_FACTOR)
    );
  }

  /**
   * @brief Returns meters per second.
   */
  [[nodiscard]] constexpr T as_mps() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns kilometers per hour.
   */
  [[nodiscard]] constexpr T as_kmph() const noexcept
  {
    return (this->magnitude() * static_cast<T>(METER_KILOMETER_FACTOR))
         * static_cast<T>(SECONDS_HOURS_FACTOR);
  }

  /**
   * @brief Returns miles per hour.
   */
  [[nodiscard]] constexpr T as_mph() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_MILE_FACTOR)
         * static_cast<T>(SECONDS_HOURS_FACTOR);
  }

  using Base::operator+=;
  using Base::operator-=;

  constexpr BasicVelocity& operator+=(T const meters_per_second) noexcept
  {
    return *this += BasicVelocity(meters_per_second);
  }

  constexpr BasicVelocity& operator-=(T const meters_per_second) noexcept
  {
    return *this -= BasicVelocity(meters_per_second);
  }
};

/**
 * @brief Velocity in single precision, the default.
 * @ingroup core_measurements
 */
using Velocity = BasicVelocity<float>;

template <std::floating_point T>
[[nodiscard]] constexpr BasicVelocity<T> operator+(
  BasicVelocity<T> lhs,
  std::type_identity_t<T> const meters_per_second
) noexcept
{
  lhs += meters_per_second;
  return lhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicVelocity<T> operator+(
  std::type_identity_t<T> const meters_per_second,
  BasicVelocity<T> const rhs
) noexcept
{
  return BasicVelocity<T>(meters_per_second) + rhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicVelocity<T> operator-(
  BasicVelocity<T> lhs,
  std::type_identity_t<T> const meters_per_second
) noexcept
{
  lhs -= meters_per_second;
  return lhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicVelocity<T> operator-(
  std::type_identity_t<T> const meters_per_second,
  BasicVelocity<T> const rhs
) noexcept
{
  return BasicVelocity<T>(meters_per_second) - rhs;
}

/**
 * @brief Returns the average velocity that covers @p distance in @p time.
 * @ingroup core_measurements
 * @relates Velocity
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicVelocity<T> operator/(
  BasicDistance<T> const distance,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicVelocity<T>::from_mps(
    static_cast<T>(distance.as_m() / std::chrono::duration<double>(time).count())
  );
}

/**
 * @brief Returns the distance covered at @p velocity during @p time.
 * @ingroup core_measurements
 * @relates Velocity
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicDistance<T> operator*(
  BasicVelocity<T> const velocity,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicDistance<T>::from_meters(
    static_cast<T>(velocity.as_mps() * std::chrono::duration<double>(time).count())
  );
}

/**
 * @brief Returns the distance covered at @p velocity during @p time.
 * @ingroup core_measurements
 * @relates Velocity
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicDistance<T> operator*(
  std::chrono::duration<Rep, Period> const time,
  BasicVelocity<T> const velocity
) noexcept
{
  return velocity * time;
}

/**
 * @brief Returns the time it takes to cover @p distance at @p velocity.
 * @ingroup core_measurements
 * @relates Velocity
 */
template <std::floating_point T>
[[nodiscard]] constexpr std::chrono::duration<T> operator/(
  BasicDistance<T> const distance,
  BasicVelocity<T> const velocity
) noexcept
{
  return std::chrono::duration<T>(distance.as_m() / velocity.as_mps());
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
