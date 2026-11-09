#pragma once

#include <concepts>
#include <ostream>
#include <limits>
#include <algorithm>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include <fmt/format.h>

#include <w/common/stdint.h>
#include <w/common/aabb.h>
#include <w/common/clip_space.h>
#include <w/common/detail/format.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/matrix4x4.h>
#include <w/common/plane.h>
#include <w/common/precision.h>
#include <w/common/sphere.h>
#include <w/common/vector2.h>
#include <w/common/vector3.h>
#include <w/common/vector4.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Where a ray meets a triangle: the ray parameter and the barycentric weights of the
 * second and third vertex.
 * @details The point is `ray.point_at(t) == a + u (b − a) + v (c − a)`, the same `u, v` as glm's
 * `intersectRayTriangle`.
 * @ingroup core_math
 * @see Ray::intersect_triangle
 */
template <std::floating_point T>
struct BasicTriangleHit
{
  T t = 0.0;
  T u = 0.0;
  T v = 0.0;

  [[nodiscard]] constexpr bool operator==(BasicTriangleHit<T> const&) const noexcept = default;
};

/**
 * @brief TriangleHit in single precision, the default.
 */
using TriangleHit = BasicTriangleHit<float>;

/**
 * @brief Half-line `origin + t * direction` for `t ≥ 0`, e.g. a picking ray under the cursor.
 * @details The direction need not be a unit vector; every `t` is in multiples of it, and is a
 * distance only for a unit direction, as from_ndc() makes.
 *
 * The intersections return the smallest `t ≥ 0` at which the ray meets the shape, or
 * `std::nullopt` if it misses or the shape is behind the origin. A solid shape, Sphere or AABB,
 * that contains the origin is met at `t = 0`.
 *
 * @par Layout
 * The storage is the Vector3 origin and the Vector3 direction, six contiguous @p T values. The
 * class is standard layout and trivially copyable.
 * @ingroup core_math
 * @see Plane
 * @see AABB
 * @see Sphere
 */
template <std::floating_point T>
class BasicRay
{
 public:
  constexpr BasicRay() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicRay(BasicRay<U> const& other) noexcept
    : BasicRay(BasicVector3<T>(other.origin()), BasicVector3<T>(other.direction()))
  {}

  constexpr BasicRay(BasicVector3<T> const origin, BasicVector3<T> const direction) noexcept
    : m_origin(origin)
    , m_direction(direction)
  {}

  [[nodiscard]] constexpr BasicVector3<T> origin() const noexcept { return this->m_origin; }

