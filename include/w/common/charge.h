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
 * @brief Charge value stored in `C`.
 * @details Charge is an electric charge stored in coulombs, with ampere-hour and
 * milliampere-hour conversions for battery capacities.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto capacity = 5000_mah;
 * auto amp_hours = capacity.as_ampere_hours(); // 5
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Current
 * @see Energy
 * @see Capacitance
 */
template <std::floating_point T>
class BasicCharge : public QuantityBase<BasicCharge<T>, T>
{
  using Base = QuantityBase<BasicCharge, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "C";

  /**
   * @brief Creates a zero charge.
   */
  constexpr BasicCharge() noexcept = default;

  /**
   * @brief Creates a charge from coulombs.
   */
  explicit constexpr BasicCharge(T const coulombs) noexcept
    : Base(coulombs)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicCharge(BasicCharge<U> const other) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a charge from coulombs.
   */
  [[nodiscard]] static constexpr BasicCharge from_coulombs(T const coulombs) noexcept
  {
    return BasicCharge(coulombs);
  }

  /**
   * @brief Creates a charge from ampere-hours.
   */
  [[nodiscard]] static constexpr BasicCharge from_ampere_hours(T const ampere_hours) noexcept
  {
    return BasicCharge(ampere_hours * static_cast<T>(SECONDS_HOURS_FACTOR));
  }

  /**
   * @brief Creates a charge from milliampere-hours.
   */
  [[nodiscard]] static constexpr BasicCharge from_milliampere_hours(
    T const milliampere_hours
  ) noexcept
  {
    return BasicCharge(milliampere_hours * (static_cast<T>(SECONDS_HOURS_FACTOR) / 1'000));
  }

  /**
   * @brief Returns coulombs.
   */
  [[nodiscard]] constexpr T as_coulombs() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns ampere-hours.
   */
  [[nodiscard]] constexpr T as_ampere_hours() const noexcept
  {
    return this->magnitude() / static_cast<T>(SECONDS_HOURS_FACTOR);
  }

  /**
   * @brief Returns milliampere-hours.
   */
  [[nodiscard]] constexpr T as_milliampere_hours() const noexcept
  {
    return this->magnitude() / (static_cast<T>(SECONDS_HOURS_FACTOR) / 1'000);
  }
};

/**
 * @brief Charge in single precision, the default.
 * @ingroup core_measurements
 */
using Charge = BasicCharge<float>;

inline namespace literals
{
  /**
   * @brief Creates a charge from an ampere-hour literal.
   * @ingroup core_measurements
   * @relates Charge
   */
  [[nodiscard]] constexpr Charge operator""_ah(long double const value) noexcept
  {
    return Charge::from_ampere_hours(static_cast<Real>(value));
  }

  /**
   * @brief Creates a charge from an ampere-hour literal.
   * @ingroup core_measurements
   * @relates Charge
   */
  [[nodiscard]] constexpr Charge operator""_ah(unsigned long long const value) noexcept
  {
    return Charge::from_ampere_hours(static_cast<Real>(value));
  }

  /**
   * @brief Creates a charge from a milliampere-hour literal.
   * @ingroup core_measurements
   * @relates Charge
   */
  [[nodiscard]] constexpr Charge operator""_mah(long double const value) noexcept
  {
    return Charge::from_milliampere_hours(static_cast<Real>(value));
  }

  /**
   * @brief Creates a charge from a milliampere-hour literal.
   * @ingroup core_measurements
   * @relates Charge
   */
  [[nodiscard]] constexpr Charge operator""_mah(unsigned long long const value) noexcept
  {
    return Charge::from_milliampere_hours(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
