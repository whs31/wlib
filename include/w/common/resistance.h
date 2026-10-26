#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Resistance value stored in `ohm`.
 * @details Resistance is a small value type with ohm and milliohm conversions. The
 * `_mohm` literal is a milliohm, not a megaohm.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto winding = 45_mohm; // 0.045 ohm
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Conductance
 * @see Voltage
 * @see Current
 */
template <std::floating_point T>
class BasicResistance : public QuantityBase<BasicResistance<T>, T>
{
  using Base = QuantityBase<BasicResistance, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "ohm";

  /**
   * @brief Creates a zero resistance.
   */
  constexpr BasicResistance() noexcept = default;

  /**
   * @brief Creates a resistance from ohms.
   */
  explicit constexpr BasicResistance(T const ohms) noexcept
    : Base(ohms)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicResistance(
    BasicResistance<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a resistance from ohms.
   */
  [[nodiscard]] static constexpr BasicResistance from_ohms(T const ohms) noexcept
  {
    return BasicResistance(ohms);
  }

  /**
   * @brief Creates a resistance from milliohms.
   */
  [[nodiscard]] static constexpr BasicResistance from_milliohms(T const milliohms) noexcept
  {
    return BasicResistance(milliohms / 1'000);
  }

  /**
   * @brief Returns ohms.
   */
  [[nodiscard]] constexpr T as_ohms() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns milliohms.
   */
  [[nodiscard]] constexpr T as_milliohms() const noexcept { return this->magnitude() * 1'000; }
};

/**
 * @brief Resistance in single precision, the default.
 * @ingroup core_measurements
 */
using Resistance = BasicResistance<float>;

inline namespace literals
{
  /**
   * @brief Creates a resistance from an ohm literal.
   * @ingroup core_measurements
   * @relates Resistance
   */
  [[nodiscard]] constexpr Resistance operator""_ohm(long double const value) noexcept
  {
    return Resistance::from_ohms(static_cast<Real>(value));
  }

  /**
   * @brief Creates a resistance from an ohm literal.
   * @ingroup core_measurements
   * @relates Resistance
   */
  [[nodiscard]] constexpr Resistance operator""_ohm(unsigned long long const value) noexcept
  {
    return Resistance::from_ohms(static_cast<Real>(value));
  }

  /**
   * @brief Creates a resistance from a milliohm literal.
   * @ingroup core_measurements
   * @relates Resistance
   */
  [[nodiscard]] constexpr Resistance operator""_mohm(long double const value) noexcept
  {
    return Resistance::from_milliohms(static_cast<Real>(value));
  }

  /**
   * @brief Creates a resistance from a milliohm literal.
   * @ingroup core_measurements
   * @relates Resistance
   */
  [[nodiscard]] constexpr Resistance operator""_mohm(unsigned long long const value) noexcept
  {
    return Resistance::from_milliohms(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
