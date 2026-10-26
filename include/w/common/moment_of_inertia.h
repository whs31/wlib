#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Moment of inertia value stored in `kg*m^2`.
 * @details MomentOfInertia is a small value type stored in kilogram-square-meters.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto rotor = 0.35_kg_m2;
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Mass
 * @see Torque
 * @see AngularAcceleration
 */
template <std::floating_point T>
class BasicMomentOfInertia : public QuantityBase<BasicMomentOfInertia<T>, T>
{
  using Base = QuantityBase<BasicMomentOfInertia, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "kg*m^2";

  /**
   * @brief Creates a zero moment of inertia.
   */
  constexpr BasicMomentOfInertia() noexcept = default;

  /**
   * @brief Creates a moment of inertia from kilogram-square-meters.
   */
  explicit constexpr BasicMomentOfInertia(T const kilogram_square_meters) noexcept
    : Base(kilogram_square_meters)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicMomentOfInertia(
    BasicMomentOfInertia<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a moment of inertia from kilogram-square-meters.
   */
  [[nodiscard]] static constexpr BasicMomentOfInertia from_kilogram_square_meters(
    T const kilogram_square_meters
  ) noexcept
  {
    return BasicMomentOfInertia(kilogram_square_meters);
  }

  /**
   * @brief Returns kilogram-square-meters.
   */
  [[nodiscard]] constexpr T as_kilogram_square_meters() const noexcept { return this->magnitude(); }
};

/**
 * @brief MomentOfInertia in single precision, the default.
 * @ingroup core_measurements
 */
using MomentOfInertia = BasicMomentOfInertia<float>;

inline namespace literals
{
  /**
   * @brief Creates a moment of inertia from a kilogram-square-meter literal.
   * @ingroup core_measurements
   * @relates MomentOfInertia
   */
  [[nodiscard]] constexpr MomentOfInertia operator""_kg_m2(long double const value) noexcept
  {
    return MomentOfInertia::from_kilogram_square_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a moment of inertia from a kilogram-square-meter literal.
   * @ingroup core_measurements
   * @relates MomentOfInertia
   */
  [[nodiscard]] constexpr MomentOfInertia operator""_kg_m2(unsigned long long const value) noexcept
  {
    return MomentOfInertia::from_kilogram_square_meters(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
