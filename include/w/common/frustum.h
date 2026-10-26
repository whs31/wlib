#pragma once

#include <concepts>
#include <ostream>
#include <cstdint>
#include <algorithm>
#include <array>
#include <functional>
#include <iosfwd>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/aabb.h>
#include <w/common/clip_space.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/matrix4x4.h>
#include <w/common/plane.h>
#include <w/common/precision.h>
#include <w/common/sphere.h>
#include <w/common/vector3.h>
#include <w/common/vector4.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief How a shape lies relative to a Frustum.
 * @ingroup core_math
 */
enum class Containment : std::uint8_t
{
  Outside,
  Intersecting,
  Inside,
};

/**
 * @brief The six planes that bound what a camera sees, normals pointing inward, for culling.
 * @details from_view_projection() extracts them from `projection * view` (Gribb–Hartmann): a
 * point is inside where its clip coordinates satisfy `−w ≤ x ≤ w`, `−w ≤ y ≤ w` and
 * `0 ≤ z ≤ w`, the `[0, 1]` depth range of perspective() and orthographic(). The planes are
 * normalized, so signed distances are lengths. An infinite far plane has a zero normal and a
 * positive distance, so everything is in front of it.
 *
 * The tests are conservative: a box or sphere near a corner of the frustum, outside it but not
 * entirely behind any single plane, counts as intersecting.
 *
 * @par Layout
 * The storage is six contiguous Plane values in the order left, right, bottom, top, near, far,
 * i.e. 24 @p T values, so it bit-casts to an HLSL `float4[6]`.
 * @ingroup core_math
 * @see Plane
 * @see perspective
 */
template <std::floating_point T>
class BasicFrustum
{
 public:
  /**
   * @brief Creates the frustum of six zero planes, which contains everything.
   */
  constexpr BasicFrustum() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicFrustum(
    BasicFrustum<U> const& other
  ) noexcept
  {
    std::ranges::transform(other.planes(), this->m_planes.begin(), [](BasicPlane<U> const& plane) {
      return BasicPlane<T>(plane);
    });
  }

  /**
   * @brief Creates the frustum of @p planes in the order left, right, bottom, top, near, far,
   * normals pointing inward.
   */
  explicit constexpr BasicFrustum(std::array<BasicPlane<T>, 6> const& planes) noexcept
    : m_planes(planes)
  {}

  [[nodiscard]] constexpr std::array<BasicPlane<T>, 6> const& planes() const noexcept
  {
    return this->m_planes;
  }

  [[nodiscard]] constexpr BasicPlane<T> left_plane() const noexcept { return this->m_planes[0]; }

  [[nodiscard]] constexpr BasicPlane<T> right_plane() const noexcept { return this->m_planes[1]; }

  [[nodiscard]] constexpr BasicPlane<T> bottom_plane() const noexcept { return this->m_planes[2]; }

  [[nodiscard]] constexpr BasicPlane<T> top_plane() const noexcept { return this->m_planes[3]; }

  [[nodiscard]] constexpr BasicPlane<T> near_plane() const noexcept { return this->m_planes[4]; }

  [[nodiscard]] constexpr BasicPlane<T> far_plane() const noexcept { return this->m_planes[5]; }

  /**
   * @brief Checks whether @p point is inside the frustum or on its boundary.
   */
  [[nodiscard]] constexpr bool contains(BasicVector3<T> const point) const noexcept
  {
    return std::ranges::all_of(this->m_planes, [point](BasicPlane<T> const& plane) {
      return plane.signed_distance(point) >= 0;
    });
  }

  /**
   * @brief Returns whether @p sphere is outside, partly inside or entirely inside the frustum.
   */
  [[nodiscard]] constexpr Containment classify(BasicSphere<T> const& sphere) const noexcept
  {
    auto result = Containment::Inside;
    for(auto const& plane : this->m_planes) {
      auto const distance = plane.signed_distance(sphere.center());
      if(distance < -sphere.radius())
        return Containment::Outside;
      if(distance < sphere.radius())
        result = Containment::Intersecting;
    }
    return result;
  }

