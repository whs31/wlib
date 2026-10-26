#pragma once

#include <concepts>
#include <chrono>
#include <string_view>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>
#include <w/common/velocity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Acceleration value stored in `m/s^2`.
 * @details Acceleration is a small value type with meters-per-second-squared and standard
 * gravity (g₀ = 9.80665 m/s²) conversions. It relates to Velocity through `std::chrono`
 * durations.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * using namespace std::chrono_literals;
 *
 * auto gravity = 1_gn;            // 9.80665 m/s^2
 * auto fall_speed = gravity * 2s; // 19.6133 m/s
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Velocity
 * @see Force
 */
template <std::floating_point T>
class BasicAcceleration : public QuantityBase<BasicAcceleration<T>, T>
{
  using Base = QuantityBase<BasicAcceleration, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "m/s^2";

  /**
   * @brief Creates a zero acceleration.
   */
  constexpr BasicAcceleration() noexcept = default;

  /**
   * @brief Creates an acceleration from meters per second squared.
   */
  explicit constexpr BasicAcceleration(T const mps2) noexcept
    : Base(mps2)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicAcceleration(
    BasicAcceleration<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates an acceleration from meters per second squared.
   */
  [[nodiscard]] static constexpr BasicAcceleration from_mps2(T const mps2) noexcept
  {
    return BasicAcceleration(mps2);
  }

  /**
   * @brief Creates an acceleration from multiples of standard gravity.
   */
  [[nodiscard]] static constexpr BasicAcceleration from_standard_gravity(
    T const standard_gravity
  ) noexcept
  {
    return BasicAcceleration(standard_gravity * static_cast<T>(STANDARD_GRAVITY));
  }

  /**
   * @brief Returns meters per second squared.
   */
  [[nodiscard]] constexpr T as_mps2() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns multiples of standard gravity.
   */
  [[nodiscard]] constexpr T as_standard_gravity() const noexcept
  {
    return this->magnitude() / static_cast<T>(STANDARD_GRAVITY);
  }
};

/**
 * @brief Acceleration in single precision, the default.
 * @ingroup core_measurements
 */
using Acceleration = BasicAcceleration<float>;

/**
 * @brief Returns the average acceleration that changes a velocity by @p delta in @p time.
 * @ingroup core_measurements
 * @relates Acceleration
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicAcceleration<T> operator/(
  BasicVelocity<T> const delta,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicAcceleration<T>::from_mps2(
    static_cast<T>(delta.as_mps() / std::chrono::duration<double>(time).count())
  );
}

/**
 * @brief Returns the change of a velocity at @p rate during @p time.
 * @ingroup core_measurements
 * @relates Acceleration
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicVelocity<T> operator*(
  BasicAcceleration<T> const rate,
  std::chrono::duration<Rep, Period> const time
) noexcept
{
  return BasicVelocity<T>::from_mps(
    static_cast<T>(rate.as_mps2() * std::chrono::duration<double>(time).count())
  );
}

/**
 * @brief Returns the change of a velocity at @p rate during @p time.
 * @ingroup core_measurements
 * @relates Acceleration
 */
template <std::floating_point T, typename Rep, typename Period>
[[nodiscard]] constexpr BasicVelocity<T> operator*(
  std::chrono::duration<Rep, Period> const time,
  BasicAcceleration<T> const rate
) noexcept
{
  return rate * time;
}

inline namespace literals
{
  /**
   * @brief Creates an acceleration from a meters-per-second-squared literal.
   * @ingroup core_measurements
   * @relates Acceleration
   */
  [[nodiscard]] constexpr Acceleration operator""_mps2(long double const value) noexcept
  {
    return Acceleration::from_mps2(static_cast<Real>(value));
  }

  /**
   * @brief Creates an acceleration from a meters-per-second-squared literal.
   * @ingroup core_measurements
   * @relates Acceleration
   */
  [[nodiscard]] constexpr Acceleration operator""_mps2(unsigned long long const value) noexcept
  {
    return Acceleration::from_mps2(static_cast<Real>(value));
  }

  /**
   * @brief Creates an acceleration from a standard-gravity literal.
   * @ingroup core_measurements
   * @relates Acceleration
   */
  [[nodiscard]] constexpr Acceleration operator""_gn(long double const value) noexcept
  {
    return Acceleration::from_standard_gravity(static_cast<Real>(value));
  }

  /**
   * @brief Creates an acceleration from a standard-gravity literal.
   * @ingroup core_measurements
   * @relates Acceleration
   */
  [[nodiscard]] constexpr Acceleration operator""_gn(unsigned long long const value) noexcept
  {
    return Acceleration::from_standard_gravity(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
