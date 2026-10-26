#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Conductance value stored in `S`.
 * @details Conductance is a small value type stored in siemens. Its literal is `_siemens`
 * rather than `_s`, which would read as seconds next to `std::chrono` literals.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto load = 0.25_siemens;
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Resistance
 * @see Current
 */
template <std::floating_point T>
class BasicConductance : public QuantityBase<BasicConductance<T>, T>
{
  using Base = QuantityBase<BasicConductance, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "S";

  /**
   * @brief Creates a zero conductance.
   */
  constexpr BasicConductance() noexcept = default;

  /**
   * @brief Creates a conductance from siemens.
   */
  explicit constexpr BasicConductance(T const siemens) noexcept
    : Base(siemens)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicConductance(
    BasicConductance<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a conductance from siemens.
   */
  [[nodiscard]] static constexpr BasicConductance from_siemens(T const siemens) noexcept
  {
    return BasicConductance(siemens);
  }

  /**
   * @brief Returns siemens.
   */
  [[nodiscard]] constexpr T as_siemens() const noexcept { return this->magnitude(); }
};

/**
 * @brief Conductance in single precision, the default.
 * @ingroup core_measurements
 */
using Conductance = BasicConductance<float>;

inline namespace literals
{
  /**
   * @brief Creates a conductance from a siemens literal.
   * @ingroup core_measurements
   * @relates Conductance
   */
  [[nodiscard]] constexpr Conductance operator""_siemens(long double const value) noexcept
  {
    return Conductance::from_siemens(static_cast<Real>(value));
  }

  /**
   * @brief Creates a conductance from a siemens literal.
   * @ingroup core_measurements
   * @relates Conductance
   */
  [[nodiscard]] constexpr Conductance operator""_siemens(unsigned long long const value) noexcept
  {
    return Conductance::from_siemens(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
