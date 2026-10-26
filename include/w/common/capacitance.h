#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Capacitance value stored in `F`.
 * @details Capacitance is a small value type with farad and microfarad conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto bus = 470_uf; // 0.00047 F
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Charge
 * @see Voltage
 */
template <std::floating_point T>
class BasicCapacitance : public QuantityBase<BasicCapacitance<T>, T>
{
  using Base = QuantityBase<BasicCapacitance, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "F";

  /**
   * @brief Creates a zero capacitance.
   */
  constexpr BasicCapacitance() noexcept = default;

  /**
   * @brief Creates a capacitance from farads.
   */
  explicit constexpr BasicCapacitance(T const farads) noexcept
    : Base(farads)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicCapacitance(
    BasicCapacitance<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a capacitance from farads.
   */
  [[nodiscard]] static constexpr BasicCapacitance from_farads(T const farads) noexcept
  {
    return BasicCapacitance(farads);
  }

  /**
   * @brief Creates a capacitance from microfarads.
   */
  [[nodiscard]] static constexpr BasicCapacitance from_microfarads(T const microfarads) noexcept
  {
    return BasicCapacitance(microfarads / 1'000'000);
  }

  /**
   * @brief Returns farads.
   */
  [[nodiscard]] constexpr T as_farads() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns microfarads.
   */
  [[nodiscard]] constexpr T as_microfarads() const noexcept
  {
    return this->magnitude() * 1'000'000;
  }
};

/**
 * @brief Capacitance in single precision, the default.
 * @ingroup core_measurements
 */
using Capacitance = BasicCapacitance<float>;

inline namespace literals
{
  /**
   * @brief Creates a capacitance from a farad literal.
   * @ingroup core_measurements
   * @relates Capacitance
   */
  [[nodiscard]] constexpr Capacitance operator""_f(long double const value) noexcept
  {
    return Capacitance::from_farads(static_cast<Real>(value));
  }

  /**
   * @brief Creates a capacitance from a farad literal.
   * @ingroup core_measurements
   * @relates Capacitance
   */
  [[nodiscard]] constexpr Capacitance operator""_f(unsigned long long const value) noexcept
  {
    return Capacitance::from_farads(static_cast<Real>(value));
  }

  /**
   * @brief Creates a capacitance from a microfarad literal.
   * @ingroup core_measurements
   * @relates Capacitance
   */
  [[nodiscard]] constexpr Capacitance operator""_uf(long double const value) noexcept
  {
    return Capacitance::from_microfarads(static_cast<Real>(value));
  }

  /**
   * @brief Creates a capacitance from a microfarad literal.
   * @ingroup core_measurements
   * @relates Capacitance
   */
  [[nodiscard]] constexpr Capacitance operator""_uf(unsigned long long const value) noexcept
  {
    return Capacitance::from_microfarads(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
