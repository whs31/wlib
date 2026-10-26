#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Torque value stored in `N*m`.
 * @details Torque is a small value type stored in newton-meters.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto load = 1.5_nm * 2.0; // 3 N*m
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Force
 * @see AngularAcceleration
 * @see MomentOfInertia
 */
template <std::floating_point T>
class BasicTorque : public QuantityBase<BasicTorque<T>, T>
{
  using Base = QuantityBase<BasicTorque, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "N*m";

  /**
   * @brief Creates a zero torque.
   */
  constexpr BasicTorque() noexcept = default;

  /**
   * @brief Creates a torque from newton-meters.
   */
  explicit constexpr BasicTorque(T const newton_meters) noexcept
    : Base(newton_meters)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicTorque(BasicTorque<U> const other) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a torque from newton-meters.
   */
  [[nodiscard]] static constexpr BasicTorque from_newton_meters(T const newton_meters) noexcept
  {
    return BasicTorque(newton_meters);
  }

  /**
   * @brief Returns newton-meters.
   */
  [[nodiscard]] constexpr T as_newton_meters() const noexcept { return this->magnitude(); }
};

/**
 * @brief Torque in single precision, the default.
 * @ingroup core_measurements
 */
using Torque = BasicTorque<float>;

inline namespace literals
{
  /**
   * @brief Creates a torque from a newton-meter literal.
   * @ingroup core_measurements
   * @relates Torque
   */
  [[nodiscard]] constexpr Torque operator""_nm(long double const value) noexcept
  {
    return Torque::from_newton_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a torque from a newton-meter literal.
   * @ingroup core_measurements
   * @relates Torque
   */
  [[nodiscard]] constexpr Torque operator""_nm(unsigned long long const value) noexcept
  {
    return Torque::from_newton_meters(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
