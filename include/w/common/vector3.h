#pragma once

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

#include <w/common/config.h>
#include <w/common/detail/export.h>
#include <w/common/detail/format.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/math.h>
#include <w/common/vector2.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetatype.h>
#  include <qmetaobject.h>
#  include <qpoint.h>
#  include <qstring.h>
#  include <qvector2d.h>
#  include <qvector3d.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Three-dimensional vector backed by Real values: a position, an offset or a direction.
 * @details The vector does not know its frame or unit; name the variable after them instead,
 * e.g. `velocity_ned`, `omega_body` or `position_world`. Whether it is a position or a direction
 * shows in how it is transformed: Matrix4x4::transform_point() translates it,
 * Matrix4x4::transform_direction() does not.
 *
 * @par Layout
 * The storage is exactly three contiguous Real values in `x, y, z` order: 12 bytes, with no
 * padding and no SIMD alignment. The class is standard layout and trivially copyable, so it can be
 * reflected by offset or bit-cast to `std::array<Real, 3>`. `Q_GADGET` adds no data members and
 * does not affect the layout. The layout is part of the API and is checked by `static_assert`s in
 * this header. A hot loop that needs SIMD gets batch functions over `std::span<Vector3>`, never
 * padding.
 *
 * @note Qt properties, conversions, and metatype registration are available
 * only when wlib is built with `with_qt=True`.
 * @ingroup core_measurements
 * @see Vector2
 * @see Vector4
 * @see Matrix3x3
 */
class W_COMMON_API Vector3
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double x READ x CONSTANT FINAL)
  Q_PROPERTY(double y READ y CONSTANT FINAL)
  Q_PROPERTY(double z READ z CONSTANT FINAL)
  Q_PROPERTY(double length READ length CONSTANT FINAL)
  Q_PROPERTY(double lengthSquared READ length_squared CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  constexpr Vector3() noexcept = default;

  constexpr Vector3(Real const x, Real const y, Real const z) noexcept
    : m_x(x)
    , m_y(y)
    , m_z(z)
  {}

  // The default z intentionally preserves the implicit Vector2-to-Vector3 conversion.
  // NOLINTNEXTLINE(google-explicit-constructor)
  constexpr Vector3(Vector2 const vector, Real const z = 0.0) noexcept
    : m_x(vector.x())
    , m_y(vector.y())
    , m_z(z)
  {}

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  explicit Vector3(QPointF point) noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  explicit Vector3(QVector2D const& vector) noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  explicit Vector3(QVector3D const& vector) noexcept;
#endif

  [[nodiscard]] constexpr Real x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr Real y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr Real z() const noexcept { return this->m_z; }

  [[nodiscard]] constexpr Vector2 xy() const noexcept { return {this->m_x, this->m_y}; }

  constexpr void set_x(Real const x) noexcept { this->m_x = x; }

  constexpr void set_y(Real const y) noexcept { this->m_y = y; }

  constexpr void set_z(Real const z) noexcept { this->m_z = z; }

  /**
   * @brief Returns a copy with the x component replaced.
   */
  [[nodiscard]] constexpr Vector3 with_x(Real const x) const noexcept
  {
    return {x, this->m_y, this->m_z};
  }

  /**
   * @brief Returns a copy with the y component replaced.
   */
  [[nodiscard]] constexpr Vector3 with_y(Real const y) const noexcept
  {
    return {this->m_x, y, this->m_z};
  }

  /**
   * @brief Returns a copy with the z component replaced.
   */
  [[nodiscard]] constexpr Vector3 with_z(Real const z) const noexcept
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

  [[nodiscard]] constexpr Real length_squared() const noexcept
  {
    return this->m_x * this->m_x + this->m_y * this->m_y + this->m_z * this->m_z;
  }

  [[nodiscard]] Real length() const noexcept { return std::sqrt(this->length_squared()); }

  [[nodiscard]] constexpr Real dot(Vector3 const& other) const noexcept
  {
    return this->m_x * other.m_x + this->m_y * other.m_y + this->m_z * other.m_z;
  }

  [[nodiscard]] constexpr Vector3 cross(Vector3 const& other) const noexcept
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
  [[nodiscard]] Vector3 normalized() const noexcept
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
  [[nodiscard]] std::optional<Vector3> try_normalized() const noexcept
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
  [[nodiscard]] Real distance_to(Vector3 const& other) const noexcept
  {
    return Vector3(other.m_x - this->m_x, other.m_y - this->m_y, other.m_z - this->m_z).length();
  }

  /**
   * @brief Returns the component-wise product, e.g. a vector scaled per axis.
   */
  [[nodiscard]] constexpr Vector3 component_mul(Vector3 const& other) const noexcept
  {
    return {this->m_x * other.m_x, this->m_y * other.m_y, this->m_z * other.m_z};
  }

  /**
   * @brief Returns the component-wise quotient.
   */
  [[nodiscard]] constexpr Vector3 component_div(Vector3 const& other) const noexcept
  {
    return {this->m_x / other.m_x, this->m_y / other.m_y, this->m_z / other.m_z};
  }

  /**
   * @brief Returns the component-wise minimum.
   */
  [[nodiscard]] constexpr Vector3 min(Vector3 const& other) const noexcept
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
  [[nodiscard]] constexpr Vector3 max(Vector3 const& other) const noexcept
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
  [[nodiscard]] constexpr Vector3 abs() const noexcept
  {
    return {common::abs(this->m_x), common::abs(this->m_y), common::abs(this->m_z)};
  }

  /**
   * @brief Returns `*this + (other - *this) * t`; @p t is not clamped.
   */
  [[nodiscard]] constexpr Vector3 lerp(Vector3 const& other, Real const t) const noexcept
  {
    return {
      this->m_x + (other.m_x - this->m_x) * t,
      this->m_y + (other.m_y - this->m_y) * t,
      this->m_z + (other.m_z - this->m_z) * t,
    };
  }

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QPointF to_qpointf() const noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QVector2D to_qvector2d() const noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QVector3D to_qvector3d() const noexcept;
#endif

  [[nodiscard]] std::string to_string() const;

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QString to_qstring() const;
#endif

  [[nodiscard]] static constexpr Vector3 zero() noexcept { return {}; }

  /**
   * @brief Returns the unit vector `(1, 0, 0)`.
   */
  [[nodiscard]] static constexpr Vector3 unit_x() noexcept { return {1.0, 0.0, 0.0}; }

  /**
   * @brief Returns the unit vector `(0, 1, 0)`.
   */
  [[nodiscard]] static constexpr Vector3 unit_y() noexcept { return {0.0, 1.0, 0.0}; }

  /**
   * @brief Returns the unit vector `(0, 0, 1)`.
   */
  [[nodiscard]] static constexpr Vector3 unit_z() noexcept { return {0.0, 0.0, 1.0}; }

  [[nodiscard]] static constexpr Vector3 splat(Real const value) noexcept
  {
    return {value, value, value};
  }

  [[nodiscard]] static constexpr Vector3 from_xyz(Real const x, Real const y, Real const z) noexcept
  {
    return {x, y, z};
  }

  [[nodiscard]] static constexpr Vector3 from_vector2(
    Vector2 const vector,
    Real const z = 0.0
  ) noexcept
  {
    return {vector, z};
  }

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] static Vector3 from_qpointf(QPointF point) noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] static Vector3 from_qvector2d(QVector2D const& vector) noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] static Vector3 from_qvector3d(QVector3D const& vector) noexcept;
#endif

  [[nodiscard]] constexpr bool operator==(Vector3 const&) const noexcept = default;

  [[nodiscard]] constexpr std::partial_ordering operator<=>(Vector3 const&) const noexcept =
    default;

  [[nodiscard]] constexpr Vector3 operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr Vector3 operator-() const noexcept
  {
    return {-this->m_x, -this->m_y, -this->m_z};
  }

  constexpr Vector3& operator+=(Vector3 const rhs) noexcept
  {
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    this->m_z += rhs.m_z;
    return *this;
  }

  constexpr Vector3& operator-=(Vector3 const rhs) noexcept
  {
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    this->m_z -= rhs.m_z;
    return *this;
  }

  constexpr Vector3& operator*=(Real const rhs) noexcept
  {
    this->m_x *= rhs;
    this->m_y *= rhs;
    this->m_z *= rhs;
    return *this;
  }

  constexpr Vector3& operator/=(Real const rhs) noexcept
  {
    this->m_x /= rhs;
    this->m_y /= rhs;
    this->m_z /= rhs;
    return *this;
  }

 private:
  Real m_x = 0.0;
  Real m_y = 0.0;
  Real m_z = 0.0;
};

