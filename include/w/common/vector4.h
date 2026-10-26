#pragma once

#include <concepts>
#include <ostream>
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <compare>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/detail/format.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Four-dimensional vector backed by @p T values, mostly a homogeneous point or direction.
 * @details A position is `(x, y, z, 1)` and a direction is `(x, y, z, 0)`, so a Matrix4x4
 * translates the first and not the second; from_point() and from_direction() spell the choice out.
 *
 * @par Layout
 * The storage is exactly four contiguous @p T values in `x, y, z, w` order: 16 bytes, with no
 * padding, so it bit-casts to `std::array<T, 4>` and to an HLSL `float4`. The class is standard
 * layout and trivially copyable. The layout is part of the API and is checked by `static_assert`s
 * in this header.
 *
 * @ingroup core_math
 * @see Vector3
 * @see Matrix4x4
 */
template <std::floating_point T>
class BasicVector4
{
 public:
  constexpr BasicVector4() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicVector4(
    BasicVector4<U> const& other
  ) noexcept
    : BasicVector4(
        static_cast<T>(other.x()),
        static_cast<T>(other.y()),
        static_cast<T>(other.z()),
        static_cast<T>(other.w())
      )
  {}

  constexpr BasicVector4(T const x, T const y, T const z, T const w) noexcept
    : m_x(x)
    , m_y(y)
    , m_z(z)
    , m_w(w)
  {}

  constexpr BasicVector4(BasicVector3<T> const xyz, T const w) noexcept
    : m_x(xyz.x())
    , m_y(xyz.y())
    , m_z(xyz.z())
    , m_w(w)
  {}

  [[nodiscard]] constexpr T x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr T y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr T z() const noexcept { return this->m_z; }

  [[nodiscard]] constexpr T w() const noexcept { return this->m_w; }

  /**
   * @brief Returns `(x, y, z)`, dropping `w` without dividing by it.
   * @see perspective_divided
   */
  [[nodiscard]] constexpr BasicVector3<T> xyz() const noexcept
  {
    return {this->m_x, this->m_y, this->m_z};
  }

  /**
   * @brief Returns `(x, y, z) / w`, the Cartesian point of a homogeneous point.
   * @details A `w` of zero gives infinite or NaN components.
   */
  [[nodiscard]] constexpr BasicVector3<T> perspective_divided() const noexcept
  {
    return this->xyz() / this->m_w;
  }

  constexpr void set_x(T const x) noexcept { this->m_x = x; }

  constexpr void set_y(T const y) noexcept { this->m_y = y; }

  constexpr void set_z(T const z) noexcept { this->m_z = z; }

  constexpr void set_w(T const w) noexcept { this->m_w = w; }

  [[nodiscard]] constexpr BasicVector4<T> with_x(T const x) const noexcept
  {
    return {x, this->m_y, this->m_z, this->m_w};
  }

  [[nodiscard]] constexpr BasicVector4<T> with_y(T const y) const noexcept
  {
    return {this->m_x, y, this->m_z, this->m_w};
  }

  [[nodiscard]] constexpr BasicVector4<T> with_z(T const z) const noexcept
  {
    return {this->m_x, this->m_y, z, this->m_w};
  }

