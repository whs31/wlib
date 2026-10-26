#pragma once

#include <chrono>
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
 * @brief Frequency value stored in `Hz`.
 * @details Frequency is a small value type with hertz and kilohertz conversions. It
 * converts to and from the period as a `std::chrono` duration.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * using namespace std::chrono_literals;
 *
 * auto physics_rate = 400_hz;
 * auto step = physics_rate.period(); // 2.5 ms
 * auto display_rate = Frequency::from_period(16ms); // 62.5 Hz
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see AngularVelocity
 */
class W_COMMON_API Frequency : public QuantityBase<Frequency>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double hertz READ as_hertz CONSTANT FINAL)
  Q_PROPERTY(double kilohertz READ as_kilohertz CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "Hz";

  /**
   * @brief Creates a zero frequency.
   */
  constexpr Frequency() noexcept = default;

  /**
   * @brief Creates a frequency from hertz.
   */
  explicit constexpr Frequency(Real const hertz) noexcept
    : QuantityBase(hertz)
  {}

  /**
   * @brief Creates a frequency from hertz.
   */
  [[nodiscard]] static constexpr Frequency from_hertz(Real const hertz) noexcept
  {
    return Frequency(hertz);
  }

  /**
   * @brief Creates a frequency from kilohertz.
   */
  [[nodiscard]] static constexpr Frequency from_kilohertz(Real const kilohertz) noexcept
  {
    return Frequency(kilohertz * 1'000);
  }

  /**
   * @brief Returns hertz.
   */
  [[nodiscard]] constexpr Real as_hertz() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns kilohertz.
   */
  [[nodiscard]] constexpr Real as_kilohertz() const noexcept { return this->magnitude() / 1'000; }

  /**
   * @brief Creates the frequency of an event that repeats every @p period.
   * @details A zero period yields an infinite frequency.
   */
  template <typename Rep, typename Period>
  [[nodiscard]] static constexpr Frequency from_period(
    std::chrono::duration<Rep, Period> const period
  ) noexcept
  {
    return Frequency(1 / std::chrono::duration<Real>(period).count());
  }

  /**
   * @brief Returns the time between two events at this frequency.
   * @details A zero frequency yields an infinite period.
   */
  [[nodiscard]] constexpr std::chrono::duration<Real> period() const noexcept
  {
    return std::chrono::duration<Real>(1 / this->magnitude());
  }
};

inline namespace literals
{
  /**
   * @brief Creates a frequency from a hertz literal.
   * @ingroup core_measurements
   * @relates Frequency
   */
  [[nodiscard]] constexpr Frequency operator""_hz(long double const value) noexcept
  {
    return Frequency::from_hertz(static_cast<Real>(value));
  }

  /**
   * @brief Creates a frequency from a hertz literal.
   * @ingroup core_measurements
   * @relates Frequency
   */
  [[nodiscard]] constexpr Frequency operator""_hz(unsigned long long const value) noexcept
  {
    return Frequency::from_hertz(static_cast<Real>(value));
  }

  /**
   * @brief Creates a frequency from a kilohertz literal.
   * @ingroup core_measurements
   * @relates Frequency
   */
  [[nodiscard]] constexpr Frequency operator""_khz(long double const value) noexcept
  {
    return Frequency::from_kilohertz(static_cast<Real>(value));
  }

  /**
   * @brief Creates a frequency from a kilohertz literal.
   * @ingroup core_measurements
   * @relates Frequency
   */
  [[nodiscard]] constexpr Frequency operator""_khz(unsigned long long const value) noexcept
  {
    return Frequency::from_kilohertz(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Frequency)
#endif
