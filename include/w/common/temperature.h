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
 * @brief Temperature value stored in `K`.
 * @details Temperature is an absolute thermodynamic temperature stored in kelvin, with
 * Celsius and Fahrenheit conversions.
 *
 * The difference of two temperatures is also a Temperature, in kelvin. It is correct as a
 * kelvin or Celsius *difference*, but its as_celsius() and as_fahrenheit() are meaningless, since
 * those scales are offset from absolute zero.
 *
 * Negation is deleted, so a negative Celsius or Fahrenheit literal such as `-5_degc`, which C++
 * parses as `-(5_degc)`, does not compile. Write `Temperature::from_celsius(-5.0)` instead.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto ambient = 15_degc;
 * auto kelvin = ambient.as_kelvin(); // 288.15
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Pressure
 * @see Density
 */
template <std::floating_point T>
class BasicTemperature : public QuantityBase<BasicTemperature<T>, T>
{
  using Base = QuantityBase<BasicTemperature, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "K";

  /**
   * @brief Creates a zero temperature.
   */
  constexpr BasicTemperature() noexcept = default;

  /**
   * @brief Creates a temperature from kelvin.
   */
  explicit constexpr BasicTemperature(T const kelvin) noexcept
    : Base(kelvin)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicTemperature(
    BasicTemperature<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a temperature from kelvin.
   */
  [[nodiscard]] static constexpr BasicTemperature from_kelvin(T const kelvin) noexcept
  {
    return BasicTemperature(kelvin);
  }

  /**
   * @brief Creates a temperature from degrees Celsius.
   */
  [[nodiscard]] static constexpr BasicTemperature from_celsius(T const celsius) noexcept
  {
    return BasicTemperature(celsius + static_cast<T>(CELSIUS_ZERO_KELVIN));
  }

  /**
   * @brief Creates a temperature from degrees Fahrenheit.
   */
  [[nodiscard]] static constexpr BasicTemperature from_fahrenheit(T const fahrenheit) noexcept
  {
    return BasicTemperature(((fahrenheit - 32) * 5 / 9) + static_cast<T>(CELSIUS_ZERO_KELVIN));
  }

  /**
   * @brief Returns kelvin.
   */
  [[nodiscard]] constexpr T as_kelvin() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns degrees Celsius.
   */
  [[nodiscard]] constexpr T as_celsius() const noexcept
  {
    return this->magnitude() - static_cast<T>(CELSIUS_ZERO_KELVIN);
  }

  /**
   * @brief Returns degrees Fahrenheit.
   */
  [[nodiscard]] constexpr T as_fahrenheit() const noexcept
  {
    return ((this->magnitude() - static_cast<T>(CELSIUS_ZERO_KELVIN)) * 9 / 5) + 32;
  }

  /**
   * @brief Deleted: the negation of an absolute temperature has no meaning.
   * @details It also rejects literals such as `-5_degc`, which C++ parses as `-(5_degc)`.
   */
  // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
  BasicTemperature operator-() const = delete;
};

/**
 * @brief Temperature in single precision, the default.
 * @ingroup core_measurements
 */
using Temperature = BasicTemperature<float>;

inline namespace literals
{
  /**
   * @brief Creates a temperature from a kelvin literal.
   * @ingroup core_measurements
   * @relates Temperature
   */
  [[nodiscard]] constexpr Temperature operator""_k(long double const value) noexcept
  {
    return Temperature::from_kelvin(static_cast<Real>(value));
  }

  /**
   * @brief Creates a temperature from a kelvin literal.
   * @ingroup core_measurements
   * @relates Temperature
   */
  [[nodiscard]] constexpr Temperature operator""_k(unsigned long long const value) noexcept
  {
    return Temperature::from_kelvin(static_cast<Real>(value));
  }

  /**
   * @brief Creates a temperature from a degree-Celsius literal.
   * @ingroup core_measurements
   * @relates Temperature
   */
  [[nodiscard]] constexpr Temperature operator""_degc(long double const value) noexcept
  {
    return Temperature::from_celsius(static_cast<Real>(value));
  }

  /**
   * @brief Creates a temperature from a degree-Celsius literal.
   * @ingroup core_measurements
   * @relates Temperature
   */
  [[nodiscard]] constexpr Temperature operator""_degc(unsigned long long const value) noexcept
  {
    return Temperature::from_celsius(static_cast<Real>(value));
  }

  /**
   * @brief Creates a temperature from a degree-Fahrenheit literal.
   * @ingroup core_measurements
   * @relates Temperature
   */
  [[nodiscard]] constexpr Temperature operator""_degf(long double const value) noexcept
  {
    return Temperature::from_fahrenheit(static_cast<Real>(value));
  }

  /**
   * @brief Creates a temperature from a degree-Fahrenheit literal.
   * @ingroup core_measurements
   * @relates Temperature
   */
  [[nodiscard]] constexpr Temperature operator""_degf(unsigned long long const value) noexcept
  {
    return Temperature::from_fahrenheit(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
