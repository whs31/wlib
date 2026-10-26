#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Density value stored in `kg/m^3`.
 * @details Density is a small value type stored in kilograms per cubic meter.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto sea_level_air = 1.225_kg_per_m3;
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Mass
 * @see Pressure
 * @see Temperature
 */
template <std::floating_point T>
class BasicDensity : public QuantityBase<BasicDensity<T>, T>
{
  using Base = QuantityBase<BasicDensity, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "kg/m^3";

  /**
   * @brief Creates a zero density.
   */
  constexpr BasicDensity() noexcept = default;

  /**
   * @brief Creates a density from kilograms per cubic meter.
   */
  explicit constexpr BasicDensity(T const kilograms_per_cubic_meter) noexcept
    : Base(kilograms_per_cubic_meter)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicDensity(
    BasicDensity<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a density from kilograms per cubic meter.
   */
  [[nodiscard]] static constexpr BasicDensity from_kilograms_per_cubic_meter(
    T const kilograms_per_cubic_meter
  ) noexcept
  {
    return BasicDensity(kilograms_per_cubic_meter);
  }

  /**
   * @brief Returns kilograms per cubic meter.
   */
  [[nodiscard]] constexpr T as_kilograms_per_cubic_meter() const noexcept
  {
    return this->magnitude();
  }
};

/**
 * @brief Density in single precision, the default.
 * @ingroup core_measurements
 */
using Density = BasicDensity<float>;

inline namespace literals
{
  /**
   * @brief Creates a density from a kilogram-per-cubic-meter literal.
   * @ingroup core_measurements
   * @relates Density
   */
  [[nodiscard]] constexpr Density operator""_kg_per_m3(long double const value) noexcept
  {
    return Density::from_kilograms_per_cubic_meter(static_cast<Real>(value));
  }

  /**
   * @brief Creates a density from a kilogram-per-cubic-meter literal.
   * @ingroup core_measurements
   * @relates Density
   */
  [[nodiscard]] constexpr Density operator""_kg_per_m3(unsigned long long const value) noexcept
  {
    return Density::from_kilograms_per_cubic_meter(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
