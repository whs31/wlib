#pragma once

#include <concepts>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Mass value stored in `kg`.
 * @details Mass is a small value type with kilogram and gram conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto payload = 2.5_kg + 300_g; // 2.8 kg
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Force
 * @see Density
 * @see MomentOfInertia
 */
template <std::floating_point T>
class BasicMass : public QuantityBase<BasicMass<T>, T>
{
  using Base = QuantityBase<BasicMass, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "kg";

  /**
   * @brief Creates a zero mass.
   */
  constexpr BasicMass() noexcept = default;

  /**
   * @brief Creates a mass from kilograms.
   */
  explicit constexpr BasicMass(T const kilograms) noexcept
    : Base(kilograms)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicMass(BasicMass<U> const other) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a mass from kilograms.
   */
  [[nodiscard]] static constexpr BasicMass from_kilograms(T const kilograms) noexcept
  {
    return BasicMass(kilograms);
  }

  /**
   * @brief Creates a mass from grams.
   */
  [[nodiscard]] static constexpr BasicMass from_grams(T const grams) noexcept
  {
    return BasicMass(grams / 1'000);
  }

  /**
   * @brief Returns kilograms.
   */
  [[nodiscard]] constexpr T as_kilograms() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns grams.
   */
  [[nodiscard]] constexpr T as_grams() const noexcept { return this->magnitude() * 1'000; }
};

/**
 * @brief Mass in single precision, the default.
 * @ingroup core_measurements
 */
using Mass = BasicMass<float>;

inline namespace literals
{
  /**
   * @brief Creates a mass from a kilogram literal.
   * @ingroup core_measurements
   * @relates Mass
   */
  [[nodiscard]] constexpr Mass operator""_kg(long double const value) noexcept
  {
    return Mass::from_kilograms(static_cast<Real>(value));
  }

  /**
   * @brief Creates a mass from a kilogram literal.
   * @ingroup core_measurements
   * @relates Mass
   */
  [[nodiscard]] constexpr Mass operator""_kg(unsigned long long const value) noexcept
  {
    return Mass::from_kilograms(static_cast<Real>(value));
  }

  /**
   * @brief Creates a mass from a gram literal.
   * @ingroup core_measurements
   * @relates Mass
   */
  [[nodiscard]] constexpr Mass operator""_g(long double const value) noexcept
  {
    return Mass::from_grams(static_cast<Real>(value));
  }

  /**
   * @brief Creates a mass from a gram literal.
   * @ingroup core_measurements
   * @relates Mass
   */
  [[nodiscard]] constexpr Mass operator""_g(unsigned long long const value) noexcept
  {
    return Mass::from_grams(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
