#pragma once

#include <array>
#include <bit>
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
#include <w/common/vector4.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Plane of the points `p` with `normal · p + distance == 0`: the HLSL and glm `(a, b, c, d)`
 * plane.
 * @details The normal points into the positive half-space, "in front of" the plane, where
 * signed_distance() is positive. signed_distance() is a true distance only for a unit normal;
 * normalized() makes it one. The default value has a zero normal, which no point is on.
 *
 * @par Layout
 * The storage is exactly four contiguous Real values in `a, b, c, d` order, so it bit-casts to an
 * HLSL `float4`. The class is standard layout and trivially copyable. The layout is part of the
 * API and is checked by `static_assert`s in this header.
 * @ingroup core_math
 * @see Frustum
 * @see Ray
 */
class W_COMMON_API Plane
{
 public:
  constexpr Plane() noexcept = default;

  constexpr Plane(Vector3 const normal, Real const distance) noexcept
    : m_normal(normal)
    , m_distance(distance)
  {}

  [[nodiscard]] constexpr Vector3 normal() const noexcept { return this->m_normal; }

  /**
   * @brief Returns `d`: the signed distance of the origin from the plane, for a unit normal.
   */
  [[nodiscard]] constexpr Real distance() const noexcept { return this->m_distance; }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return this->m_normal.is_finite() and common::is_finite(this->m_distance);
  }

  /**
   * @brief Returns `normal · point + distance`: positive in front of the plane, negative behind
   * it, and in units of length for a unit normal.
   */
  [[nodiscard]] constexpr Real signed_distance(Vector3 const point) const noexcept
  {
    return this->m_normal.dot(point) + this->m_distance;
  }

  /**
   * @brief Returns the same plane with a unit normal.
   * @details A plane with a zero normal stays as it is: a plane of no points, entirely in front
   * or behind by the sign of its distance, as perspective() makes of an infinite far plane.
   */
  [[nodiscard]] Plane normalized() const noexcept
  {
    auto const length = this->m_normal.length();
    if(is_null(length))
      return *this;
    return {this->m_normal / length, this->m_distance / length};
  }

  /**
   * @brief Returns the same plane facing the other way.
   */
  [[nodiscard]] constexpr Plane flipped() const noexcept
  {
    return {-this->m_normal, -this->m_distance};
  }

  /**
   * @brief Returns the point of the plane closest to @p point.
   * @pre The normal is a unit vector.
   */
  [[nodiscard]] constexpr Vector3 closest_point(Vector3 const point) const noexcept
  {
    return point - this->m_normal * this->signed_distance(point);
  }

  /**
   * @brief Returns `(a, b, c, d)`.
   */
  [[nodiscard]] constexpr Vector4 to_vector4() const noexcept
  {
    return {this->m_normal, this->m_distance};
  }

  [[nodiscard]] std::string to_string() const;

  /**
   * @brief Returns the plane `(a, b, c, d)`.
   */
  [[nodiscard]] static constexpr Plane from_vector4(Vector4 const abcd) noexcept
  {
    return {abcd.xyz(), abcd.w()};
  }

  /**
   * @brief Returns the plane through @p point with the normal @p normal, as given.
   */
  [[nodiscard]] static constexpr Plane from_point_normal(
    Vector3 const point,
    Vector3 const normal
  ) noexcept
  {
    return {normal, -normal.dot(point)};
  }

  /**
   * @brief Returns the plane through the triangle @p a, @p b, @p c with a unit normal
   * `(b − a) × (c − a)`: in front is where the triangle winds counterclockwise in a right-handed
   * frame.
   * @details Returns `std::nullopt` if the points are collinear or not finite.
   */
  [[nodiscard]] static std::optional<Plane> from_points(
    Vector3 const a,
    Vector3 const b,
    Vector3 const c
  ) noexcept
  {
    auto const normal = (b - a).cross(c - a).try_normalized();
    if(not normal.has_value())
      return std::nullopt;
    return from_point_normal(a, *normal);
  }

  [[nodiscard]] constexpr bool operator==(Plane const&) const noexcept = default;

 private:
  Vector3 m_normal;
  Real m_distance = 0.0;
};

W_COMMON_API std::ostream& operator<<(std::ostream& os, Plane const& plane);

static_assert(std::is_standard_layout_v<Plane>);
static_assert(std::is_trivially_copyable_v<Plane>);
static_assert(sizeof(Plane) == 16);
static_assert(
  std::bit_cast<std::array<Real, 4>>(Plane({1.0, 2.0, 3.0}, 4.0))
    == std::array<Real, 4> {1, 2, 3, 4},
  "Plane must store a, b, c, d contiguously"
);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Plane>
{
  [[nodiscard]] std::size_t operator()(w::common::Plane const& plane) const noexcept
  {
    return w::common::_hash_impl::combine(plane.normal(), plane.distance());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(a, b, c, d)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <>
struct fmt::formatter<w::common::Plane> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Plane const& plane, format_context& ctx) const
  {
    return w::common::_format_impl::format_components<4>(*this, plane.to_vector4().to_array(), ctx);
  }
};
#endif
