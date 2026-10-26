#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Force value stored in `N`.
 * @details Force is a small value type with newton, kilonewton and kilogram-force conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto thrust = 1.2_kn - 20_kgf; // 1003.87 N
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Mass
 * @see Torque
 * @see Pressure
 */
template <std::floating_point T>
class BasicForce : public QuantityBase<BasicForce<T>, T>
{
  using Base = QuantityBase<BasicForce, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "N";

  /**
   * @brief Creates a zero force.
   */
  constexpr BasicForce() noexcept = default;

  /**
   * @brief Creates a force from newtons.
   */
  explicit constexpr BasicForce(T const newtons) noexcept
    : Base(newtons)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicForce(BasicForce<U> const other) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a force from newtons.
   */
  [[nodiscard]] static constexpr BasicForce from_newtons(T const newtons) noexcept
  {
    return BasicForce(newtons);
  }

  /**
   * @brief Creates a force from kilonewtons.
   */
  [[nodiscard]] static constexpr BasicForce from_kilonewtons(T const kilonewtons) noexcept
  {
    return BasicForce(kilonewtons * 1'000);
  }

  /**
   * @brief Creates a force from kilograms-force.
   */
  [[nodiscard]] static constexpr BasicForce from_kilograms_force(T const kilograms_force) noexcept
  {
    return BasicForce(kilograms_force * static_cast<T>(STANDARD_GRAVITY));
  }

  /**
   * @brief Returns newtons.
   */
  [[nodiscard]] constexpr T as_newtons() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns kilonewtons.
   */
  [[nodiscard]] constexpr T as_kilonewtons() const noexcept { return this->magnitude() / 1'000; }

  /**
   * @brief Returns kilograms-force.
   */
  [[nodiscard]] constexpr T as_kilograms_force() const noexcept
  {
    return this->magnitude() / static_cast<T>(STANDARD_GRAVITY);
  }
};

/**
 * @brief Force in single precision, the default.
 * @ingroup core_measurements
 */
using Force = BasicForce<float>;

inline namespace literals
{
  /**
   * @brief Creates a force from a newton literal.
   * @ingroup core_measurements
   * @relates Force
   */
  [[nodiscard]] constexpr Force operator""_n(long double const value) noexcept
  {
    return Force::from_newtons(static_cast<Real>(value));
  }

  /**
   * @brief Creates a force from a newton literal.
   * @ingroup core_measurements
   * @relates Force
   */
  [[nodiscard]] constexpr Force operator""_n(unsigned long long const value) noexcept
  {
    return Force::from_newtons(static_cast<Real>(value));
  }

  /**
   * @brief Creates a force from a kilonewton literal.
   * @ingroup core_measurements
   * @relates Force
   */
  [[nodiscard]] constexpr Force operator""_kn(long double const value) noexcept
  {
    return Force::from_kilonewtons(static_cast<Real>(value));
  }

  /**
   * @brief Creates a force from a kilonewton literal.
   * @ingroup core_measurements
   * @relates Force
   */
  [[nodiscard]] constexpr Force operator""_kn(unsigned long long const value) noexcept
  {
    return Force::from_kilonewtons(static_cast<Real>(value));
  }

  /**
   * @brief Creates a force from a kilogram-force literal.
   * @ingroup core_measurements
   * @relates Force
   */
  [[nodiscard]] constexpr Force operator""_kgf(long double const value) noexcept
  {
    return Force::from_kilograms_force(static_cast<Real>(value));
  }

  /**
   * @brief Creates a force from a kilogram-force literal.
   * @ingroup core_measurements
   * @relates Force
   */
  [[nodiscard]] constexpr Force operator""_kgf(unsigned long long const value) noexcept
  {
    return Force::from_kilograms_force(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
