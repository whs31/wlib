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

#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/detail/export.h>
#include <w/common/detail/format.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/math.h>

#ifdef W_COMMON_WITH_QT
#  include <qmetatype.h>
#  include <qmetaobject.h>
#  include <qpoint.h>
#  include <qstring.h>
#  include <qvector2d.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Two-dimensional vector backed by Real values: a position, an offset or a direction.
 * @details The vector does not know its frame or unit; name the variable after them instead,
 * e.g. `cursor_px` or `offset_world`.
 *
 * @par Layout
 * The storage is exactly two contiguous Real values in `x, y` order, with no padding. The class is
 * standard layout and trivially copyable, so it can be reflected by offset or bit-cast to
 * `std::array<Real, 2>`. `Q_GADGET` adds no data members and does not affect the layout. The
 * layout is part of the API and is checked by `static_assert`s in this header.
 *
 * @note Qt properties, conversions, and metatype registration are available
 * only when wlib is built with `with_qt=True`.
 *
 * @ingroup core_measurements
 * @see Vector3
 * @see Angle
 */
class W_COMMON_API Vector2
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double x READ x CONSTANT FINAL)
  Q_PROPERTY(double y READ y CONSTANT FINAL)
  Q_PROPERTY(double length READ length CONSTANT FINAL)
  Q_PROPERTY(double lengthSquared READ length_squared CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  constexpr Vector2() noexcept = default;

  constexpr Vector2(Real const x, Real const y) noexcept
    : m_x(x)
    , m_y(y)
  {}

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  explicit Vector2(QPointF point) noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  explicit Vector2(QVector2D const& vector) noexcept;
#endif

  [[nodiscard]] constexpr Real x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr Real y() const noexcept { return this->m_y; }

  constexpr void set_x(Real const x) noexcept { this->m_x = x; }

  constexpr void set_y(Real const y) noexcept { this->m_y = y; }

  /**
   * @brief Returns a copy with the x component replaced.
   */
  [[nodiscard]] constexpr Vector2 with_x(Real const x) const noexcept { return {x, this->m_y}; }

  /**
   * @brief Returns a copy with the y component replaced.
   */
  [[nodiscard]] constexpr Vector2 with_y(Real const y) const noexcept { return {this->m_x, y}; }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_x) and common::is_finite(this->m_y);
  }

  [[nodiscard]] constexpr Real length_squared() const noexcept
  {
    return this->m_x * this->m_x + this->m_y * this->m_y;
  }

  [[nodiscard]] Real length() const noexcept { return std::sqrt(this->length_squared()); }

  [[nodiscard]] Angle angle() const noexcept
  {
    return Angle::from_radians(std::atan2(this->m_y, this->m_x));
  }

  [[nodiscard]] constexpr Real dot(Vector2 const& other) const noexcept
  {
    return this->m_x * other.m_x + this->m_y * other.m_y;
  }

  [[nodiscard]] constexpr Real cross(Vector2 const& other) const noexcept
  {
    return this->m_x * other.m_y - this->m_y * other.m_x;
  }

  /**
   * @brief Returns the shortest signed angle from the direction of this vector to the direction of
   * @p other.
   * @see angle_towards
   */
  [[nodiscard]] Angle angle_to(Vector2 const& other) const noexcept
  {
    return this->angle().angle_to(other.angle());
  }

  /**
   * @brief Returns the direction from this position to the position @p target: the angle of
   * `target - *this`.
   * @see angle_to
   */
  [[nodiscard]] Angle angle_towards(Vector2 const& target) const noexcept
  {
    return Vector2(target.m_x - this->m_x, target.m_y - this->m_y).angle();
  }

  /**
   * @brief Returns the distance from this position to the position @p other.
   */
  [[nodiscard]] Real distance_to(Vector2 const& other) const noexcept
  {
    return Vector2(other.m_x - this->m_x, other.m_y - this->m_y).length();
  }

  /**
   * @brief Returns the component-wise product.
   */
  [[nodiscard]] constexpr Vector2 component_mul(Vector2 const& other) const noexcept
  {
    return {this->m_x * other.m_x, this->m_y * other.m_y};
  }

  /**
   * @brief Returns the component-wise quotient.
   */
  [[nodiscard]] constexpr Vector2 component_div(Vector2 const& other) const noexcept
  {
    return {this->m_x / other.m_x, this->m_y / other.m_y};
  }

  /**
   * @brief Returns the component-wise minimum.
   */
  [[nodiscard]] constexpr Vector2 min(Vector2 const& other) const noexcept
  {
    return {std::min(this->m_x, other.m_x), std::min(this->m_y, other.m_y)};
  }

  /**
   * @brief Returns the component-wise maximum.
   */
  [[nodiscard]] constexpr Vector2 max(Vector2 const& other) const noexcept
  {
    return {std::max(this->m_x, other.m_x), std::max(this->m_y, other.m_y)};
  }

  /**
   * @brief Returns the component-wise absolute value.
   */
  [[nodiscard]] constexpr Vector2 abs() const noexcept
  {
    return {common::abs(this->m_x), common::abs(this->m_y)};
  }

  /**
   * @brief Returns `*this + (other - *this) * t`; @p t is not clamped.
   */
  [[nodiscard]] constexpr Vector2 lerp(Vector2 const& other, Real const t) const noexcept
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
  [[nodiscard]] Vector2 normalized() const noexcept
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
  [[nodiscard]] std::optional<Vector2> try_normalized() const noexcept
  {
    auto const len = this->length();
    if(is_null(len) or not common::is_finite(len))
      return std::nullopt;

    auto result = *this;
    result /= len;
    return result;
  }

  [[nodiscard]] Vector2 rotated(Angle const angle) const noexcept
  {
    auto const c = angle.cos();
    auto const s = angle.sin();
    return {this->m_x * c - this->m_y * s, this->m_x * s + this->m_y * c};
  }

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QPointF to_qpointf() const noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QVector2D to_qvector2d() const noexcept;
#endif

  [[nodiscard]] std::string to_string() const;

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QString to_qstring() const;
#endif

  [[nodiscard]] static constexpr Vector2 zero() noexcept { return {}; }

  /**
   * @brief Returns the unit vector `(1, 0)`.
   */
  [[nodiscard]] static constexpr Vector2 unit_x() noexcept { return {1.0, 0.0}; }

  /**
   * @brief Returns the unit vector `(0, 1)`.
   */
  [[nodiscard]] static constexpr Vector2 unit_y() noexcept { return {0.0, 1.0}; }

  [[nodiscard]] static constexpr Vector2 splat(Real const value) noexcept { return {value, value}; }

  [[nodiscard]] static constexpr Vector2 from_xy(Real const x, Real const y) noexcept
  {
    return {x, y};
  }

  [[nodiscard]] static Vector2 from_angle(Angle const angle) noexcept
  {
    return {angle.cos(), angle.sin()};
  }

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] static Vector2 from_qpointf(QPointF point) noexcept;
#endif

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] static Vector2 from_qvector2d(QVector2D const& vector) noexcept;
#endif

  [[nodiscard]] constexpr bool operator==(Vector2 const&) const noexcept = default;

  [[nodiscard]] constexpr std::partial_ordering operator<=>(Vector2 const&) const noexcept =
    default;

  [[nodiscard]] constexpr Vector2 operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr Vector2 operator-() const noexcept { return {-this->m_x, -this->m_y}; }

  constexpr Vector2& operator+=(Vector2 const rhs) noexcept
  {
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    return *this;
  }

  constexpr Vector2& operator-=(Vector2 const rhs) noexcept
  {
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    return *this;
  }

  constexpr Vector2& operator*=(Real const rhs) noexcept
  {
    this->m_x *= rhs;
    this->m_y *= rhs;
    return *this;
  }

  constexpr Vector2& operator/=(Real const rhs) noexcept
  {
    this->m_x /= rhs;
    this->m_y /= rhs;
    return *this;
  }

 private:
  Real m_x = 0.0;
  Real m_y = 0.0;
};

