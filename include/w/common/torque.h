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
 * @brief Torque value stored in `N*m`.
 * @details Torque is a small value type stored in newton-meters.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto load = 1.5_nm * 2.0; // 3 N*m
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Force
 * @see AngularAcceleration
 * @see MomentOfInertia
 */
class W_COMMON_API Torque : public QuantityBase<Torque>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double newtonMeters READ as_newton_meters CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "N*m";

  /**
   * @brief Creates a zero torque.
   */
  constexpr Torque() noexcept = default;

  /**
   * @brief Creates a torque from newton-meters.
   */
  explicit constexpr Torque(Real const newton_meters) noexcept
    : QuantityBase(newton_meters)
  {}

  /**
   * @brief Creates a torque from newton-meters.
   */
  [[nodiscard]] static constexpr Torque from_newton_meters(Real const newton_meters) noexcept
  {
    return Torque(newton_meters);
  }

  /**
   * @brief Returns newton-meters.
   */
  [[nodiscard]] constexpr Real as_newton_meters() const noexcept { return this->magnitude(); }
};

inline namespace literals
{
  /**
   * @brief Creates a torque from a newton-meter literal.
   * @ingroup core_measurements
   * @relates Torque
   */
  [[nodiscard]] constexpr Torque operator""_nm(long double const value) noexcept
  {
    return Torque::from_newton_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a torque from a newton-meter literal.
   * @ingroup core_measurements
   * @relates Torque
   */
  [[nodiscard]] constexpr Torque operator""_nm(unsigned long long const value) noexcept
  {
    return Torque::from_newton_meters(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Torque)
#endif
