#pragma once

#include <concepts>
#include <ostream>
#include <limits>
#include <cstddef>
#include <array>
#include <functional>
#include <iosfwd>
#include <span>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/detail/format.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/matrix4x4.h>
#include <w/common/precision.h>
#include <w/common/sphere.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Axis-aligned bounding box from the corner min() to the corner max(), boundary included.
 * @details The default value is empty(): `min` at +∞ and `max` at −∞, which contains nothing and
 * which merged() grows from, so a box accumulates its points from the default value.
 *
 * @par Layout
 * The storage is the Vector3 min corner and the Vector3 max corner, six contiguous @p T values.
 * The class is standard layout and trivially copyable.
 * @ingroup core_math
 * @see Sphere
 * @see Frustum
 * @see Ray
 */
template <std::floating_point T>
class BasicAABB
{
 public:
  /**
   * @brief Creates the empty box.
   */
  constexpr BasicAABB() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicAABB(BasicAABB<U> const& other) noexcept
    : BasicAABB(BasicVector3<T>(other.min()), BasicVector3<T>(other.max()))
  {}

  constexpr BasicAABB(BasicVector3<T> const min, BasicVector3<T> const max) noexcept
    : m_min(min)
    , m_max(max)
  {}

  [[nodiscard]] constexpr BasicVector3<T> min() const noexcept { return this->m_min; }

  [[nodiscard]] constexpr BasicVector3<T> max() const noexcept { return this->m_max; }

  [[nodiscard]] constexpr BasicVector3<T> center() const noexcept
  {
    return (this->m_min + this->m_max) / 2;
  }

  /**
   * @brief Returns the half size: the distance from the center to the faces along each axis.
   */
  [[nodiscard]] constexpr BasicVector3<T> extents() const noexcept
  {
    return (this->m_max - this->m_min) / 2;
  }

  [[nodiscard]] constexpr BasicVector3<T> size() const noexcept
  {
    return this->m_max - this->m_min;
  }

  /**
   * @brief Checks whether the box holds no point: some component of min() exceeds that of max().
   */
  [[nodiscard]] constexpr bool is_empty() const noexcept
  {
    return this->m_min.x() > this->m_max.x() or this->m_min.y() > this->m_max.y()
        or this->m_min.z() > this->m_max.z();
  }

  /**
   * @brief Checks whether both corners are neither infinite nor NaN; false for the empty box.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return this->m_min.is_finite() and this->m_max.is_finite();
  }

  /**
   * @brief Checks whether @p point is inside the box or on its boundary.
   */
  [[nodiscard]] constexpr bool contains(BasicVector3<T> const point) const noexcept
  {
    return point.x() >= this->m_min.x() and point.x() <= this->m_max.x()
       and point.y() >= this->m_min.y() and point.y() <= this->m_max.y()
       and point.z() >= this->m_min.z() and point.z() <= this->m_max.z();
  }

  /**
   * @brief Checks whether @p other is a non-empty box entirely inside this one.
   */
  [[nodiscard]] constexpr bool contains(BasicAABB<T> const& other) const noexcept
  {
    return not other.is_empty() and this->contains(other.m_min) and this->contains(other.m_max);
  }

  /**
   * @brief Checks whether the boxes overlap or touch.
   */
  [[nodiscard]] constexpr bool intersects(BasicAABB<T> const& other) const noexcept
  {
    return this->m_min.x() <= other.m_max.x() and this->m_max.x() >= other.m_min.x()
       and this->m_min.y() <= other.m_max.y() and this->m_max.y() >= other.m_min.y()
       and this->m_min.z() <= other.m_max.z() and this->m_max.z() >= other.m_min.z();
  }

  /**
   * @brief Checks whether the box and @p sphere overlap or touch.
   */
  [[nodiscard]] constexpr bool intersects(BasicSphere<T> const& sphere) const noexcept
  {
    return not this->is_empty()
       and (this->closest_point(sphere.center()) - sphere.center()).length_squared()
             <= sphere.radius() * sphere.radius();
  }

  /**
   * @brief Returns the point of the box closest to @p point: @p point itself if it is inside.
   * @pre The box is not empty.
   */
  [[nodiscard]] constexpr BasicVector3<T> closest_point(BasicVector3<T> const point) const noexcept
  {
    return point.max(this->m_min).min(this->m_max);
  }

  /**
   * @brief Returns the smallest box that holds this one and @p point.
   */
  [[nodiscard]] constexpr BasicAABB<T> merged(BasicVector3<T> const point) const noexcept
  {
    return {this->m_min.min(point), this->m_max.max(point)};
  }

