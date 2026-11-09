#pragma once

#include <concepts>
#include <ostream>
#include <algorithm>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/stdint.h>
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
 * The storage is a Vector3 center and a @p T radius, four contiguous @p T values, so it bit-casts
 * to an HLSL `float4`. The class is standard layout and trivially copyable.
 * @ingroup core_math
 * @see AABB
 * @see Frustum
 */
template <std::floating_point T>
class BasicSphere
{
 public:
  constexpr BasicSphere() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicSphere(BasicSphere<U> const& other) noexcept
    : BasicSphere(BasicVector3<T>(other.center()), static_cast<T>(other.radius()))
  {}

  constexpr BasicSphere(BasicVector3<T> const center, T const radius) noexcept
    : m_center(center)
    , m_radius(radius)
  {}

  [[nodiscard]] constexpr BasicVector3<T> center() const noexcept { return this->m_center; }

  [[nodiscard]] constexpr T radius() const noexcept { return this->m_radius; }

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
  [[nodiscard]] constexpr bool contains(BasicVector3<T> const point) const noexcept
  {
    return (point - this->m_center).length_squared() <= this->m_radius * this->m_radius;
  }

  /**
   * @brief Checks whether the two spheres overlap or touch.
   */
  [[nodiscard]] constexpr bool intersects(BasicSphere<T> const& other) const noexcept
  {
    auto const reach = this->m_radius + other.m_radius;
    return (other.m_center - this->m_center).length_squared() <= reach * reach;
  }

  /**
   * @brief Returns a sphere that contains this one after the affine transform @p matrix: the
   * center transformed, the radius scaled by the largest scale of the matrix.
   */
  [[nodiscard]] BasicSphere<T> transformed(BasicMatrix4x4<T> const& matrix) const noexcept
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

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  [[nodiscard]] constexpr bool operator==(BasicSphere<T> const&) const noexcept = default;

 private:
  BasicVector3<T> m_center;
  T m_radius = 0.0;
};

/**
 * @brief Sphere in single precision, the default.
 */
using Sphere = BasicSphere<float>;

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicSphere<T> const& sphere)
{
  return os << sphere.to_string();
}

static_assert(std::is_standard_layout_v<Sphere>);
static_assert(std::is_trivially_copyable_v<Sphere>);
static_assert(sizeof(Sphere) == 16);

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicSphere<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicSphere<T> const& sphere) const noexcept
  {
    return w::common::_hash_impl::combine(sphere.center(), sphere.radius());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `{center (x, y, z), radius r}`; the format spec, e.g. `{:.3f}`, applies to
 * every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicSphere<T>> : formatter<T>
{
  format_context::iterator format(
    w::common::BasicSphere<T> const& sphere,
    format_context& ctx
  ) const
  {
    auto const center = sphere.center();
    auto out = format_to(ctx.out(), "{{center ");
    ctx.advance_to(out);
    out = w::common::_format_impl::
      format_components<3, T>(*this, {center.x(), center.y(), center.z()}, ctx);
    out = format_to(out, ", radius ");
    ctx.advance_to(out);
    out = formatter<T>::format(sphere.radius(), ctx);
    *out++ = '}';
    return out;
  }
};
#endif