  /**
   * @brief Returns whether @p box is outside, partly inside or entirely inside the frustum.
   * @details Tests the corner of the box farthest along each inward normal, and the nearest one.
   * The empty box is outside.
   */
  [[nodiscard]] constexpr Containment classify(BasicAABB<T> const& box) const noexcept
  {
    if(box.is_empty())
      return Containment::Outside;
    auto result = Containment::Inside;
    auto const min = box.min();
    auto const max = box.max();
    for(auto const& plane : this->m_planes) {
      auto const n = plane.normal();
      auto const farthest = BasicVector3<T>(
        n.x() >= 0 ? max.x() : min.x(),
        n.y() >= 0 ? max.y() : min.y(),
        n.z() >= 0 ? max.z() : min.z()
      );
      if(plane.signed_distance(farthest) < 0)
        return Containment::Outside;
      auto const nearest = BasicVector3<T>(
        n.x() >= 0 ? min.x() : max.x(),
        n.y() >= 0 ? min.y() : max.y(),
        n.z() >= 0 ? min.z() : max.z()
      );
      if(plane.signed_distance(nearest) < 0)
        result = Containment::Intersecting;
    }
    return result;
  }

  /**
   * @brief Checks whether @p sphere may be visible: classify() is not Containment::Outside.
   */
  [[nodiscard]] constexpr bool intersects(BasicSphere<T> const& sphere) const noexcept
  {
    return this->classify(sphere) != Containment::Outside;
  }

  /**
   * @brief Checks whether @p box may be visible: classify() is not Containment::Outside.
   */
  [[nodiscard]] constexpr bool intersects(BasicAABB<T> const& box) const noexcept
  {
    return this->classify(box) != Containment::Outside;
  }

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  /**
   * @brief Returns the frustum of the matrix @p view_projection, `projection * view`, whose
   * planes are then in world space.
   * @details Pass the ClipDepth of the projection, so near_plane() and far_plane() name the right
   * planes; the volume is the same either way.
   */
  [[nodiscard]] static BasicFrustum<T> from_view_projection(
    BasicMatrix4x4<T> const& view_projection,
    ClipDepth const depth = ClipDepth::Standard
  ) noexcept
  {
    auto const x = view_projection.row(0);
    auto const y = view_projection.row(1);
    auto const z = view_projection.row(2);
    auto const w = view_projection.row(3);
    auto const plane = [](BasicVector4<T> const abcd) {
      return BasicPlane<T>::from_vector4(abcd).normalized();
    };
    auto const depth_0 = plane(z);
    auto const depth_1 = plane(w - z);
    return BasicFrustum<T>({
      plane(w + x),
      plane(w - x),
      plane(w + y),
      plane(w - y),
      depth == ClipDepth::Standard ? depth_0 : depth_1,
      depth == ClipDepth::Standard ? depth_1 : depth_0,
    });
  }

  [[nodiscard]] constexpr bool operator==(BasicFrustum<T> const&) const noexcept = default;

 private:
  std::array<BasicPlane<T>, 6> m_planes {};
};

/**
 * @brief Frustum in single precision, the default.
 */
using Frustum = BasicFrustum<float>;

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicFrustum<T> const& frustum)
{
  return os << frustum.to_string();
}

static_assert(std::is_standard_layout_v<Frustum>);
static_assert(std::is_trivially_copyable_v<Frustum>);
static_assert(sizeof(Frustum) == 6 * sizeof(Plane));

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicFrustum<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicFrustum<T> const& frustum) const noexcept
  {
    auto const& [l, r, b, t, n, f] = frustum.planes();
    return w::common::_hash_impl::combine(l, r, b, t, n, f);
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `[left, right, bottom, top, near, far]`, each plane as `(a, b, c, d)`; the
 * format spec, e.g. `{:.3f}`, applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicFrustum<T>> : fmt::formatter<w::common::BasicPlane<T>>
{
  format_context::iterator format(
    w::common::BasicFrustum<T> const& frustum,
    format_context& ctx
  ) const
  {
    auto out = ctx.out();
    *out++ = '[';
    auto first = true;
    for(auto const& plane : frustum.planes()) {
      if(not first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
      ctx.advance_to(out);
      out = fmt::formatter<w::common::BasicPlane<T>>::format(plane, ctx);
    }
    *out++ = ']';
    return out;
  }
};
#endif
