#pragma once

#include <string_view>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/quantity.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qstring.h>
#endif

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
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
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
class W_COMMON_API Temperature : public QuantityBase<Temperature>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double kelvin READ as_kelvin CONSTANT FINAL)
  Q_PROPERTY(double celsius READ as_celsius CONSTANT FINAL)
  Q_PROPERTY(double fahrenheit READ as_fahrenheit CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "K";

  /**
   * @brief Creates a zero temperature.
   */
  constexpr Temperature() noexcept = default;

  /**
   * @brief Creates a temperature from kelvin.
   */
  explicit constexpr Temperature(Real const kelvin) noexcept
    : QuantityBase(kelvin)
  {}

  /**
   * @brief Creates a temperature from kelvin.
   */
  [[nodiscard]] static constexpr Temperature from_kelvin(Real const kelvin) noexcept
  {
    return Temperature(kelvin);
  }

  /**
   * @brief Creates a temperature from degrees Celsius.
   */
  [[nodiscard]] static constexpr Temperature from_celsius(Real const celsius) noexcept
  {
    return Temperature(celsius + CELSIUS_ZERO_KELVIN);
  }

  /**
   * @brief Creates a temperature from degrees Fahrenheit.
   */
  [[nodiscard]] static constexpr Temperature from_fahrenheit(Real const fahrenheit) noexcept
  {
    return Temperature(((fahrenheit - 32) * 5 / 9) + CELSIUS_ZERO_KELVIN);
  }

  /**
   * @brief Returns kelvin.
   */
  [[nodiscard]] constexpr Real as_kelvin() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns degrees Celsius.
   */
  [[nodiscard]] constexpr Real as_celsius() const noexcept
  {
    return this->magnitude() - CELSIUS_ZERO_KELVIN;
  }

  /**
   * @brief Returns degrees Fahrenheit.
   */
  [[nodiscard]] constexpr Real as_fahrenheit() const noexcept
  {
    return ((this->magnitude() - CELSIUS_ZERO_KELVIN) * 9 / 5) + 32;
  }

  /**
   * @brief Deleted: the negation of an absolute temperature has no meaning.
   * @details It also rejects literals such as `-5_degc`, which C++ parses as `-(5_degc)`.
   */
  // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
  Temperature operator-() const = delete;
};

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

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Temperature)
#endif
