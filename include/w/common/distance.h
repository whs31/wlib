#pragma once

#include <concepts>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <type_traits>

#include <w/common/config.h>
#include <w/common/constants.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Distance value stored in meters.
 * @details Distance is a small value type with common metric and imperial
 * conversions. Arithmetic combines the stored meter values directly, matching
 * the lightweight Rust type this class replaces. Dividing two distances yields
 * their dimensionless ratio.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto total = 1.2_km + 300_m;
 * fmt::print("{}\n", total); // "1.5 km"
 * auto ratio = 50_m / 10_m;  // 5.0
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see Velocity
 */
template <std::floating_point T>
class BasicDistance : public QuantityBase<BasicDistance<T>, T>
{
  using Base = QuantityBase<BasicDistance, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "m";

  /**
   * @brief Creates a zero distance.
   */
  constexpr BasicDistance() noexcept = default;

  /**
   * @brief Creates a distance from meters.
   */
  explicit constexpr BasicDistance(T const meters) noexcept
    : Base(meters)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicDistance(
    BasicDistance<U> const other
  ) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Creates a distance from meters.
   */
  [[nodiscard]] static constexpr BasicDistance new_distance(T const meters) noexcept
  {
    return BasicDistance(meters);
  }

  /**
   * @brief Creates a distance from meters.
   */
  [[nodiscard]] static constexpr BasicDistance from_meters(T const meters) noexcept
  {
    return BasicDistance(meters);
  }

  [[nodiscard]] static constexpr BasicDistance from_nanometers(T const nanometers) noexcept
  {
    return from_meters(nanometers / static_cast<T>(METER_NANOMETER_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_micrometers(T const micrometers) noexcept
  {
    return from_meters(micrometers / static_cast<T>(METER_MICROMETER_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_millimeters(T const millimeters) noexcept
  {
    return from_meters(millimeters / static_cast<T>(METER_MILLIMETER_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_centimeters(T const centimeters) noexcept
  {
    return from_meters(centimeters / static_cast<T>(METER_CENTIMETER_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_decimeters(T const decimeters) noexcept
  {
    return from_meters(decimeters / static_cast<T>(METER_DECIMETER_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_hectometers(T const hectometers) noexcept
  {
    return from_meters(hectometers / static_cast<T>(METER_HECTOMETER_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_kilometers(T const kilometers) noexcept
  {
    return from_meters(kilometers / static_cast<T>(METER_KILOMETER_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_inches(T const inches) noexcept
  {
    return from_meters(inches / static_cast<T>(METER_INCH_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_feet(T const feet) noexcept
  {
    return from_meters(feet / static_cast<T>(METER_FEET_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_yards(T const yards) noexcept
  {
    return from_meters(yards / static_cast<T>(METER_YARD_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_furlongs(T const furlongs) noexcept
  {
    return from_meters(furlongs / static_cast<T>(METER_FURLONG_FACTOR));
  }

  [[nodiscard]] static constexpr BasicDistance from_miles(T const miles) noexcept
  {
    return from_meters(miles / static_cast<T>(METER_MILE_FACTOR));
  }

  [[nodiscard]] constexpr T as_nm() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_NANOMETER_FACTOR);
  }

  [[nodiscard]] constexpr T as_um() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_MICROMETER_FACTOR);
  }

  [[nodiscard]] constexpr T as_mm() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_MILLIMETER_FACTOR);
  }

  [[nodiscard]] constexpr T as_cm() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_CENTIMETER_FACTOR);
  }

  [[nodiscard]] constexpr T as_dm() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_DECIMETER_FACTOR);
  }

  [[nodiscard]] constexpr T as_m() const noexcept { return this->magnitude(); }

  [[nodiscard]] constexpr T as_hm() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_HECTOMETER_FACTOR);
  }

  [[nodiscard]] constexpr T as_km() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_KILOMETER_FACTOR);
  }

  [[nodiscard]] constexpr T as_inches() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_INCH_FACTOR);
  }

  [[nodiscard]] constexpr T as_feet() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_FEET_FACTOR);
  }

  [[nodiscard]] constexpr T as_yards() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_YARD_FACTOR);
  }

  [[nodiscard]] constexpr T as_furlongs() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_FURLONG_FACTOR);
  }

  [[nodiscard]] constexpr T as_miles() const noexcept
  {
    return this->magnitude() * static_cast<T>(METER_MILE_FACTOR);
  }

  /**
   * @brief Returns a compact string using millimeters, centimeters, meters, or kilometers.
   */
  // Replaces the SI display of QuantityBase::to_string().
  // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
  [[nodiscard]] std::string to_string() const
  {
    auto const magnitude = abs(this->magnitude());
    if(magnitude < T {0.1})
      return fmt::format("{:.6g} mm", this->as_mm());
    if(magnitude < 1)
      return fmt::format("{:.6g} cm", this->as_cm());
    if(magnitude < 1'000)
      return fmt::format("{:.6g} m", this->as_m());
    return fmt::format("{:.6g} km", this->as_km());
  }

  using Base::operator+=;
  using Base::operator-=;

  constexpr BasicDistance& operator+=(T const meters) noexcept
  {
    return *this += BasicDistance(meters);
  }

  constexpr BasicDistance& operator-=(T const meters) noexcept
  {
    return *this -= BasicDistance(meters);
  }
};

/**
 * @brief Distance in single precision, the default.
 * @ingroup core_measurements
 */
using Distance = BasicDistance<float>;

template <std::floating_point T>
[[nodiscard]] constexpr BasicDistance<T> operator+(
  BasicDistance<T> lhs,
  std::type_identity_t<T> const meters
) noexcept
{
  lhs += meters;
  return lhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicDistance<T> operator+(
  std::type_identity_t<T> const meters,
  BasicDistance<T> const rhs
) noexcept
{
  return BasicDistance<T>(meters) + rhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicDistance<T> operator-(
  BasicDistance<T> lhs,
  std::type_identity_t<T> const meters
) noexcept
{
  lhs -= meters;
  return lhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicDistance<T> operator-(
  std::type_identity_t<T> const meters,
  BasicDistance<T> const rhs
) noexcept
{
  return BasicDistance<T>(meters) - rhs;
}

inline namespace literals
{
  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_meters(long double const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_meters(unsigned long long const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_kilometers(long double const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_kilometers(unsigned long long const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_m(long double const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a meter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_m(unsigned long long const value) noexcept
  {
    return Distance::from_meters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_km(long double const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a kilometer literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_km(unsigned long long const value) noexcept
  {
    return Distance::from_kilometers(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a centimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_cm(long double const value) noexcept
  {
    return Distance::from_centimeters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a centimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_cm(unsigned long long const value) noexcept
  {
    return Distance::from_centimeters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a millimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_mm(long double const value) noexcept
  {
    return Distance::from_millimeters(static_cast<Real>(value));
  }

  /**
   * @brief Creates a distance from a millimeter literal.
   * @ingroup core_measurements
   * @relates Distance
   */
  [[nodiscard]] constexpr Distance operator""_mm(unsigned long long const value) noexcept
  {
    return Distance::from_millimeters(static_cast<Real>(value));
  }
}  // namespace literals

W_COMMON_NAMESPACE_END
