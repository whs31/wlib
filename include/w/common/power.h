#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Power value stored in `W`.
 * @details Power is a small value type with watt and kilowatt conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto hover = 1.8_kw + 150_w; // 1950 W
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Energy
 * @see Voltage
 * @see Current
 */
template <std::floating_point T>
class BasicPower : public QuantityBase<BasicPower<T>, T>
{
  using Base = QuantityBase<BasicPower, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "W";

  /**
   * @brief Creates a zero power.
   */
  constexpr BasicPower() noexcept = default;

  /**
   * @brief Creates a power from watts.
   */
  explicit constexpr BasicPower(T const watts) noexcept
    : Base(watts)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicPower(BasicPower<U> const other) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a power from watts.
   */
  [[nodiscard]] static constexpr BasicPower from_watts(T const watts) noexcept
  {
    return BasicPower(watts);
  }

  /**
   * @brief Creates a power from kilowatts.
   */
  [[nodiscard]] static constexpr BasicPower from_kilowatts(T const kilowatts) noexcept
  {
    return BasicPower(kilowatts * 1'000);
  }

  /**
   * @brief Returns watts.
   */
  [[nodiscard]] constexpr T as_watts() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns kilowatts.
   */
  [[nodiscard]] constexpr T as_kilowatts() const noexcept { return this->magnitude() / 1'000; }
};

/**
 * @brief Power in single precision, the default.
 * @ingroup core_measurements
 */
using Power = BasicPower<float>;

inline namespace literals
{
  /**
   * @brief Creates a power from a watt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_w(long double const value) noexcept
  {
    return Power::from_watts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a power from a watt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_w(unsigned long long const value) noexcept
  {
    return Power::from_watts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a power from a kilowatt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_kw(long double const value) noexcept
  {
    return Power::from_kilowatts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a power from a kilowatt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_kw(unsigned long long const value) noexcept
  {
    return Power::from_kilowatts(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