  /**
   * @brief Returns the smallest box that holds this one and @p other.
   */
  [[nodiscard]] constexpr BasicAABB<T> merged(BasicAABB<T> const& other) const noexcept
  {
    return {this->m_min.min(other.m_min), this->m_max.max(other.m_max)};
  }

  /**
   * @brief Returns the box grown by @p margin on every side; a negative margin shrinks it.
   */
  [[nodiscard]] constexpr BasicAABB<T> expanded(T const margin) const noexcept
  {
    return {
      this->m_min - BasicVector3<T>::splat(margin),
      this->m_max + BasicVector3<T>::splat(margin)
    };
  }

  /**
   * @brief Returns the eight corners, the bit `1 << axis` of the index choosing max() on that
   * axis.
   */
  [[nodiscard]] constexpr std::array<BasicVector3<T>, 8> corners() const noexcept
  {
    auto result = std::array<BasicVector3<T>, 8> {};
    for(auto index = std::size_t {0}; index < result.size(); ++index)
      result.at(index) = {
        (index & 1U) != 0 ? this->m_max.x() : this->m_min.x(),
        (index & 2U) != 0 ? this->m_max.y() : this->m_min.y(),
        (index & 4U) != 0 ? this->m_max.z() : this->m_min.z(),
      };
    return result;
  }

  /**
   * @brief Returns the box around this one after the affine transform @p matrix.
   * @details Arvo's method: exact for the transformed corners, so a rotated box grows. The empty
   * box stays empty.
   */
  [[nodiscard]] constexpr BasicAABB<T> transformed(BasicMatrix4x4<T> const& matrix) const noexcept
  {
    if(this->is_empty())
      return {};
    auto const center = matrix.transform_point(this->center());
    auto const extents = this->extents();
    auto const linear = matrix.linear();
    auto const reach = BasicVector3<T>(
      linear.row(0).abs().dot(extents),
      linear.row(1).abs().dot(extents),
      linear.row(2).abs().dot(extents)
    );
    return {center - reach, center + reach};
  }

  /**
   * @brief Returns the sphere through the corners, around the center.
   * @pre The box is not empty.
   */
  [[nodiscard]] BasicSphere<T> bounding_sphere() const noexcept
  {
    return {this->center(), this->extents().length()};
  }

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  /**
   * @brief Returns the box that contains nothing: min() at +∞ and max() at −∞.
   */
  [[nodiscard]] static constexpr BasicAABB<T> empty() noexcept { return {}; }

  [[nodiscard]] static constexpr BasicAABB<T> from_center_extents(
    BasicVector3<T> const center,
    BasicVector3<T> const extents
  ) noexcept
  {
    return {center - extents, center + extents};
  }

  /**
   * @brief Returns the smallest box that holds every point of @p points, or the empty box for no
   * points.
   */
  [[nodiscard]] static constexpr BasicAABB<T> from_points(
    std::span<BasicVector3<T> const> const points
  ) noexcept
  {
    auto result = BasicAABB<T>();
    for(auto const point : points)
      result = result.merged(point);
    return result;
  }

  [[nodiscard]] constexpr bool operator==(BasicAABB<T> const&) const noexcept = default;

 private:
  BasicVector3<T> m_min = BasicVector3<T>::splat(std::numeric_limits<T>::infinity());
  BasicVector3<T> m_max = BasicVector3<T>::splat(-std::numeric_limits<T>::infinity());
};

/**
 * @brief AABB in single precision, the default.
 */
using AABB = BasicAABB<float>;

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicAABB<T> const& box)
{
  return os << box.to_string();
}

static_assert(std::is_standard_layout_v<AABB>);
static_assert(std::is_trivially_copyable_v<AABB>);
static_assert(sizeof(AABB) == 6 * sizeof(float));

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicAABB<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicAABB<T> const& box) const noexcept
  {
    return w::common::_hash_impl::combine(box.min(), box.max());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `[(min x, min y, min z), (max x, max y, max z)]`; the format spec, e.g.
 * `{:.3f}`, applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicAABB<T>> : fmt::formatter<T>
{
  format_context::iterator format(w::common::BasicAABB<T> const& box, format_context& ctx) const
  {
    auto const min = box.min();
    auto const max = box.max();
    auto out = ctx.out();
    *out++ = '[';
    ctx.advance_to(out);
    out = w::common::_format_impl::format_components<3, T>(*this, {min.x(), min.y(), min.z()}, ctx);
    *out++ = ',';
    *out++ = ' ';
    ctx.advance_to(out);
    out = w::common::_format_impl::format_components<3, T>(*this, {max.x(), max.y(), max.z()}, ctx);
    *out++ = ']';
    return out;
  }
};
#endif