  [[nodiscard]] constexpr BasicVector3<T> direction() const noexcept { return this->m_direction; }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return this->m_origin.is_finite() and this->m_direction.is_finite();
  }

  /**
   * @brief Returns `origin + t * direction`.
   */
  [[nodiscard]] constexpr BasicVector3<T> point_at(T const t) const noexcept
  {
    return this->m_origin + this->m_direction * t;
  }

  /**
   * @brief Returns where the ray crosses @p plane, from either side.
   * @details A ray parallel to the plane misses it, even one lying in it.
   */
  [[nodiscard]] constexpr std::optional<T> intersect(BasicPlane<T> const& plane) const noexcept
  {
    auto const approach = plane.normal().dot(this->m_direction);
    if(approach == 0)
      return std::nullopt;
    auto const t = -plane.signed_distance(this->m_origin) / approach;
    if(not(t >= 0) or not common::is_finite(t))
      return std::nullopt;
    return t;
  }

  /**
   * @brief Returns where the ray enters @p sphere, or 0 if it starts inside.
   */
  [[nodiscard]] std::optional<T> intersect(BasicSphere<T> const& sphere) const noexcept
  {
    auto const offset = this->m_origin - sphere.center();
    auto const a = this->m_direction.length_squared();
    auto const half_b = offset.dot(this->m_direction);
    auto const c = offset.length_squared() - sphere.radius() * sphere.radius();
    if(c <= 0)
      return T {0};
    auto const discriminant = half_b * half_b - a * c;
    if(half_b > 0 or discriminant < 0 or a == 0)
      return std::nullopt;
    return (-half_b - std::sqrt(discriminant)) / a;
  }

  /**
   * @brief Returns where the ray enters @p box, or 0 if it starts inside: the slab method.
   * @details A ray along a face counts as meeting the box. The empty box is never met.
   */
  [[nodiscard]] constexpr std::optional<T> intersect(BasicAABB<T> const& box) const noexcept
  {
    auto enter = T {0};
    auto exit = std::numeric_limits<T>::infinity();
    auto const slab = [&enter, &exit](T const origin, T const direction, T const min, T const max) {
      if(direction == 0)
        return origin >= min and origin <= max;
      auto near_t = (min - origin) / direction;
      auto far_t = (max - origin) / direction;
      if(near_t > far_t)
        std::swap(near_t, far_t);
      enter = std::max(enter, near_t);
      exit = std::min(exit, far_t);
      return enter <= exit;
    };
    auto const o = this->m_origin;
    auto const d = this->m_direction;
    auto const min = box.min();
    auto const max = box.max();
    if(
      slab(o.x(), d.x(), min.x(), max.x()) and slab(o.y(), d.y(), min.y(), max.y())
      and slab(o.z(), d.z(), min.z(), max.z())
    )
      return enter;
    return std::nullopt;
  }

  /**
   * @brief Returns where the ray meets the triangle @p a, @p b, @p c from either side: the
   * Möller–Trumbore method.
   * @details Hits on an edge count. A ray in the plane of the triangle misses it.
   */
  [[nodiscard]] constexpr std::optional<BasicTriangleHit<T>> intersect_triangle(
    BasicVector3<T> const a,
    BasicVector3<T> const b,
    BasicVector3<T> const c
  ) const noexcept
  {
    auto const edge_b = b - a;
    auto const edge_c = c - a;
    auto const p = this->m_direction.cross(edge_c);
    auto const det = edge_b.dot(p);
    if(det == 0 or not common::is_finite(det))
      return std::nullopt;

    auto const inverse_det = 1 / det;
    auto const offset = this->m_origin - a;
    auto const u = offset.dot(p) * inverse_det;
    if(u < 0 or u > 1)
      return std::nullopt;
    auto const q = offset.cross(edge_b);
    auto const v = this->m_direction.dot(q) * inverse_det;
    if(v < 0 or u + v > 1)
      return std::nullopt;
    auto const t = edge_c.dot(q) * inverse_det;
    if(t < 0)
      return std::nullopt;
    return BasicTriangleHit<T> {.t = t, .u = u, .v = v};
  }

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  /**
   * @brief Returns the ray from @p origin through @p target, with a unit direction.
   * @details Coincident points give a zero direction, which meets only shapes containing
   * @p origin.
   */
  [[nodiscard]] static BasicRay<T> through(
    BasicVector3<T> const origin,
    BasicVector3<T> const target
  ) noexcept
  {
    return {origin, (target - origin).normalized()};
  }

  /**
   * @brief Returns the picking ray under the point @p ndc of the screen, in normalized device
   * coordinates `[-1, 1]` with y up, from the near plane away from the camera, with a unit
   * direction.
   * @details Unprojects two depths of @p ndc through @p inverse_view_projection, the inverse of
   * `projection * view`. Works for perspective() and orthographic(), with a finite or infinite
   * far plane, as long as @p depth is the ClipDepth of the projection.
   */
  [[nodiscard]] static BasicRay<T> from_ndc(
    BasicVector2<T> const ndc,
    BasicMatrix4x4<T> const& inverse_view_projection,
    ClipDepth const depth = ClipDepth::Standard
  ) noexcept
  {
    // the far plane may be at infinity, so the second point is halfway in depth rather than on it
    auto const near_depth = depth == ClipDepth::Standard ? T {0} : T {1};
    auto const unproject = [&](T const z) {
      return (inverse_view_projection * BasicVector4<T>(ndc.x(), ndc.y(), z, 1.0))
        .perspective_divided();
    };
    return through(unproject(near_depth), unproject(0.5));
  }

  [[nodiscard]] constexpr bool operator==(BasicRay<T> const&) const noexcept = default;

 private:
  BasicVector3<T> m_origin;
  BasicVector3<T> m_direction;
};

/**
 * @brief Ray in single precision, the default.
 */
using Ray = BasicRay<float>;

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicRay<T> const& ray)
{
  return os << ray.to_string();
}

static_assert(std::is_standard_layout_v<Ray>);
static_assert(std::is_trivially_copyable_v<Ray>);
static_assert(sizeof(Ray) == 6 * sizeof(float));

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicRay<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicRay<T> const& ray) const noexcept
  {
    return w::common::_hash_impl::combine(ray.origin(), ray.direction());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `{origin (x, y, z), direction (x, y, z)}`; the format spec, e.g. `{:.3f}`,
 * applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicRay<T>> : formatter<T>
{
  format_context::iterator format(w::common::BasicRay<T> const& ray, format_context& ctx) const
  {
    auto const origin = ray.origin();
    auto const direction = ray.direction();
    auto out = format_to(ctx.out(), "{{origin ");
    ctx.advance_to(out);
    out = w::common::_format_impl::
      format_components<3, T>(*this, {origin.x(), origin.y(), origin.z()}, ctx);
    out = format_to(out, ", direction ");
    ctx.advance_to(out);
    out = w::common::_format_impl::
      format_components<3, T>(*this, {direction.x(), direction.y(), direction.z()}, ctx);
    *out++ = '}';
    return out;
  }
};
#endif
