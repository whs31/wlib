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
#include <w/common/config.h>
#include <w/common/detail/format.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/math.h>
#include <w/common/vector2.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Three-dimensional vector backed by @p T values: a position, an offset or a direction.
 * @details The vector does not know its frame or unit; name the variable after them instead,
 * e.g. `velocity_ned`, `omega_body` or `position_world`. Whether it is a position or a direction
 * shows in how it is transformed: Matrix4x4::transform_point() translates it,
 * Matrix4x4::transform_direction() does not.
 *
 * @par Layout
 * The storage is exactly three contiguous @p T values in `x, y, z` order: 12 bytes, with no
 * padding and no SIMD alignment. The class is standard layout and trivially copyable, so it can be
 * reflected by offset or bit-cast to `std::array<T, 3>`. The layout is part of the API and is
 * checked by `static_assert`s in this header. A hot loop that needs SIMD gets batch functions over
 * `std::span<Vector3>`, never padding.
 *
 * @ingroup core_measurements
 * @see Vector2
 * @see Vector4
 * @see Matrix3x3
 */
template <std::floating_point T>
class BasicVector3
{
 public:
  constexpr BasicVector3() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicVector3(
    BasicVector3<U> const& other
  ) noexcept
    : BasicVector3(static_cast<T>(other.x()), static_cast<T>(other.y()), static_cast<T>(other.z()))
  {}

  constexpr BasicVector3(T const x, T const y, T const z) noexcept
    : m_x(x)
    , m_y(y)
    , m_z(z)
  {}

  // The default z intentionally preserves the implicit Vector2-to-Vector3 conversion.
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr BasicVector3(BasicVector2<T> const vector, T const z = 0.0) noexcept
    : m_x(vector.x())
    , m_y(vector.y())
    , m_z(z)
  {}

  [[nodiscard]] constexpr T x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr T y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr T z() const noexcept { return this->m_z; }

  [[nodiscard]] constexpr BasicVector2<T> xy() const noexcept { return {this->m_x, this->m_y}; }

  constexpr void set_x(T const x) noexcept { this->m_x = x; }

  constexpr void set_y(T const y) noexcept { this->m_y = y; }

  constexpr void set_z(T const z) noexcept { this->m_z = z; }

  /**
   * @brief Returns a copy with the x component replaced.
   */
  [[nodiscard]] constexpr BasicVector3<T> with_x(T const x) const noexcept
  {
    return {x, this->m_y, this->m_z};
  }

  /**
   * @brief Returns a copy with the y component replaced.
   */
  [[nodiscard]] constexpr BasicVector3<T> with_y(T const y) const noexcept
  {
    return {this->m_x, y, this->m_z};
  }

