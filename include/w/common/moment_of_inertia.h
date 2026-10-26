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
 * @brief Moment of inertia value stored in `kg*m^2`.
 * @details MomentOfInertia is a small value type stored in kilogram-square-meters.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
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
class W_COMMON_API MomentOfInertia : public QuantityBase<MomentOfInertia>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double kilogramSquareMeters READ as_kilogram_square_meters CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "kg*m^2";

  /**
   * @brief Creates a zero moment of inertia.
   */
  constexpr MomentOfInertia() noexcept = default;

  /**
   * @brief Creates a moment of inertia from kilogram-square-meters.
   */
  explicit constexpr MomentOfInertia(Real const kilogram_square_meters) noexcept
    : QuantityBase(kilogram_square_meters)
  {}

  /**
   * @brief Creates a moment of inertia from kilogram-square-meters.
   */
  [[nodiscard]] static constexpr MomentOfInertia from_kilogram_square_meters(
    Real const kilogram_square_meters
  ) noexcept
  {
    return MomentOfInertia(kilogram_square_meters);
  }

  /**
   * @brief Returns kilogram-square-meters.
   */
  [[nodiscard]] constexpr Real as_kilogram_square_meters() const noexcept
  {
    return this->magnitude();
  }
};

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

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::MomentOfInertia)
#endif
