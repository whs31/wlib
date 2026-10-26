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
 * @brief Energy value stored in `J`.
 * @details Energy is a small value type with joule, watt-hour and kilowatt-hour conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto battery = 222_wh;
 * auto joules = battery.as_joules(); // 799200
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Power
 * @see Charge
 */
template <std::floating_point T>
class BasicEnergy : public QuantityBase<BasicEnergy<T>, T>
{
  using Base = QuantityBase<BasicEnergy, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "J";

  /**
   * @brief Creates a zero energy.
   */
  constexpr BasicEnergy() noexcept = default;

  /**
   * @brief Creates an energy from joules.
   */
  explicit constexpr BasicEnergy(T const joules) noexcept
    : Base(joules)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicEnergy(BasicEnergy<U> const other) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates an energy from joules.
   */
  [[nodiscard]] static constexpr BasicEnergy from_joules(T const joules) noexcept
  {
    return BasicEnergy(joules);
  }

  /**
   * @brief Creates an energy from watt-hours.
   */
  [[nodiscard]] static constexpr BasicEnergy from_watt_hours(T const watt_hours) noexcept
  {
    return BasicEnergy(watt_hours * static_cast<T>(SECONDS_HOURS_FACTOR));
  }

  /**
   * @brief Creates an energy from kilowatt-hours.
   */
  [[nodiscard]] static constexpr BasicEnergy from_kilowatt_hours(T const kilowatt_hours) noexcept
  {
    return BasicEnergy(kilowatt_hours * (static_cast<T>(SECONDS_HOURS_FACTOR) * 1'000));
  }

  /**
   * @brief Returns joules.
   */
  [[nodiscard]] constexpr T as_joules() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns watt-hours.
   */
  [[nodiscard]] constexpr T as_watt_hours() const noexcept
  {
    return this->magnitude() / static_cast<T>(SECONDS_HOURS_FACTOR);
  }

  /**
   * @brief Returns kilowatt-hours.
   */
  [[nodiscard]] constexpr T as_kilowatt_hours() const noexcept
  {
    return this->magnitude() / (static_cast<T>(SECONDS_HOURS_FACTOR) * 1'000);
  }
};

/**
 * @brief Energy in single precision, the default.
 * @ingroup core_measurements
 */
using Energy = BasicEnergy<float>;

inline namespace literals
{
  /**
   * @brief Creates an energy from a joule literal.
   * @ingroup core_measurements
   * @relates Energy
   */
  [[nodiscard]] constexpr Energy operator""_j(long double const value) noexcept
  {
    return Energy::from_joules(static_cast<Real>(value));
  }

  /**
   * @brief Creates an energy from a joule literal.
   * @ingroup core_measurements
   * @relates Energy
   */
  [[nodiscard]] constexpr Energy operator""_j(unsigned long long const value) noexcept
  {
    return Energy::from_joules(static_cast<Real>(value));
  }

  /**
   * @brief Creates an energy from a watt-hour literal.
   * @ingroup core_measurements
   * @relates Energy
   */
  [[nodiscard]] constexpr Energy operator""_wh(long double const value) noexcept
  {
    return Energy::from_watt_hours(static_cast<Real>(value));
  }

  /**
   * @brief Creates an energy from a watt-hour literal.
   * @ingroup core_measurements
   * @relates Energy
   */
  [[nodiscard]] constexpr Energy operator""_wh(unsigned long long const value) noexcept
  {
    return Energy::from_watt_hours(static_cast<Real>(value));
  }

  /**
   * @brief Creates an energy from a kilowatt-hour literal.
   * @ingroup core_measurements
   * @relates Energy
   */
  [[nodiscard]] constexpr Energy operator""_kwh(long double const value) noexcept
  {
    return Energy::from_kilowatt_hours(static_cast<Real>(value));
  }

  /**
   * @brief Creates an energy from a kilowatt-hour literal.
   * @ingroup core_measurements
   * @relates Energy
   */
  [[nodiscard]] constexpr Energy operator""_kwh(unsigned long long const value) noexcept
  {
    return Energy::from_kilowatt_hours(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
