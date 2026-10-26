#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Voltage value stored in `V`.
 * @details Voltage is a small value type with volt and millivolt conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto cell = 3.7_v;
 * auto pack = cell * 6.0; // 22.2 V
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Current
 * @see Resistance
 * @see Power
 */
template <std::floating_point T>
class BasicVoltage : public QuantityBase<BasicVoltage<T>, T>
{
  using Base = QuantityBase<BasicVoltage, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "V";

  /**
   * @brief Creates a zero voltage.
   */
  constexpr BasicVoltage() noexcept = default;

  /**
   * @brief Creates a voltage from volts.
   */
  explicit constexpr BasicVoltage(T const volts) noexcept
    : Base(volts)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicVoltage(
    BasicVoltage<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a voltage from volts.
   */
  [[nodiscard]] static constexpr BasicVoltage from_volts(T const volts) noexcept
  {
    return BasicVoltage(volts);
  }

  /**
   * @brief Creates a voltage from millivolts.
   */
  [[nodiscard]] static constexpr BasicVoltage from_millivolts(T const millivolts) noexcept
  {
    return BasicVoltage(millivolts / 1'000);
  }

  /**
   * @brief Returns volts.
   */
  [[nodiscard]] constexpr T as_volts() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns millivolts.
   */
  [[nodiscard]] constexpr T as_millivolts() const noexcept { return this->magnitude() * 1'000; }
};

/**
 * @brief Voltage in single precision, the default.
 * @ingroup core_measurements
 */
using Voltage = BasicVoltage<float>;

inline namespace literals
{
  /**
   * @brief Creates a voltage from a volt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_v(long double const value) noexcept
  {
    return Voltage::from_volts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a voltage from a volt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_v(unsigned long long const value) noexcept
  {
    return Voltage::from_volts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a voltage from a millivolt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_mv(long double const value) noexcept
  {
    return Voltage::from_millivolts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a voltage from a millivolt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_mv(unsigned long long const value) noexcept
  {
    return Voltage::from_millivolts(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
