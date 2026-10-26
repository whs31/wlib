#pragma once

#include <concepts>
#include <chrono>
#include <string_view>

#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Frequency value stored in `Hz`.
 * @details Frequency is a small value type with hertz and kilohertz conversions. It
 * converts to and from the period as a `std::chrono` duration.
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
template <std::floating_point T>
class BasicFrequency : public QuantityBase<BasicFrequency<T>, T>
{
  using Base = QuantityBase<BasicFrequency, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "Hz";

  /**
   * @brief Creates a zero frequency.
   */
  constexpr BasicFrequency() noexcept = default;

  /**
   * @brief Creates a frequency from hertz.
   */
  explicit constexpr BasicFrequency(T const hertz) noexcept
    : Base(hertz)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicFrequency(
    BasicFrequency<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a frequency from hertz.
   */
  [[nodiscard]] static constexpr BasicFrequency from_hertz(T const hertz) noexcept
  {
    return BasicFrequency(hertz);
  }

  /**
   * @brief Creates a frequency from kilohertz.
   */
  [[nodiscard]] static constexpr BasicFrequency from_kilohertz(T const kilohertz) noexcept
  {
    return BasicFrequency(kilohertz * 1'000);
  }

  /**
   * @brief Returns hertz.
   */
  [[nodiscard]] constexpr T as_hertz() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns kilohertz.
   */
  [[nodiscard]] constexpr T as_kilohertz() const noexcept { return this->magnitude() / 1'000; }

  /**
   * @brief Creates the frequency of an event that repeats every @p period.
   * @details A zero period yields an infinite frequency.
   */
  template <typename Rep, typename Period>
  [[nodiscard]] static constexpr BasicFrequency from_period(
    std::chrono::duration<Rep, Period> const period
  ) noexcept
  {
    return BasicFrequency(static_cast<T>(1 / std::chrono::duration<double>(period).count()));
  }

  /**
   * @brief Returns the time between two events at this frequency.
   * @details A zero frequency yields an infinite period.
   */
  [[nodiscard]] constexpr std::chrono::duration<T> period() const noexcept
  {
    return std::chrono::duration<T>(1 / this->magnitude());
  }
};

/**
 * @brief Frequency in single precision, the default.
 * @ingroup core_measurements
 */
using Frequency = BasicFrequency<float>;

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
