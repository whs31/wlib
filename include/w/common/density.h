#pragma once

#include <string_view>

#include <w/common/config.h>
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
 * @brief Density value stored in `kg/m^3`.
 * @details Density is a small value type stored in kilograms per cubic meter.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
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
class W_COMMON_API Density : public QuantityBase<Density>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double kilogramsPerCubicMeter READ as_kilograms_per_cubic_meter CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "kg/m^3";

  /**
   * @brief Creates a zero density.
   */
  constexpr Density() noexcept = default;

  /**
   * @brief Creates a density from kilograms per cubic meter.
   */
  explicit constexpr Density(Real const kilograms_per_cubic_meter) noexcept
    : QuantityBase(kilograms_per_cubic_meter)
  {}

  /**
   * @brief Creates a density from kilograms per cubic meter.
   */
  [[nodiscard]] static constexpr Density from_kilograms_per_cubic_meter(
    Real const kilograms_per_cubic_meter
  ) noexcept
  {
    return Density(kilograms_per_cubic_meter);
  }

  /**
   * @brief Returns kilograms per cubic meter.
   */
  [[nodiscard]] constexpr Real as_kilograms_per_cubic_meter() const noexcept
  {
    return this->magnitude();
  }
};

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

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Density)
#endif