  [[nodiscard]] constexpr BasicVector4<T> with_w(T const w) const noexcept
  {
    return {this->m_x, this->m_y, this->m_z, w};
  }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_x) and common::is_finite(this->m_y)
       and common::is_finite(this->m_z) and common::is_finite(this->m_w);
  }

  [[nodiscard]] constexpr T dot(BasicVector4<T> const& other) const noexcept
  {
    return this->m_x * other.m_x + this->m_y * other.m_y + this->m_z * other.m_z
         + this->m_w * other.m_w;
  }

  [[nodiscard]] constexpr T length_squared() const noexcept { return this->dot(*this); }

  [[nodiscard]] T length() const noexcept { return std::sqrt(this->length_squared()); }

  /**
   * @brief Returns the unit vector in the same direction.
   * @details Returns the zero vector when the length is approximately zero.
   * @see try_normalized
   */
  [[nodiscard]] BasicVector4<T> normalized() const noexcept
  {
    return this->try_normalized().value_or(BasicVector4<T>());
  }

  /**
   * @brief Returns the unit vector in the same direction, or `std::nullopt` when the length is
   * approximately zero or not finite.
   * @see normalized
   */
  [[nodiscard]] std::optional<BasicVector4<T>> try_normalized() const noexcept
  {
    auto const len = this->length();
    if(is_null(len) or not common::is_finite(len))
      return std::nullopt;

    auto result = *this;
    result /= len;
    return result;
  }

  /**
   * @brief Returns the component-wise product.
   */
  [[nodiscard]] constexpr BasicVector4<T> component_mul(BasicVector4<T> const& other) const noexcept
  {
    return {
      this->m_x * other.m_x,
      this->m_y * other.m_y,
      this->m_z * other.m_z,
      this->m_w * other.m_w,
    };
  }

  /**
   * @brief Returns the component-wise minimum.
   */
  [[nodiscard]] constexpr BasicVector4<T> min(BasicVector4<T> const& other) const noexcept
  {
    return {
      std::min(this->m_x, other.m_x),
      std::min(this->m_y, other.m_y),
      std::min(this->m_z, other.m_z),
      std::min(this->m_w, other.m_w),
    };
  }

  /**
   * @brief Returns the component-wise maximum.
   */
  [[nodiscard]] constexpr BasicVector4<T> max(BasicVector4<T> const& other) const noexcept
  {
    return {
      std::max(this->m_x, other.m_x),
      std::max(this->m_y, other.m_y),
      std::max(this->m_z, other.m_z),
      std::max(this->m_w, other.m_w),
    };
  }

  /**
   * @brief Returns `*this + (other - *this) * t`; @p t is not clamped.
   */
  [[nodiscard]] constexpr BasicVector4<T> lerp(
    BasicVector4<T> const& other,
    T const t
  ) const noexcept
  {
    return {
      this->m_x + (other.m_x - this->m_x) * t,
      this->m_y + (other.m_y - this->m_y) * t,
      this->m_z + (other.m_z - this->m_z) * t,
      this->m_w + (other.m_w - this->m_w) * t,
    };
  }

  /**
   * @brief Returns the components as `std::array {x, y, z, w}`.
   */
  [[nodiscard]] constexpr std::array<T, 4> to_array() const noexcept
  {
    return {this->m_x, this->m_y, this->m_z, this->m_w};
  }

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  [[nodiscard]] static constexpr BasicVector4<T> zero() noexcept { return {}; }

  [[nodiscard]] static constexpr BasicVector4<T> splat(T const value) noexcept
  {
    return {value, value, value, value};
  }

  [[nodiscard]] static constexpr BasicVector4<T> unit_x() noexcept { return {1.0, 0.0, 0.0, 0.0}; }

  [[nodiscard]] static constexpr BasicVector4<T> unit_y() noexcept { return {0.0, 1.0, 0.0, 0.0}; }

  [[nodiscard]] static constexpr BasicVector4<T> unit_z() noexcept { return {0.0, 0.0, 1.0, 0.0}; }

  [[nodiscard]] static constexpr BasicVector4<T> unit_w() noexcept { return {0.0, 0.0, 0.0, 1.0}; }

  [[nodiscard]] static constexpr BasicVector4<T> from_xyzw(
    T const x,
    T const y,
    T const z,
    T const w
  ) noexcept
  {
    return {x, y, z, w};
  }

  /**
   * @brief Returns the homogeneous point `(position, 1)`, which a Matrix4x4 translates.
   */
  [[nodiscard]] static constexpr BasicVector4<T> from_point(BasicVector3<T> const position) noexcept
  {
    return {position, 1.0};
  }

  /**
   * @brief Returns the homogeneous direction `(direction, 0)`, which a Matrix4x4 does not
   * translate.
   */
  [[nodiscard]] static constexpr BasicVector4<T> from_direction(
    BasicVector3<T> const direction
  ) noexcept
  {
    return {direction, 0.0};
  }

  [[nodiscard]] constexpr bool operator==(BasicVector4<T> const&) const noexcept = default;

  [[nodiscard]] constexpr std::partial_ordering operator<=>(BasicVector4<T> const&) const noexcept =
    default;

  [[nodiscard]] constexpr BasicVector4<T> operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr BasicVector4<T> operator-() const noexcept
  {
    return {-this->m_x, -this->m_y, -this->m_z, -this->m_w};
  }

  constexpr BasicVector4<T>& operator+=(BasicVector4<T> const rhs) noexcept
  {
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    this->m_z += rhs.m_z;
    this->m_w += rhs.m_w;
    return *this;
  }

  constexpr BasicVector4<T>& operator-=(BasicVector4<T> const rhs) noexcept
  {
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    this->m_z -= rhs.m_z;
    this->m_w -= rhs.m_w;
    return *this;
  }

  constexpr BasicVector4<T>& operator*=(T const rhs) noexcept
  {
    this->m_x *= rhs;
    this->m_y *= rhs;
    this->m_z *= rhs;
    this->m_w *= rhs;
    return *this;
  }

  constexpr BasicVector4<T>& operator/=(T const rhs) noexcept
  {
    this->m_x /= rhs;
    this->m_y /= rhs;
    this->m_z /= rhs;
    this->m_w /= rhs;
    return *this;
  }

  [[nodiscard]] friend constexpr BasicVector4<T> operator+(
    BasicVector4<T> lhs,
    BasicVector4<T> const rhs
  ) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector4<T> operator-(
    BasicVector4<T> lhs,
    BasicVector4<T> const rhs
  ) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector4<T> operator*(
    BasicVector4<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs *= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicVector4<T> operator*(
    T const lhs,
    BasicVector4<T> const rhs
  ) noexcept
  {
    return rhs * lhs;
  }

  [[nodiscard]] friend constexpr BasicVector4<T> operator/(
    BasicVector4<T> lhs,
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
  T m_w = 0.0;
};

/**
 * @brief Vector4 in single precision, the default.
 */
using Vector4 = BasicVector4<float>;

/**
 * @brief Returns the dot product `a · b`.
 * @relates Vector4
 */
template <std::floating_point T>
[[nodiscard]] constexpr T dot(BasicVector4<T> const& a, BasicVector4<T> const& b) noexcept
{
  return a.dot(b);
}

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicVector4<T> const& vector)
{
  return os << vector.to_string();
}

static_assert(std::is_standard_layout_v<Vector4>);
static_assert(std::is_trivially_copyable_v<Vector4>);
static_assert(sizeof(Vector4) == 16);
static_assert(
  std::bit_cast<std::array<float, 4>>(Vector4(1.0, 2.0, 3.0, 4.0))
    == std::array<float, 4> {1, 2, 3, 4},
  "Vector4 must store x, y, z, w contiguously"
);

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicVector4<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicVector4<T> const vector) const noexcept
  {
    return w::common::_hash_impl::combine(vector.x(), vector.y(), vector.z(), vector.w());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(x, y, z, w)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicVector4<T>> : fmt::formatter<T>
{
  format_context::iterator format(
    w::common::BasicVector4<T> const vector,
    format_context& ctx
  ) const
  {
    return w::common::_format_impl::format_components<4, T>(*this, vector.to_array(), ctx);
  }
};
#endif
