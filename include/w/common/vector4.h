#pragma once

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

#include <w/common/detail/export.h>
#include <w/common/detail/format.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Four-dimensional vector backed by Real values, mostly a homogeneous point or direction.
 * @details A position is `(x, y, z, 1)` and a direction is `(x, y, z, 0)`, so a Matrix4x4
 * translates the first and not the second; from_point() and from_direction() spell the choice out.
 *
 * @par Layout
 * The storage is exactly four contiguous Real values in `x, y, z, w` order: 16 bytes, with no
 * padding, so it bit-casts to `std::array<Real, 4>` and to an HLSL `float4`. The class is standard
 * layout and trivially copyable. The layout is part of the API and is checked by `static_assert`s
 * in this header.
 *
 * @ingroup core_math
 * @see Vector3
 * @see Matrix4x4
 */
class W_COMMON_API Vector4
{
 public:
  constexpr Vector4() noexcept = default;

  constexpr Vector4(Real const x, Real const y, Real const z, Real const w) noexcept
    : m_x(x)
    , m_y(y)
    , m_z(z)
    , m_w(w)
  {}

  constexpr Vector4(Vector3 const xyz, Real const w) noexcept
    : m_x(xyz.x())
    , m_y(xyz.y())
    , m_z(xyz.z())
    , m_w(w)
  {}

  [[nodiscard]] constexpr Real x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr Real y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr Real z() const noexcept { return this->m_z; }

  [[nodiscard]] constexpr Real w() const noexcept { return this->m_w; }

  /**
   * @brief Returns `(x, y, z)`, dropping `w` without dividing by it.
   * @see perspective_divided
   */
  [[nodiscard]] constexpr Vector3 xyz() const noexcept { return {this->m_x, this->m_y, this->m_z}; }

  /**
   * @brief Returns `(x, y, z) / w`, the Cartesian point of a homogeneous point.
   * @details A `w` of zero gives infinite or NaN components.
   */
  [[nodiscard]] constexpr Vector3 perspective_divided() const noexcept
  {
    return this->xyz() / this->m_w;
  }

  constexpr void set_x(Real const x) noexcept { this->m_x = x; }

  constexpr void set_y(Real const y) noexcept { this->m_y = y; }

  constexpr void set_z(Real const z) noexcept { this->m_z = z; }

  constexpr void set_w(Real const w) noexcept { this->m_w = w; }

  [[nodiscard]] constexpr Vector4 with_x(Real const x) const noexcept
  {
    return {x, this->m_y, this->m_z, this->m_w};
  }

  [[nodiscard]] constexpr Vector4 with_y(Real const y) const noexcept
  {
    return {this->m_x, y, this->m_z, this->m_w};
  }

  [[nodiscard]] constexpr Vector4 with_z(Real const z) const noexcept
  {
    return {this->m_x, this->m_y, z, this->m_w};
  }

  [[nodiscard]] constexpr Vector4 with_w(Real const w) const noexcept
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

  [[nodiscard]] constexpr Real dot(Vector4 const& other) const noexcept
  {
    return this->m_x * other.m_x + this->m_y * other.m_y + this->m_z * other.m_z
         + this->m_w * other.m_w;
  }

  [[nodiscard]] constexpr Real length_squared() const noexcept { return this->dot(*this); }

  [[nodiscard]] Real length() const noexcept { return std::sqrt(this->length_squared()); }

  /**
   * @brief Returns the unit vector in the same direction.
   * @details Returns the zero vector when the length is approximately zero.
   * @see try_normalized
   */
  [[nodiscard]] Vector4 normalized() const noexcept
  {
    return this->try_normalized().value_or(Vector4());
  }

  /**
   * @brief Returns the unit vector in the same direction, or `std::nullopt` when the length is
   * approximately zero or not finite.
   * @see normalized
   */
  [[nodiscard]] std::optional<Vector4> try_normalized() const noexcept
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
  [[nodiscard]] constexpr Vector4 component_mul(Vector4 const& other) const noexcept
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
  [[nodiscard]] constexpr Vector4 min(Vector4 const& other) const noexcept
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
  [[nodiscard]] constexpr Vector4 max(Vector4 const& other) const noexcept
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
  [[nodiscard]] constexpr Vector4 lerp(Vector4 const& other, Real const t) const noexcept
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
  [[nodiscard]] constexpr std::array<Real, 4> to_array() const noexcept
  {
    return {this->m_x, this->m_y, this->m_z, this->m_w};
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] static constexpr Vector4 zero() noexcept { return {}; }

