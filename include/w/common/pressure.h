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
 * @brief Pressure value stored in `Pa`.
 * @details Pressure is a small value type with SI, meteorological and aviation pressure units.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto qnh = 1013.25_hpa;
 * auto altimeter = qnh.as_inches_of_mercury(); // 29.92
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Force
 * @see Density
 * @see Temperature
 */
template <std::floating_point T>
class BasicPressure : public QuantityBase<BasicPressure<T>, T>
{
  using Base = QuantityBase<BasicPressure, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "Pa";

  /**
   * @brief Creates a zero pressure.
   */
  constexpr BasicPressure() noexcept = default;

  /**
   * @brief Creates a pressure from pascals.
   */
  explicit constexpr BasicPressure(T const pascals) noexcept
    : Base(pascals)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicPressure(
    BasicPressure<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a pressure from pascals.
   */
  [[nodiscard]] static constexpr BasicPressure from_pascals(T const pascals) noexcept
  {
    return BasicPressure(pascals);
  }

  /**
   * @brief Creates a pressure from hectopascals.
   */
  [[nodiscard]] static constexpr BasicPressure from_hectopascals(T const hectopascals) noexcept
  {
    return BasicPressure(hectopascals * 100);
  }

  /**
   * @brief Creates a pressure from kilopascals.
   */
  [[nodiscard]] static constexpr BasicPressure from_kilopascals(T const kilopascals) noexcept
  {
    return BasicPressure(kilopascals * 1'000);
  }

  /**
   * @brief Creates a pressure from millibars.
   */
  [[nodiscard]] static constexpr BasicPressure from_millibars(T const millibars) noexcept
  {
    return BasicPressure(millibars * 100);
  }

  /**
   * @brief Creates a pressure from conventional inches of mercury.
   */
  [[nodiscard]] static constexpr BasicPressure from_inches_of_mercury(
    T const inches_of_mercury
  ) noexcept
  {
    return BasicPressure(inches_of_mercury * static_cast<T>(INHG_PASCAL_FACTOR));
  }

  /**
   * @brief Returns pascals.
   */
  [[nodiscard]] constexpr T as_pascals() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns hectopascals.
   */
  [[nodiscard]] constexpr T as_hectopascals() const noexcept { return this->magnitude() / 100; }

  /**
   * @brief Returns kilopascals.
   */
  [[nodiscard]] constexpr T as_kilopascals() const noexcept { return this->magnitude() / 1'000; }

  /**
   * @brief Returns millibars.
   */
  [[nodiscard]] constexpr T as_millibars() const noexcept { return this->magnitude() / 100; }

  /**
   * @brief Returns conventional inches of mercury.
   */
  [[nodiscard]] constexpr T as_inches_of_mercury() const noexcept
  {
    return this->magnitude() / static_cast<T>(INHG_PASCAL_FACTOR);
  }
};

/**
 * @brief Pressure in single precision, the default.
 * @ingroup core_measurements
 */
using Pressure = BasicPressure<float>;

inline namespace literals
{
  /**
   * @brief Creates a pressure from a pascal literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_pa(long double const value) noexcept
  {
    return Pressure::from_pascals(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from a pascal literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_pa(unsigned long long const value) noexcept
  {
    return Pressure::from_pascals(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from a hectopascal literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_hpa(long double const value) noexcept
  {
    return Pressure::from_hectopascals(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from a hectopascal literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_hpa(unsigned long long const value) noexcept
  {
    return Pressure::from_hectopascals(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from a kilopascal literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_kpa(long double const value) noexcept
  {
    return Pressure::from_kilopascals(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from a kilopascal literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_kpa(unsigned long long const value) noexcept
  {
    return Pressure::from_kilopascals(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from a millibar literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_mbar(long double const value) noexcept
  {
    return Pressure::from_millibars(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from a millibar literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_mbar(unsigned long long const value) noexcept
  {
    return Pressure::from_millibars(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from an inch-of-mercury literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_inhg(long double const value) noexcept
  {
    return Pressure::from_inches_of_mercury(static_cast<Real>(value));
  }

  /**
   * @brief Creates a pressure from an inch-of-mercury literal.
   * @ingroup core_measurements
   * @relates Pressure
   */
  [[nodiscard]] constexpr Pressure operator""_inhg(unsigned long long const value) noexcept
  {
    return Pressure::from_inches_of_mercury(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
