#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Current value stored in `A`.
 * @details Current is a small value type with ampere and milliampere conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto load = 12_a + 350_ma; // 12.35 A
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Voltage
 * @see Charge
 * @see Power
 */
template <std::floating_point T>
class BasicCurrent : public QuantityBase<BasicCurrent<T>, T>
{
  using Base = QuantityBase<BasicCurrent, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "A";

  /**
   * @brief Creates a zero current.
   */
  constexpr BasicCurrent() noexcept = default;

  /**
   * @brief Creates a current from amperes.
   */
  explicit constexpr BasicCurrent(T const amperes) noexcept
    : Base(amperes)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicCurrent(
    BasicCurrent<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a current from amperes.
   */
  [[nodiscard]] static constexpr BasicCurrent from_amperes(T const amperes) noexcept
  {
    return BasicCurrent(amperes);
  }

  /**
   * @brief Creates a current from milliamperes.
   */
  [[nodiscard]] static constexpr BasicCurrent from_milliamperes(T const milliamperes) noexcept
  {
    return BasicCurrent(milliamperes / 1'000);
  }

  /**
   * @brief Returns amperes.
   */
  [[nodiscard]] constexpr T as_amperes() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns milliamperes.
   */
  [[nodiscard]] constexpr T as_milliamperes() const noexcept { return this->magnitude() * 1'000; }
};

/**
 * @brief Current in single precision, the default.
 * @ingroup core_measurements
 */
using Current = BasicCurrent<float>;

inline namespace literals
{
  /**
   * @brief Creates a current from an ampere literal.
   * @ingroup core_measurements
   * @relates Current
   */
  [[nodiscard]] constexpr Current operator""_a(long double const value) noexcept
  {
    return Current::from_amperes(static_cast<Real>(value));
  }

  /**
   * @brief Creates a current from an ampere literal.
   * @ingroup core_measurements
   * @relates Current
   */
  [[nodiscard]] constexpr Current operator""_a(unsigned long long const value) noexcept
  {
    return Current::from_amperes(static_cast<Real>(value));
  }

  /**
   * @brief Creates a current from a milliampere literal.
   * @ingroup core_measurements
   * @relates Current
   */
  [[nodiscard]] constexpr Current operator""_ma(long double const value) noexcept
  {
    return Current::from_milliamperes(static_cast<Real>(value));
  }

  /**
   * @brief Creates a current from a milliampere literal.
   * @ingroup core_measurements
   * @relates Current
   */
  [[nodiscard]] constexpr Current operator""_ma(unsigned long long const value) noexcept
  {
    return Current::from_milliamperes(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
