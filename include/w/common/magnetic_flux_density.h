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
 * @brief Magnetic flux density value stored in `T`.
 * @details MagneticFluxDensity is a scalar magnetic field strength stored in teslas, with
 * gauss, microtesla and nanotesla conversions.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto earth_field = 0.5_gauss;
 * auto micro = earth_field.as_microteslas(); // 50
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Current
 */
template <std::floating_point T>
class BasicMagneticFluxDensity : public QuantityBase<BasicMagneticFluxDensity<T>, T>
{
  using Base = QuantityBase<BasicMagneticFluxDensity, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "T";

  /**
   * @brief Creates a zero magnetic flux density.
   */
  constexpr BasicMagneticFluxDensity() noexcept = default;

  /**
   * @brief Creates a magnetic flux density from teslas.
   */
  explicit constexpr BasicMagneticFluxDensity(T const teslas) noexcept
    : Base(teslas)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicMagneticFluxDensity(
    BasicMagneticFluxDensity<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a magnetic flux density from teslas.
   */
  [[nodiscard]] static constexpr BasicMagneticFluxDensity from_teslas(T const teslas) noexcept
  {
    return BasicMagneticFluxDensity(teslas);
  }

  /**
   * @brief Creates a magnetic flux density from gauss.
   */
  [[nodiscard]] static constexpr BasicMagneticFluxDensity from_gauss(T const gauss) noexcept
  {
    return BasicMagneticFluxDensity(gauss / static_cast<T>(TESLA_GAUSS_FACTOR));
  }

  /**
   * @brief Creates a magnetic flux density from microteslas.
   */
  [[nodiscard]] static constexpr BasicMagneticFluxDensity from_microteslas(
    T const microteslas
  ) noexcept
  {
    return BasicMagneticFluxDensity(microteslas / 1'000'000);
  }

  /**
   * @brief Creates a magnetic flux density from nanoteslas.
   */
  [[nodiscard]] static constexpr BasicMagneticFluxDensity from_nanoteslas(
    T const nanoteslas
  ) noexcept
  {
    return BasicMagneticFluxDensity(nanoteslas / 1'000'000'000);
  }

  /**
   * @brief Returns teslas.
   */
  [[nodiscard]] constexpr T as_teslas() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns gauss.
   */
  [[nodiscard]] constexpr T as_gauss() const noexcept
  {
    return this->magnitude() * static_cast<T>(TESLA_GAUSS_FACTOR);
  }

  /**
   * @brief Returns microteslas.
   */
  [[nodiscard]] constexpr T as_microteslas() const noexcept
  {
    return this->magnitude() * 1'000'000;
  }

  /**
   * @brief Returns nanoteslas.
   */
  [[nodiscard]] constexpr T as_nanoteslas() const noexcept
  {
    return this->magnitude() * 1'000'000'000;
  }
};

/**
 * @brief MagneticFluxDensity in single precision, the default.
 * @ingroup core_measurements
 */
using MagneticFluxDensity = BasicMagneticFluxDensity<float>;

inline namespace literals
{
  /**
   * @brief Creates a magnetic flux density from a gauss literal.
   * @ingroup core_measurements
   * @relates MagneticFluxDensity
   */
  [[nodiscard]] constexpr MagneticFluxDensity operator""_gauss(long double const value) noexcept
  {
    return MagneticFluxDensity::from_gauss(static_cast<Real>(value));
  }

  /**
   * @brief Creates a magnetic flux density from a gauss literal.
   * @ingroup core_measurements
   * @relates MagneticFluxDensity
   */
  [[nodiscard]] constexpr MagneticFluxDensity operator""_gauss(
    unsigned long long const value
  ) noexcept
  {
    return MagneticFluxDensity::from_gauss(static_cast<Real>(value));
  }

  /**
   * @brief Creates a magnetic flux density from a microtesla literal.
   * @ingroup core_measurements
   * @relates MagneticFluxDensity
   */
  [[nodiscard]] constexpr MagneticFluxDensity operator""_ut(long double const value) noexcept
  {
    return MagneticFluxDensity::from_microteslas(static_cast<Real>(value));
  }

  /**
   * @brief Creates a magnetic flux density from a microtesla literal.
   * @ingroup core_measurements
   * @relates MagneticFluxDensity
   */
  [[nodiscard]] constexpr MagneticFluxDensity operator""_ut(unsigned long long const value) noexcept
  {
    return MagneticFluxDensity::from_microteslas(static_cast<Real>(value));
  }

  /**
   * @brief Creates a magnetic flux density from a nanotesla literal.
   * @ingroup core_measurements
   * @relates MagneticFluxDensity
   */
  [[nodiscard]] constexpr MagneticFluxDensity operator""_nt(long double const value) noexcept
  {
    return MagneticFluxDensity::from_nanoteslas(static_cast<Real>(value));
  }

  /**
   * @brief Creates a magnetic flux density from a nanotesla literal.
   * @ingroup core_measurements
   * @relates MagneticFluxDensity
   */
  [[nodiscard]] constexpr MagneticFluxDensity operator""_nt(unsigned long long const value) noexcept
  {
    return MagneticFluxDensity::from_nanoteslas(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
