#pragma once

#include <concepts>
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <compare>
#include <functional>
#include <iosfwd>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <w/common/stdint.h>
#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/detail/format.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/math.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Two-dimensional vector backed by @p T values: a position, an offset or a direction.
 * @details The vector does not know its frame or unit; name the variable after them instead,
 * e.g. `cursor_px` or `offset_world`.
 *
 * @par Layout
 * The storage is exactly two contiguous @p T values in `x, y` order, with no padding. The class is
 * standard layout and trivially copyable, so it can be reflected by offset or bit-cast to
 * `std::array<T, 2>`. The
 * layout is part of the API and is checked by `static_assert`s in this header.
 *
 * @ingroup core_measurements
 * @see Vector3
 * @see Angle
 */
template <std::floating_point T>
class BasicVector2
{
 public:
  constexpr BasicVector2() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicVector2(
    BasicVector2<U> const& other
  ) noexcept
    : BasicVector2(static_cast<T>(other.x()), static_cast<T>(other.y()))
  {}

  constexpr BasicVector2(T const x, T const y) noexcept
    : m_x(x)
    , m_y(y)
  {}

  [[nodiscard]] constexpr T x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr T y() const noexcept { return this->m_y; }

  constexpr void set_x(T const x) noexcept { this->m_x = x; }

  constexpr void set_y(T const y) noexcept { this->m_y = y; }

  /**
   * @brief Returns a copy with the x component replaced.
   */
  [[nodiscard]] constexpr BasicVector2<T> with_x(T const x) const noexcept
  {
    return {x, this->m_y};
  }

  /**
   * @brief Returns a copy with the y component replaced.
   */
  [[nodiscard]] constexpr BasicVector2<T> with_y(T const y) const noexcept
  {
    return {this->m_x, y};
  }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_x) and common::is_finite(this->m_y);
  }

  [[nodiscard]] constexpr T length_squared() const noexcept
  {
    return this->m_x * this->m_x + this->m_y * this->m_y;
  }

  [[nodiscard]] T length() const noexcept { return std::sqrt(this->length_squared()); }

  [[nodiscard]] BasicAngle<T> angle() const noexcept
  {
    return BasicAngle<T>::from_radians(std::atan2(this->m_y, this->m_x));
  }

  [[nodiscard]] constexpr T dot(BasicVector2<T> const& other) const noexcept
  {
    return this->m_x * other.m_x + this->m_y * other.m_y;
  }

  [[nodiscard]] constexpr T cross(BasicVector2<T> const& other) const noexcept
  {
    return this->m_x * other.m_y - this->m_y * other.m_x;
  }

  /**
   * @brief Returns the shortest signed angle from the direction of this vector to the direction of
   * @p other.
   * @see angle_towards
   */
  [[nodiscard]] BasicAngle<T> angle_to(BasicVector2<T> const& other) const noexcept
  {
    return this->angle().angle_to(other.angle());
  }

  /**
   * @brief Returns the direction from this position to the position @p target: the angle of
   * `target - *this`.
   * @see angle_to
   */
  [[nodiscard]] BasicAngle<T> angle_towards(BasicVector2<T> const& target) const noexcept
  {
    return BasicVector2<T>(target.m_x - this->m_x, target.m_y - this->m_y).angle();
  }

  /**
   * @brief Returns the distance from this position to the position @p other.
   */
  [[nodiscard]] T distance_to(BasicVector2<T> const& other) const noexcept
  {
    return BasicVector2<T>(other.m_x - this->m_x, other.m_y - this->m_y).length();
  }

  /**
   * @brief Returns the component-wise product.
   */
  [[nodiscard]] constexpr BasicVector2<T> component_mul(BasicVector2<T> const& other) const noexcept
  {
    return {this->m_x * other.m_x, this->m_y * other.m_y};
  }

  /**
   * @brief Returns the component-wise quotient.
   */
  [[nodiscard]] constexpr BasicVector2<T> component_div(BasicVector2<T> const& other) const noexcept
  {
    return {this->m_x / other.m_x, this->m_y / other.m_y};
  }

  /**
   * @brief Returns the component-wise minimum.
   */
  [[nodiscard]] constexpr BasicVector2<T> min(BasicVector2<T> const& other) const noexcept
  {
    return {std::min(this->m_x, other.m_x), std::min(this->m_y, other.m_y)};
  }

  /**
   * @brief Returns the component-wise maximum.
   */
  [[nodiscard]] constexpr BasicVector2<T> max(BasicVector2<T> const& other) const noexcept
  {
    return {std::max(this->m_x, other.m_x), std::max(this->m_y, other.m_y)};
  }

  /**
   * @brief Returns the component-wise absolute value.
   */
  [[nodiscard]] constexpr BasicVector2<T> abs() const noexcept
  {
    return {common::abs(this->m_x), common::abs(this->m_y)};
  }

  /**
   * @brief Returns `*this + (other - *this) * t`; @p t is not clamped.
   */
  [[nodiscard]] constexpr BasicVector2<T> lerp(
    BasicVector2<T> const& other,
    T const t
  ) const noexcept
  {
    return {
      this->m_x + (other.m_x - this->m_x) * t,
      this->m_y + (other.m_y - this->m_y) * t,
    };
  }

  /**
   * @brief Returns the unit vector in the same direction.
   * @details Returns the zero vector when the length is approximately zero.
   * @see try_normalized
   */
  [[nodiscard]] BasicVector2<T> normalized() const noexcept
  {
    auto const len = this->length();
    if(is_null(len))
      return {};

    auto result = *this;
    result /= len;
    return result;
  }

  /**
   * @brief Returns the unit vector in the same direction, or `std::nullopt` when the length is
   * approximately zero or not finite.
   * @see normalized
   */
  [[nodiscard]] std::optional<BasicVector2<T>> try_normalized() const noexcept
  {
    auto const len = this->length();
    if(is_null(len) or not common::is_finite(len))
      return std::nullopt;

    auto result = *this;
    result /= len;
    return result;
  }

  [[nodiscard]] BasicVector2<T> rotated(BasicAngle<T> const angle) const noexcept
  {
    auto const c = angle.cos();
    auto const s = angle.sin();
    return {this->m_x * c - this->m_y * s, this->m_x * s + this->m_y * c};
  }

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  [[nodiscard]] static constexpr BasicVector2<T> zero() noexcept { return {}; }

  /**
   * @brief Returns the unit vector `(1, 0)`.
   */
  [[nodiscard]] static constexpr BasicVector2<T> unit_x() noexcept { return {1.0, 0.0}; }

  /**
   * @brief Returns the unit vector `(0, 1)`.
   */
  [[nodiscard]] static constexpr BasicVector2<T> unit_y() noexcept { return {0.0, 1.0}; }

  [[nodiscard]] static constexpr BasicVector2<T> splat(T const value) noexcept
  {
    return {value, value};
  }

  [[nodiscard]] static constexpr BasicVector2<T> from_xy(T const x, T const y) noexcept
  {
    return {x, y};
  }

  [[nodiscard]] static BasicVector2<T> from_angle(BasicAngle<T> const angle) noexcept
  {
    return {angle.cos(), angle.sin()};
  }

  [[nodiscard]] constexpr bool operator==(BasicVector2<T> const&) const noexcept = default;

  [[nodiscard]] constexpr std::partial_ordering operator<=>(BasicVector2<T> const&) const noexcept =
    default;

  [[nodiscard]] constexpr BasicVector2<T> operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr BasicVector2<T> operator-() const noexcept
  {
    return {-this->m_x, -this->m_y};
  }

  constexpr BasicVector2<T>& operator+=(BasicVector2<T> const rhs) noexcept
  {
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    return *this;
  }

  constexpr BasicVector2<T>& operator-=(BasicVector2<T> const rhs) noexcept
  {
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    return *this;
  }

  constexpr BasicVector2<T>& operator*=(T const rhs) noexcept
  {
    this->m_x *= rhs;
    this->m_y *= rhs;
    return *this;
  }

  constexpr BasicVector2<T>& operator/=(T const rhs) noexcept
  {
    this->m_x /= rhs;
    this->m_y /= rhs;
    return *this;
  }

  [[nodiscard]] friend constexpr BasicVector2<T> operator+(
    BasicVector2<T> lhs,
    BasicVector2<T> const rhs
  ) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector2<T> operator-(
    BasicVector2<T> lhs,
    BasicVector2<T> const rhs
  ) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector2<T> operator*(
    BasicVector2<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs *= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector2<T> operator*(
    T const lhs,
    BasicVector2<T> const rhs
  ) noexcept
  {
    return rhs * lhs;
  }

  [[nodiscard]] friend constexpr BasicVector2<T> operator/(
    BasicVector2<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs /= rhs;
    return lhs;
  }

 private:
  T m_x = 0.0;
  T m_y = 0.0;
};

