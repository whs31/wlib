#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/detail/export.h>
#include <w/common/detail/format.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/matrix4x4.h>
#include <w/common/precision.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Ball of the points within @p radius of @p center, e.g. a bounding sphere for culling.
 * @details The default value is the point at the origin, with a zero radius.
 *
 * @par Layout
 * The storage is a Vector3 center and a Real radius, four contiguous Real values, so it bit-casts
 * to an HLSL `float4`. The class is standard layout and trivially copyable.
 * @ingroup core_math
 * @see AABB
 * @see Frustum
 */
class W_COMMON_API Sphere
{
 public:
  constexpr Sphere() noexcept = default;

  constexpr Sphere(Vector3 const center, Real const radius) noexcept
    : m_center(center)
    , m_radius(radius)
  {}

  [[nodiscard]] constexpr Vector3 center() const noexcept { return this->m_center; }

  [[nodiscard]] constexpr Real radius() const noexcept { return this->m_radius; }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return this->m_center.is_finite() and common::is_finite(this->m_radius);
  }

  /**
   * @brief Checks whether @p point is inside or on the sphere.
   */
  [[nodiscard]] constexpr bool contains(Vector3 const point) const noexcept
  {
    return (point - this->m_center).length_squared() <= this->m_radius * this->m_radius;
  }

  /**
   * @brief Checks whether the two spheres overlap or touch.
   */
  [[nodiscard]] constexpr bool intersects(Sphere const& other) const noexcept
  {
    auto const reach = this->m_radius + other.m_radius;
    return (other.m_center - this->m_center).length_squared() <= reach * reach;
  }

  /**
   * @brief Returns a sphere that contains this one after the affine transform @p matrix: the
   * center transformed, the radius scaled by the largest scale of the matrix.
   */
  [[nodiscard]] Sphere transformed(Matrix4x4 const& matrix) const noexcept
  {
    auto const linear = matrix.linear();
    auto const scale = std::sqrt(
      std::max({
        linear.column(0).length_squared(),
        linear.column(1).length_squared(),
        linear.column(2).length_squared(),
      })
    );
    return {matrix.transform_point(this->m_center), this->m_radius * scale};
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] constexpr bool operator==(Sphere const&) const noexcept = default;

 private:
  Vector3 m_center;
  Real m_radius = 0.0;
};

W_COMMON_API std::ostream& operator<<(std::ostream& os, Sphere const& sphere);

static_assert(std::is_standard_layout_v<Sphere>);
static_assert(std::is_trivially_copyable_v<Sphere>);
static_assert(sizeof(Sphere) == 16);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Sphere>
{
  [[nodiscard]] std::size_t operator()(w::common::Sphere const& sphere) const noexcept
  {
    return w::common::_hash_impl::combine(sphere.center(), sphere.radius());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `{center (x, y, z), radius r}`; the format spec, e.g. `{:.3f}`, applies to
 * every component.
 */
template <>
struct fmt::formatter<w::common::Sphere> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Sphere const& sphere, format_context& ctx) const
  {
    auto const center = sphere.center();
    auto out = fmt::format_to(ctx.out(), "{{center ");
    ctx.advance_to(out);
    out = w::common::_format_impl::format_components<
      3>(*this, {center.x(), center.y(), center.z()}, ctx);
    out = fmt::format_to(out, ", radius ");
    ctx.advance_to(out);
    out = fmt::formatter<w::common::Real>::format(sphere.radius(), ctx);
    *out++ = '}';
    return out;
  }
};
#endif
