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
 * @brief Power value stored in `W`.
 * @details Power is a small value type with watt and kilowatt conversions.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto hover = 1.8_kw + 150_w; // 1950 W
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Energy
 * @see Voltage
 * @see Current
 */
class W_COMMON_API Power : public QuantityBase<Power>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double watts READ as_watts CONSTANT FINAL)
  Q_PROPERTY(double kilowatts READ as_kilowatts CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "W";

  /**
   * @brief Creates a zero power.
   */
  constexpr Power() noexcept = default;

  /**
   * @brief Creates a power from watts.
   */
  explicit constexpr Power(Real const watts) noexcept
    : QuantityBase(watts)
  {}

  /**
   * @brief Creates a power from watts.
   */
  [[nodiscard]] static constexpr Power from_watts(Real const watts) noexcept
  {
    return Power(watts);
  }

  /**
   * @brief Creates a power from kilowatts.
   */
  [[nodiscard]] static constexpr Power from_kilowatts(Real const kilowatts) noexcept
  {
    return Power(kilowatts * 1'000);
  }

  /**
   * @brief Returns watts.
   */
  [[nodiscard]] constexpr Real as_watts() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns kilowatts.
   */
  [[nodiscard]] constexpr Real as_kilowatts() const noexcept { return this->magnitude() / 1'000; }
};

inline namespace literals
{
  /**
   * @brief Creates a power from a watt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_w(long double const value) noexcept
  {
    return Power::from_watts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a power from a watt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_w(unsigned long long const value) noexcept
  {
    return Power::from_watts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a power from a kilowatt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_kw(long double const value) noexcept
  {
    return Power::from_kilowatts(static_cast<Real>(value));
  }

  /**
   * @brief Creates a power from a kilowatt literal.
   * @ingroup core_measurements
   * @relates Power
   */
  [[nodiscard]] constexpr Power operator""_kw(unsigned long long const value) noexcept
  {
    return Power::from_kilowatts(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Power)
#endif