/**
 * @brief Vector2 in single precision, the default.
 */
using Vector2 = BasicVector2<float>;

/**
 * @brief Returns the dot product `a · b`.
 * @relates Vector2
 */
template <std::floating_point T>
[[nodiscard]] constexpr T dot(BasicVector2<T> const& a, BasicVector2<T> const& b) noexcept
{
  return a.dot(b);
}

/**
 * @brief Returns the z component of the cross product `a × b` of the vectors lifted to 3D.
 * @relates Vector2
 */
template <std::floating_point T>
[[nodiscard]] constexpr T cross(BasicVector2<T> const& a, BasicVector2<T> const& b) noexcept
{
  return a.cross(b);
}

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicVector2<T> const& vector)
{
  return os << vector.to_string();
}

static_assert(std::is_standard_layout_v<Vector2>);
static_assert(std::is_trivially_copyable_v<Vector2>);
static_assert(sizeof(Vector2) == 8);
static_assert(
  std::bit_cast<std::array<float, 2>>(Vector2(1.0, 2.0)) == std::array<float, 2> {1, 2},
  "Vector2 must store x, y contiguously"
);

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicVector2<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicVector2<T> const vector) const noexcept
  {
    auto hash = std::hash<T> {}(vector.x());
    hash ^= std::hash<T> {}(vector.y()) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash;
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(x, y)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicVector2<T>> : formatter<T>
{
  format_context::iterator format(
    w::common::BasicVector2<T> const vector,
    format_context& ctx
  ) const
  {
    return w::common::_format_impl::format_components<2, T>(*this, {vector.x(), vector.y()}, ctx);
  }
};
#endif