  [[nodiscard]] static constexpr Vector4 splat(Real const value) noexcept
  {
    return {value, value, value, value};
  }

  [[nodiscard]] static constexpr Vector4 unit_x() noexcept { return {1.0, 0.0, 0.0, 0.0}; }

  [[nodiscard]] static constexpr Vector4 unit_y() noexcept { return {0.0, 1.0, 0.0, 0.0}; }

  [[nodiscard]] static constexpr Vector4 unit_z() noexcept { return {0.0, 0.0, 1.0, 0.0}; }

  [[nodiscard]] static constexpr Vector4 unit_w() noexcept { return {0.0, 0.0, 0.0, 1.0}; }

  [[nodiscard]] static constexpr Vector4 from_xyzw(
    Real const x,
    Real const y,
    Real const z,
    Real const w
  ) noexcept
  {
    return {x, y, z, w};
  }

  /**
   * @brief Returns the homogeneous point `(position, 1)`, which a Matrix4x4 translates.
   */
  [[nodiscard]] static constexpr Vector4 from_point(Vector3 const position) noexcept
  {
    return {position, 1.0};
  }

  /**
   * @brief Returns the homogeneous direction `(direction, 0)`, which a Matrix4x4 does not
   * translate.
   */
  [[nodiscard]] static constexpr Vector4 from_direction(Vector3 const direction) noexcept
  {
    return {direction, 0.0};
  }

  [[nodiscard]] constexpr bool operator==(Vector4 const&) const noexcept = default;

  [[nodiscard]] constexpr std::partial_ordering operator<=>(Vector4 const&) const noexcept =
    default;

  [[nodiscard]] constexpr Vector4 operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr Vector4 operator-() const noexcept
  {
    return {-this->m_x, -this->m_y, -this->m_z, -this->m_w};
  }

  constexpr Vector4& operator+=(Vector4 const rhs) noexcept
  {
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    this->m_z += rhs.m_z;
    this->m_w += rhs.m_w;
    return *this;
  }

  constexpr Vector4& operator-=(Vector4 const rhs) noexcept
  {
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    this->m_z -= rhs.m_z;
    this->m_w -= rhs.m_w;
    return *this;
  }

  constexpr Vector4& operator*=(Real const rhs) noexcept
  {
    this->m_x *= rhs;
    this->m_y *= rhs;
    this->m_z *= rhs;
    this->m_w *= rhs;
    return *this;
  }

  constexpr Vector4& operator/=(Real const rhs) noexcept
  {
    this->m_x /= rhs;
    this->m_y /= rhs;
    this->m_z /= rhs;
    this->m_w /= rhs;
    return *this;
  }

 private:
  Real m_x = 0.0;
  Real m_y = 0.0;
  Real m_z = 0.0;
  Real m_w = 0.0;
};

[[nodiscard]] constexpr Vector4 operator+(Vector4 lhs, Vector4 const rhs) noexcept
{
  lhs += rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector4 operator-(Vector4 lhs, Vector4 const rhs) noexcept
{
  lhs -= rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector4 operator*(Vector4 lhs, Real const rhs) noexcept
{
  lhs *= rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector4 operator*(Real const lhs, Vector4 const rhs) noexcept
{
  return rhs * lhs;
}

[[nodiscard]] constexpr Vector4 operator/(Vector4 lhs, Real const rhs) noexcept
{
  lhs /= rhs;
  return lhs;
}

/**
 * @brief Returns the dot product `a · b`.
 * @relates Vector4
 */
[[nodiscard]] constexpr Real dot(Vector4 const& a, Vector4 const& b) noexcept { return a.dot(b); }

W_COMMON_API std::ostream& operator<<(std::ostream& os, Vector4 vector);

static_assert(std::is_standard_layout_v<Vector4>);
static_assert(std::is_trivially_copyable_v<Vector4>);
static_assert(sizeof(Vector4) == 16);
static_assert(
  std::bit_cast<std::array<Real, 4>>(Vector4(1.0, 2.0, 3.0, 4.0))
    == std::array<Real, 4> {1, 2, 3, 4},
  "Vector4 must store x, y, z, w contiguously"
);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Vector4>
{
  [[nodiscard]] std::size_t operator()(w::common::Vector4 const vector) const noexcept
  {
    return w::common::_hash_impl::combine(vector.x(), vector.y(), vector.z(), vector.w());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(x, y, z, w)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <>
struct fmt::formatter<w::common::Vector4> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Vector4 const vector, format_context& ctx) const
  {
    return w::common::_format_impl::format_components<4>(*this, vector.to_array(), ctx);
  }
};
#endif
