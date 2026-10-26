#pragma once

#include <string_view>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/quantity.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Magnetic flux density value stored in `T`.
 * @details MagneticFluxDensity is a scalar magnetic field strength stored in teslas, with
 * gauss, microtesla and nanotesla conversions.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
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
class W_COMMON_API MagneticFluxDensity : public QuantityBase<MagneticFluxDensity>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double teslas READ as_teslas CONSTANT FINAL)
  Q_PROPERTY(double gauss READ as_gauss CONSTANT FINAL)
  Q_PROPERTY(double microteslas READ as_microteslas CONSTANT FINAL)
  Q_PROPERTY(double nanoteslas READ as_nanoteslas CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "T";

  /**
   * @brief Creates a zero magnetic flux density.
   */
  constexpr MagneticFluxDensity() noexcept = default;

  /**
   * @brief Creates a magnetic flux density from teslas.
   */
  explicit constexpr MagneticFluxDensity(Real const teslas) noexcept
    : QuantityBase(teslas)
  {}

  /**
   * @brief Creates a magnetic flux density from teslas.
   */
  [[nodiscard]] static constexpr MagneticFluxDensity from_teslas(Real const teslas) noexcept
  {
    return MagneticFluxDensity(teslas);
  }

  /**
   * @brief Creates a magnetic flux density from gauss.
   */
  [[nodiscard]] static constexpr MagneticFluxDensity from_gauss(Real const gauss) noexcept
  {
    return MagneticFluxDensity(gauss / TESLA_GAUSS_FACTOR);
  }

  /**
   * @brief Creates a magnetic flux density from microteslas.
   */
  [[nodiscard]] static constexpr MagneticFluxDensity from_microteslas(
    Real const microteslas
  ) noexcept
  {
    return MagneticFluxDensity(microteslas / 1'000'000);
  }

  /**
   * @brief Creates a magnetic flux density from nanoteslas.
   */
  [[nodiscard]] static constexpr MagneticFluxDensity from_nanoteslas(Real const nanoteslas) noexcept
  {
    return MagneticFluxDensity(nanoteslas / 1'000'000'000);
  }

  /**
   * @brief Returns teslas.
   */
  [[nodiscard]] constexpr Real as_teslas() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns gauss.
   */
  [[nodiscard]] constexpr Real as_gauss() const noexcept
  {
    return this->magnitude() * TESLA_GAUSS_FACTOR;
  }

  /**
   * @brief Returns microteslas.
   */
  [[nodiscard]] constexpr Real as_microteslas() const noexcept
  {
    return this->magnitude() * 1'000'000;
  }

  /**
   * @brief Returns nanoteslas.
   */
  [[nodiscard]] constexpr Real as_nanoteslas() const noexcept
  {
    return this->magnitude() * 1'000'000'000;
  }
};

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

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::MagneticFluxDensity)
#endif
