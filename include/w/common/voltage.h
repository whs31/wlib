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
 * @brief Voltage value stored in `V`.
 * @details Voltage is a small value type with volt and millivolt conversions.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto cell = 3.7_v;
 * auto pack = cell * 6.0; // 22.2 V
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Current
 * @see Resistance
 * @see Power
 */
class W_COMMON_API Voltage : public QuantityBase<Voltage>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double volts READ as_volts CONSTANT FINAL)
  Q_PROPERTY(double millivolts READ as_millivolts CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "V";

  /**
   * @brief Creates a zero voltage.
   */
  constexpr Voltage() noexcept = default;

  /**
   * @brief Creates a voltage from volts.
   */
  explicit constexpr Voltage(Real const volts) noexcept
    : QuantityBase(volts)
  {}

  /**
   * @brief Creates a voltage from volts.
   */
  [[nodiscard]] static constexpr Voltage from_volts(Real const volts) noexcept
  {
    return Voltage(volts);
  }

  /**
   * @brief Creates a voltage from millivolts.
   */
  [[nodiscard]] static constexpr Voltage from_millivolts(Real const millivolts) noexcept
  {
    return Voltage(millivolts / 1'000);
  }

  /**
   * @brief Returns volts.
   */
  [[nodiscard]] constexpr Real as_volts() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns millivolts.
   */
  [[nodiscard]] constexpr Real as_millivolts() const noexcept { return this->magnitude() * 1'000; }
};

inline namespace literals
{
  /**
   * @brief Creates a voltage from a volt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_v(long double const value) noexcept
  {
    return Voltage::from_volts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a voltage from a volt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_v(unsigned long long const value) noexcept
  {
    return Voltage::from_volts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a voltage from a millivolt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_mv(long double const value) noexcept
  {
    return Voltage::from_millivolts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a voltage from a millivolt literal.
   * @ingroup core_measurements
   * @relates Voltage
   */
  [[nodiscard]] constexpr Voltage operator""_mv(unsigned long long const value) noexcept
  {
    return Voltage::from_millivolts(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Voltage)
#endif