[[nodiscard]] constexpr Vector2 operator+(Vector2 lhs, Vector2 const rhs) noexcept
{
  lhs += rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector2 operator-(Vector2 lhs, Vector2 const rhs) noexcept
{
  lhs -= rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector2 operator*(Vector2 lhs, Real const rhs) noexcept
{
  lhs *= rhs;
  return lhs;
}

[[nodiscard]] constexpr Vector2 operator*(Real const lhs, Vector2 const rhs) noexcept
{
  return rhs * lhs;
}

[[nodiscard]] constexpr Vector2 operator/(Vector2 lhs, Real const rhs) noexcept
{
  lhs /= rhs;
  return lhs;
}

/**
 * @brief Returns the dot product `a · b`.
 * @relates Vector2
 */
[[nodiscard]] constexpr Real dot(Vector2 const& a, Vector2 const& b) noexcept { return a.dot(b); }

/**
 * @brief Returns the z component of the cross product `a × b` of the vectors lifted to 3D.
 * @relates Vector2
 */
[[nodiscard]] constexpr Real cross(Vector2 const& a, Vector2 const& b) noexcept
{
  return a.cross(b);
}

W_COMMON_API std::ostream& operator<<(std::ostream& os, Vector2 vector);

static_assert(std::is_standard_layout_v<Vector2>);
static_assert(std::is_trivially_copyable_v<Vector2>);
static_assert(sizeof(Vector2) == 8);
static_assert(
  std::bit_cast<std::array<Real, 2>>(Vector2(1.0, 2.0)) == std::array<Real, 2> {1, 2},
  "Vector2 must store x, y contiguously"
);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Vector2>
{
  [[nodiscard]] std::size_t operator()(w::common::Vector2 const vector) const noexcept
  {
    auto hash = std::hash<w::common::Real> {}(vector.x());
    hash ^= std::hash<w::common::Real> {}(vector.y()) + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    return hash;
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(x, y)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <>
struct fmt::formatter<w::common::Vector2> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Vector2 const vector, format_context& ctx) const
  {
    return w::common::_format_impl::format_components<2>(*this, {vector.x(), vector.y()}, ctx);
  }
};
#endif

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Vector2)
#endif
