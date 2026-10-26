#pragma once

#include <concepts>
#include <cmath>
#include <limits>
#include <numbers>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <type_traits>

#include <w/common/concepts.h>
#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/quantity.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Angle value stored in radians.
 * @details A small value type with degree/radian conversions, wrapping helpers and interpolation
 * along the shortest path. `Angle` is the single-precision instantiation.
 *
 * @code {.cpp}
 * auto heading = w::common::Angle::from_degrees(350.0);
 * auto target = w::common::Angle::from_degrees(10.0);
 * auto halfway = heading.lerp(target, 0.5); // 0 degrees
 * @endcode
 *
 * @ingroup core_measurements
 * @see QuantityBase
 * @see to_degrees
 * @see to_radians
 */
template <std::floating_point T>
class BasicAngle : public QuantityBase<BasicAngle<T>, T>
{
  using Base = QuantityBase<BasicAngle, T>;

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "rad";

  /**
   * @brief Creates a zero angle.
   */
  constexpr BasicAngle() noexcept = default;

  /**
   * @brief Creates an angle from radians.
   */
  explicit constexpr BasicAngle(T const radians) noexcept
    : Base(radians)
  {}

  /**
   * @brief Converts the magnitude from another precision.
   * @details Implicit where no digits are lost, e.g. from `float` to `double`, and explicit
   * otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicAngle(BasicAngle<U> const other) noexcept
    : Base(static_cast<T>(static_cast<U>(other)))
  {}

  /**
   * @brief Returns radians.
   */
  [[nodiscard]] constexpr T radians() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns degrees.
   */
  [[nodiscard]] constexpr T degrees() const noexcept { return to_degrees(this->magnitude()); }

  /**
   * @brief Returns the angle wrapped into `[0, 2*pi)`.
   */
  [[nodiscard]] constexpr BasicAngle positive() const noexcept
  {
    auto constexpr two_pi = std::numbers::pi_v<T> * 2;
    auto radians = fmod(this->magnitude(), two_pi);
    if(radians < 0)
      radians += two_pi;
    if(approx_eq(radians, two_pi, std::numeric_limits<T>::epsilon() * 3))
      radians = 0;
    return from_radians(radians);
  }

  /**
   * @brief Returns the equivalent signed angle in `[-pi, pi]`.
   */
  // The trailing underscore avoids the C++ keyword while preserving the public API.
  [[nodiscard]] constexpr BasicAngle
    signed_() const noexcept  // NOLINT(readability-identifier-naming)
  {
    auto inner = pi();
    inner -= *this;
    inner = inner.positive();

    auto ret = pi();
    ret -= inner;
    if(is_null(ret.radians(), std::numeric_limits<T>::epsilon()))
      return Base::zero();
    return ret;
  }

  /**
   * @brief Returns the shortest signed angle from this angle to @p other.
   * @details The result accounts for wrapping and is suitable for steering or
   * heading interpolation.
   */
  [[nodiscard]] constexpr BasicAngle angle_to(BasicAngle const& other) const noexcept
  {
    auto constexpr max = std::numbers::pi_v<T> * 2;
    auto const d = fmod(other.radians() - this->radians(), max);
    return from_radians(fmod(2 * d, max) - d);
  }

  /**
   * @brief Linearly interpolates toward @p other using the shortest path.
   * @param other Target angle.
   * @param t Interpolation factor.
   */
  template <Number F>
  [[nodiscard]] BasicAngle lerp(BasicAngle const& other, F const t) const noexcept
  {
    auto delta = this->angle_to(other);
    delta *= static_cast<T>(t);

    auto result = *this;
    result += delta;
    return result;
  }

  [[nodiscard]] T sin() const noexcept { return std::sin(this->magnitude()); }

  [[nodiscard]] T cos() const noexcept { return std::cos(this->magnitude()); }

  [[nodiscard]] T tan() const noexcept { return std::tan(this->magnitude()); }

  [[nodiscard]] T asin() const noexcept { return std::asin(this->magnitude()); }

  [[nodiscard]] T acos() const noexcept { return std::acos(this->magnitude()); }

  [[nodiscard]] T atan() const noexcept { return std::atan(this->magnitude()); }