  /**
   * @brief Returns a copy with the z component replaced.
   */
  [[nodiscard]] constexpr BasicVector3<T> with_z(T const z) const noexcept
  {
    return {this->m_x, this->m_y, z};
  }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_x) and common::is_finite(this->m_y)
       and common::is_finite(this->m_z);
  }

  [[nodiscard]] constexpr T length_squared() const noexcept
  {
    return this->m_x * this->m_x + this->m_y * this->m_y + this->m_z * this->m_z;
  }

  [[nodiscard]] T length() const noexcept { return std::sqrt(this->length_squared()); }

  [[nodiscard]] constexpr T dot(BasicVector3<T> const& other) const noexcept
  {
    return this->m_x * other.m_x + this->m_y * other.m_y + this->m_z * other.m_z;
  }

  [[nodiscard]] constexpr BasicVector3<T> cross(BasicVector3<T> const& other) const noexcept
  {
    return {
      this->m_y * other.m_z - this->m_z * other.m_y,
      this->m_z * other.m_x - this->m_x * other.m_z,
      this->m_x * other.m_y - this->m_y * other.m_x
    };
  }

  /**
   * @brief Returns the unit vector in the same direction.
   * @details Returns the zero vector when the length is approximately zero.
   * @see try_normalized
   */
  [[nodiscard]] BasicVector3<T> normalized() const noexcept
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
  [[nodiscard]] std::optional<BasicVector3<T>> try_normalized() const noexcept
  {
    auto const len = this->length();
    if(is_null(len) or not common::is_finite(len))
      return std::nullopt;

    auto result = *this;
    result /= len;
    return result;
  }

  /**
   * @brief Returns the distance from this position to the position @p other.
   */
  [[nodiscard]] T distance_to(BasicVector3<T> const& other) const noexcept
  {
    return BasicVector3<T>(other.m_x - this->m_x, other.m_y - this->m_y, other.m_z - this->m_z)
      .length();
  }

  /**
   * @brief Returns the component-wise product, e.g. a vector scaled per axis.
   */
  [[nodiscard]] constexpr BasicVector3<T> component_mul(BasicVector3<T> const& other) const noexcept
  {
    return {this->m_x * other.m_x, this->m_y * other.m_y, this->m_z * other.m_z};
  }

  /**
   * @brief Returns the component-wise quotient.
   */
  [[nodiscard]] constexpr BasicVector3<T> component_div(BasicVector3<T> const& other) const noexcept
  {
    return {this->m_x / other.m_x, this->m_y / other.m_y, this->m_z / other.m_z};
  }

  /**
   * @brief Returns the component-wise minimum.
   */
  [[nodiscard]] constexpr BasicVector3<T> min(BasicVector3<T> const& other) const noexcept
  {
    return {
      std::min(this->m_x, other.m_x),
      std::min(this->m_y, other.m_y),
      std::min(this->m_z, other.m_z),
    };
  }

  /**
   * @brief Returns the component-wise maximum.
   */
  [[nodiscard]] constexpr BasicVector3<T> max(BasicVector3<T> const& other) const noexcept
  {
    return {
      std::max(this->m_x, other.m_x),
      std::max(this->m_y, other.m_y),
      std::max(this->m_z, other.m_z),
    };
  }

  /**
   * @brief Returns the component-wise absolute value.
   */
  [[nodiscard]] constexpr BasicVector3<T> abs() const noexcept
  {
    return {common::abs(this->m_x), common::abs(this->m_y), common::abs(this->m_z)};
  }

  /**
   * @brief Returns `*this + (other - *this) * t`; @p t is not clamped.
   */
  [[nodiscard]] constexpr BasicVector3<T> lerp(
    BasicVector3<T> const& other,
    T const t
  ) const noexcept
  {
    return {
      this->m_x + (other.m_x - this->m_x) * t,
      this->m_y + (other.m_y - this->m_y) * t,
      this->m_z + (other.m_z - this->m_z) * t,
    };
  }

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  [[nodiscard]] static constexpr BasicVector3<T> zero() noexcept { return {}; }

  /**
   * @brief Returns the unit vector `(1, 0, 0)`.
   */
  [[nodiscard]] static constexpr BasicVector3<T> unit_x() noexcept { return {1.0, 0.0, 0.0}; }

  /**
   * @brief Returns the unit vector `(0, 1, 0)`.
   */
  [[nodiscard]] static constexpr BasicVector3<T> unit_y() noexcept { return {0.0, 1.0, 0.0}; }

  /**
   * @brief Returns the unit vector `(0, 0, 1)`.
   */
  [[nodiscard]] static constexpr BasicVector3<T> unit_z() noexcept { return {0.0, 0.0, 1.0}; }

  [[nodiscard]] static constexpr BasicVector3<T> splat(T const value) noexcept
  {
    return {value, value, value};
  }

  [[nodiscard]] static constexpr BasicVector3<T> from_xyz(T const x, T const y, T const z) noexcept
  {
    return {x, y, z};
  }

  [[nodiscard]] static constexpr BasicVector3<T> from_vector2(
    BasicVector2<T> const vector,
    T const z = 0.0
  ) noexcept
  {
    return {vector, z};
  }

  [[nodiscard]] constexpr bool operator==(BasicVector3<T> const&) const noexcept = default;

  [[nodiscard]] constexpr std::partial_ordering operator<=>(BasicVector3<T> const&) const noexcept =
    default;

  [[nodiscard]] constexpr BasicVector3<T> operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr BasicVector3<T> operator-() const noexcept
  {
    return {-this->m_x, -this->m_y, -this->m_z};
  }

  constexpr BasicVector3<T>& operator+=(BasicVector3<T> const rhs) noexcept
  {
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    this->m_z += rhs.m_z;
    return *this;
  }

  constexpr BasicVector3<T>& operator-=(BasicVector3<T> const rhs) noexcept
  {
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    this->m_z -= rhs.m_z;
    return *this;
  }

  constexpr BasicVector3<T>& operator*=(T const rhs) noexcept
  {
    this->m_x *= rhs;
    this->m_y *= rhs;
    this->m_z *= rhs;
    return *this;
  }

  constexpr BasicVector3<T>& operator/=(T const rhs) noexcept
  {
    this->m_x /= rhs;
    this->m_y /= rhs;
    this->m_z /= rhs;
    return *this;
  }

  [[nodiscard]] friend constexpr BasicVector3<T> operator+(
    BasicVector3<T> lhs,
    BasicVector3<T> const rhs
  ) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector3<T> operator-(
    BasicVector3<T> lhs,
    BasicVector3<T> const rhs
  ) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector3<T> operator*(
    BasicVector3<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs *= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector3<T> operator*(
    T const lhs,
    BasicVector3<T> const rhs
  ) noexcept
  {
    return rhs * lhs;
  }

  [[nodiscard]] friend constexpr BasicVector3<T> operator/(
    BasicVector3<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs /= rhs;
    return lhs;
  }

 private:
  T m_x = 0.0;
  T m_y = 0.0;
  T m_z = 0.0;
};

/**
 * @brief Vector3 in single precision, the default.
 */
using Vector3 = BasicVector3<float>;

/**
 * @brief Returns the dot product `a · b`.
 * @relates Vector3
 */
template <std::floating_point T>
[[nodiscard]] constexpr T dot(BasicVector3<T> const& a, BasicVector3<T> const& b) noexcept
{
  return a.dot(b);
}

/**
 * @brief Returns the right-handed cross product `a × b`.
 * @relates Vector3
 */
template <std::floating_point T>
[[nodiscard]] constexpr BasicVector3<T> cross(
  BasicVector3<T> const& a,
  BasicVector3<T> const& b
) noexcept
{
  return a.cross(b);
}

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicVector3<T> const& vector)
{
  return os << vector.to_string();
}

static_assert(std::is_standard_layout_v<Vector3>);
static_assert(std::is_trivially_copyable_v<Vector3>);
static_assert(sizeof(Vector3) == 12, "Vector3 is three floats without SIMD padding");
static_assert(
  std::bit_cast<std::array<float, 3>>(Vector3(1.0, 2.0, 3.0)) == std::array<float, 3> {1, 2, 3},
  "Vector3 must store x, y, z contiguously"
);

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicVector3<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicVector3<T> const vector) const noexcept
  {
    auto hash = std::hash<T> {}(vector.x());
    hash ^= std::hash<T> {}(vector.y()) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    hash ^= std::hash<T> {}(vector.z()) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash;
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(x, y, z)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicVector3<T>> : formatter<T>
{
  format_context::iterator format(
    w::common::BasicVector3<T> const vector,
    format_context& ctx
  ) const
  {
    return w::common::_format_impl::
      format_components<3, T>(*this, {vector.x(), vector.y(), vector.z()}, ctx);
  }
};
#endif
