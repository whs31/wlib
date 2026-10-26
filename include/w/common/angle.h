#pragma once

#include <cmath>
#include <limits>
#include <numbers>
#include <string>
#include <string_view>

#include <w/common/concepts.h>
#include <w/common/config.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/quantity.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Angle value stored in radians.
 * @details Angle is a small value type with degree/radian conversions,
 * wrapping helpers, interpolation along the shortest path, and Qt/QML
 * properties.
 * @note Qt properties, QString conversion, and metatype registration are
 * available only when wlib is built with `with_qt=True`.
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
class W_COMMON_API Angle : public QuantityBase<Angle>
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double radians READ radians CONSTANT FINAL)
  Q_PROPERTY(double degrees READ degrees CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Symbol of the SI unit the magnitude is stored in.
   */
  static constexpr std::string_view si_symbol = "rad";

  /**
   * @brief Creates a zero angle.
   */
  constexpr Angle() noexcept = default;

  /**
   * @brief Creates an angle from radians.
   */
  explicit constexpr Angle(Real const radians) noexcept
    : QuantityBase(radians)
  {}

  /**
   * @brief Returns radians.
   */
  [[nodiscard]] constexpr Real radians() const noexcept { return this->magnitude(); }

  /**
   * @brief Returns degrees.
   */
  [[nodiscard]] constexpr Real degrees() const noexcept { return to_degrees(this->magnitude()); }

  /**
   * @brief Returns the angle wrapped into `[0, 2*pi)`.
   */
  [[nodiscard]] constexpr Angle positive() const noexcept
  {
    auto constexpr two_pi = std::numbers::pi_v<Real> * 2;
    auto radians = fmod(this->magnitude(), two_pi);
    if(radians < 0)
      radians += two_pi;
    if(approx_eq(radians, two_pi, std::numeric_limits<Real>::epsilon() * 3))
      radians = 0;
    return from_radians(radians);
  }

  /**
   * @brief Returns the equivalent signed angle in `[-pi, pi]`.
   */
  // The trailing underscore avoids the C++ keyword while preserving the public API.
  [[nodiscard]] constexpr Angle signed_() const noexcept  // NOLINT(readability-identifier-naming)
  {
    auto inner = pi();
    inner -= *this;
    inner = inner.positive();

    auto ret = pi();
    ret -= inner;
    if(is_null(ret.radians(), std::numeric_limits<Real>::epsilon()))
      return zero();
    return ret;
  }

  /**
   * @brief Returns the shortest signed angle from this angle to @p other.
   * @details The result accounts for wrapping and is suitable for steering or
   * heading interpolation.
   */
  [[nodiscard]] constexpr Angle angle_to(Angle const& other) const noexcept
  {
    auto constexpr max = std::numbers::pi_v<Real> * 2;
    auto const d = fmod(other.radians() - this->radians(), max);
    return from_radians(fmod(2 * d, max) - d);
  }

  /**
   * @brief Linearly interpolates toward @p other using the shortest path.
   * @param other Target angle.
   * @param t Interpolation factor.
   */
  template <Number T>
  [[nodiscard]] Angle lerp(Angle const& other, T const t) const noexcept
  {
    auto delta = this->angle_to(other);
    delta *= static_cast<Real>(t);

    auto result = *this;
    result += delta;
    return result;
  }

  [[nodiscard]] Real sin() const noexcept { return std::sin(this->magnitude()); }

  [[nodiscard]] Real cos() const noexcept { return std::cos(this->magnitude()); }

  [[nodiscard]] Real tan() const noexcept { return std::tan(this->magnitude()); }

  [[nodiscard]] Real asin() const noexcept { return std::asin(this->magnitude()); }

  [[nodiscard]] Real acos() const noexcept { return std::acos(this->magnitude()); }

  [[nodiscard]] Real atan() const noexcept { return std::atan(this->magnitude()); }

  /**
   * @brief Returns a compact string in degrees.
   */
  // Replaces the SI display of QuantityBase::to_string().
  // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
  [[nodiscard]] std::string to_string() const;

  /**
   * @brief Creates an angle from radians.
   */
  [[nodiscard]] static constexpr Angle from_radians(Real const radians) noexcept
  {
    return Angle(radians);
  }

  /**
   * @brief Creates an angle from degrees.
   */
  [[nodiscard]] static constexpr Angle from_degrees(Real const degrees) noexcept
  {
    return Angle(to_radians(degrees));
  }

  [[nodiscard]] static constexpr Angle pi() noexcept
  {
    return from_radians(std::numbers::pi_v<Real>);
  }

  [[nodiscard]] static constexpr Angle tau() noexcept
  {
    return from_radians(std::numbers::pi_v<Real> * 2);
  }

  [[nodiscard]] static constexpr Angle frac_pi_2() noexcept
  {
    return from_radians(std::numbers::pi_v<Real> / 2);
  }

  [[nodiscard]] static constexpr Angle frac_pi_3() noexcept
  {
    return from_radians(std::numbers::pi_v<Real> / 3);
  }

  [[nodiscard]] static constexpr Angle frac_pi_4() noexcept
  {
    return from_radians(std::numbers::pi_v<Real> / 4);
  }

  using QuantityBase::operator+=;
  using QuantityBase::operator-=;

  constexpr Angle& operator+=(Real const radians) noexcept { return *this += Angle(radians); }

  constexpr Angle& operator-=(Real const radians) noexcept { return *this -= Angle(radians); }
};

[[nodiscard]] constexpr Angle operator+(Angle lhs, Real const radians) noexcept
{
  lhs += radians;
  return lhs;
}

[[nodiscard]] constexpr Angle operator+(Real const radians, Angle const rhs) noexcept
{
  return Angle(radians) + rhs;
}

[[nodiscard]] constexpr Angle operator-(Angle lhs, Real const radians) noexcept
{
  lhs -= radians;
  return lhs;
}

[[nodiscard]] constexpr Angle operator-(Real const radians, Angle const rhs) noexcept
{
  return Angle(radians) - rhs;
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
[[nodiscard]] constexpr Real to_degrees(Angle const angle) noexcept { return angle.degrees(); }

/**
 * @brief Returns @p angle in radians.
 * @ingroup core_math
 * @relates Angle
 */
[[nodiscard]] constexpr Real to_radians(Angle const angle) noexcept { return angle.radians(); }

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Angle)
#endif