[[nodiscard]] constexpr Vector3 operator+(Vector3 lhs, Vector3 const rhs) noexcept
{
  lhs += rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector3 operator-(Vector3 lhs, Vector3 const rhs) noexcept
{
  lhs -= rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector3 operator*(Vector3 lhs, Real const rhs) noexcept
{
  lhs *= rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector3 operator*(Real const lhs, Vector3 const rhs) noexcept
{
  return rhs * lhs;
}

[[nodiscard]] constexpr Vector3 operator/(Vector3 lhs, Real const rhs) noexcept
{
  lhs /= rhs;
  return lhs;
}

/**
 * @brief Returns the dot product `a · b`.
 * @relates Vector3
 */
[[nodiscard]] constexpr Real dot(Vector3 const& a, Vector3 const& b) noexcept { return a.dot(b); }

/**
 * @brief Returns the right-handed cross product `a × b`.
 * @relates Vector3
 */
[[nodiscard]] constexpr Vector3 cross(Vector3 const& a, Vector3 const& b) noexcept
{
  return a.cross(b);
}

W_COMMON_API std::ostream& operator<<(std::ostream& os, Vector3 vector);

static_assert(std::is_standard_layout_v<Vector3>);
static_assert(std::is_trivially_copyable_v<Vector3>);
static_assert(sizeof(Vector3) == 12, "Vector3 is three floats without SIMD padding");
static_assert(
  std::bit_cast<std::array<Real, 3>>(Vector3(1.0, 2.0, 3.0)) == std::array<Real, 3> {1, 2, 3},
  "Vector3 must store x, y, z contiguously"
);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Vector3>
{
  [[nodiscard]] std::size_t operator()(w::common::Vector3 const vector) const noexcept
  {
    auto hash = std::hash<w::common::Real> {}(vector.x());
    hash ^= std::hash<w::common::Real> {}(vector.y()) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    hash ^= std::hash<w::common::Real> {}(vector.z()) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash;
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(x, y, z)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <>
struct fmt::formatter<w::common::Vector3> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Vector3 const vector, format_context& ctx) const
  {
    return w::common::_format_impl::format_components<
      3>(*this, {vector.x(), vector.y(), vector.z()}, ctx);
  }
};
#endif

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Vector3)
#endif