  /**
   * @brief Returns a compact string in degrees.
   */
  // Replaces the SI display of QuantityBase::to_string().
  // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
  [[nodiscard]] std::string to_string() const { return fmt::format("{:.6g} deg", this->degrees()); }

  /**
   * @brief Creates an angle from radians.
   */
  [[nodiscard]] static constexpr BasicAngle from_radians(T const radians) noexcept
  {
    return BasicAngle(radians);
  }

  /**
   * @brief Creates an angle from degrees.
   */
  [[nodiscard]] static constexpr BasicAngle from_degrees(T const degrees) noexcept
  {
    return BasicAngle(to_radians(degrees));
  }

  [[nodiscard]] static constexpr BasicAngle pi() noexcept
  {
    return from_radians(std::numbers::pi_v<T>);
  }

  [[nodiscard]] static constexpr BasicAngle tau() noexcept
  {
    return from_radians(std::numbers::pi_v<T> * 2);
  }

  [[nodiscard]] static constexpr BasicAngle frac_pi_2() noexcept
  {
    return from_radians(std::numbers::pi_v<T> / 2);
  }

  [[nodiscard]] static constexpr BasicAngle frac_pi_3() noexcept
  {
    return from_radians(std::numbers::pi_v<T> / 3);
  }

  [[nodiscard]] static constexpr BasicAngle frac_pi_4() noexcept
  {
    return from_radians(std::numbers::pi_v<T> / 4);
  }

  using Base::operator+=;
  using Base::operator-=;

  constexpr BasicAngle& operator+=(T const radians) noexcept
  {
    return *this += BasicAngle(radians);
  }

  constexpr BasicAngle& operator-=(T const radians) noexcept
  {
    return *this -= BasicAngle(radians);
  }
};

/**
 * @brief Angle in single precision, the default.
 * @ingroup core_measurements
 */
using Angle = BasicAngle<float>;

template <std::floating_point T>
[[nodiscard]] constexpr BasicAngle<T> operator+(
  BasicAngle<T> lhs,
  std::type_identity_t<T> const radians
) noexcept
{
  lhs += radians;
  return lhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicAngle<T> operator+(
  std::type_identity_t<T> const radians,
  BasicAngle<T> const rhs
) noexcept
{
  return BasicAngle<T>(radians) + rhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicAngle<T> operator-(
  BasicAngle<T> lhs,
  std::type_identity_t<T> const radians
) noexcept
{
  lhs -= radians;
  return lhs;
}

template <std::floating_point T>
[[nodiscard]] constexpr BasicAngle<T> operator-(
  std::type_identity_t<T> const radians,
  BasicAngle<T> const rhs
) noexcept
{
  return BasicAngle<T>(radians) - rhs;
}

inline namespace literals
{
  /**
   * @brief Creates an angle from a degree literal.
   * @ingroup core_measurements
   * @relates Angle
   */
  [[nodiscard]] constexpr Angle operator""_deg(long double const value) noexcept
  {
    return Angle::from_degrees(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angle from a degree literal.
   * @ingroup core_measurements
   * @relates Angle
   */
  [[nodiscard]] constexpr Angle operator""_deg(unsigned long long const value) noexcept
  {
    return Angle::from_degrees(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angle from a radian literal.
   * @ingroup core_measurements
   * @relates Angle
   */
  [[nodiscard]] constexpr Angle operator""_rad(long double const value) noexcept
  {
    return Angle::from_radians(static_cast<Real>(value));
  }

  /**
   * @brief Creates an angle from a radian literal.
   * @ingroup core_measurements
   * @relates Angle
   */
  [[nodiscard]] constexpr Angle operator""_rad(unsigned long long const value) noexcept
  {
    return Angle::from_radians(static_cast<Real>(value));
  }
}  // namespace literals

/**
 * @brief Returns @p angle in degrees.
 * @ingroup core_math
 * @relates Angle
 */
template <std::floating_point T>
[[nodiscard]] constexpr T to_degrees(BasicAngle<T> const angle) noexcept
{
  return angle.degrees();
}

/**
 * @brief Returns @p angle in radians.
 * @ingroup core_math
 * @relates Angle
 */
template <std::floating_point T>
[[nodiscard]] constexpr T to_radians(BasicAngle<T> const angle) noexcept
{
  return angle.radians();
}

W_COMMON_NAMESPACE_END
