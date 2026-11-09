#pragma once

#include <concepts>
#include <ostream>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/stdint.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/matrix3x3.h>
#include <w/common/matrix4x4.h>
#include <w/common/precision.h>
#include <w/common/quaternion.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Position, rotation and scale of an object relative to its parent: the TRS transform
 * `p ↦ position + rotation ⊗ (scale ⊙ p)`.
 * @details The default value is the identity. The scale applies first, along the object's own
 * axes, then the rotation, then the translation, so to_matrix() is `T * R * S`.
 *
 * A TRS triple cannot hold every affine transform: rotating a non-uniformly scaled child makes
 * shear. compose() and inverse() are exact while the scale they invert or propagate is uniform,
 * which is the common case for scene nodes; for the exact general result, multiply and invert the
 * matrices and decompose() the product.
 *
 * @par Layout
 * The storage is a Vector3 position, a Quaternion rotation in `w, x, y, z` order and a Vector3
 * scale, ten contiguous @p T values, with no padding. The class is standard layout and trivially
 * copyable. The layout is part of the API and is checked by `static_assert`s in this header.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto const parent = Transform({0.0, 1.0, 0.0}, Quaternion::from_axis_angle(Vector3::unit_y(),
 * 90_deg)); auto const child = Transform::from_position({1.0, 0.0, 0.0}); auto const world =
 * parent.compose(child);           // child in world space auto const gizmo =
 * Transform::decompose(world.to_matrix());
 * @endcode
 *
 * @ingroup core_math
 * @see Matrix4x4
 * @see Quaternion
 */
template <std::floating_point T>
class BasicTransform
{
 public:
  /**
   * @brief Creates the identity transform.
   */
  constexpr BasicTransform() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicTransform(
    BasicTransform<U> const& other
  ) noexcept
    : BasicTransform(
        BasicVector3<T>(other.position()),
        BasicQuaternion<T>(other.rotation()),
        BasicVector3<T>(other.scale())
      )
  {}

  explicit constexpr BasicTransform(
    BasicVector3<T> const position,
    BasicQuaternion<T> const rotation = {},
    BasicVector3<T> const scale = BasicVector3<T>::splat(1.0)
  ) noexcept
    : m_position(position)
    , m_rotation(rotation)
    , m_scale(scale)
  {}

  [[nodiscard]] constexpr BasicVector3<T> position() const noexcept { return this->m_position; }

  [[nodiscard]] constexpr BasicQuaternion<T> rotation() const noexcept { return this->m_rotation; }

  [[nodiscard]] constexpr BasicVector3<T> scale() const noexcept { return this->m_scale; }

  constexpr void set_position(BasicVector3<T> const position) noexcept
  {
    this->m_position = position;
  }

  constexpr void set_rotation(BasicQuaternion<T> const& rotation) noexcept
  {
    this->m_rotation = rotation;
  }

  constexpr void set_scale(BasicVector3<T> const scale) noexcept { this->m_scale = scale; }

  [[nodiscard]] constexpr BasicTransform<T> with_position(
    BasicVector3<T> const position
  ) const noexcept
  {
    return BasicTransform<T>(position, this->m_rotation, this->m_scale);
  }

  [[nodiscard]] constexpr BasicTransform<T> with_rotation(
    BasicQuaternion<T> const& rotation
  ) const noexcept
  {
    return BasicTransform<T>(this->m_position, rotation, this->m_scale);
  }

  [[nodiscard]] constexpr BasicTransform<T> with_scale(BasicVector3<T> const scale) const noexcept
  {
    return BasicTransform<T>(this->m_position, this->m_rotation, scale);
  }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return this->m_position.is_finite() and this->m_rotation.is_finite()
       and this->m_scale.is_finite();
  }

  /**
   * @brief Returns the matrix `T * R * S` of this transform.
   * @pre The rotation is a unit quaternion.
   */
  [[nodiscard]] constexpr BasicMatrix4x4<T> to_matrix() const noexcept
  {
    auto const rotation = this->m_rotation.to_rotation_matrix();
    auto const linear = BasicMatrix3x3<T>::from_columns(
      rotation.column(0) * this->m_scale.x(),
      rotation.column(1) * this->m_scale.y(),
      rotation.column(2) * this->m_scale.z()
    );
    return BasicMatrix4x4<T>::from_affine(linear, this->m_position);
  }

  /**
   * @brief Transforms the position @p point: scale, rotation, then translation.
   */
  [[nodiscard]] constexpr BasicVector3<T> transform_point(
    BasicVector3<T> const point
  ) const noexcept
  {
    return this->m_position + this->transform_direction(point);
  }

  /**
   * @brief Transforms the direction @p direction: scale and rotation, no translation.
   */
  [[nodiscard]] constexpr BasicVector3<T> transform_direction(
    BasicVector3<T> const direction
  ) const noexcept
  {
    return this->m_rotation.rotate(this->m_scale.component_mul(direction));
  }

  /**
   * @brief Returns the transform that applies @p child first and this transform after it: the
   * world transform of a child whose local transform is @p child.
   * @details `parent.compose(child).transform_point(p) == parent.transform_point(child
   * .transform_point(p))` holds exactly when the scale of this transform is uniform or the
   * rotation of @p child is the identity; otherwise the shear the product would need is dropped.
   */
  [[nodiscard]] constexpr BasicTransform<T> compose(BasicTransform<T> const& child) const noexcept
  {
    return BasicTransform<T>(
      this->transform_point(child.m_position),
      this->m_rotation * child.m_rotation,
      this->m_scale.component_mul(child.m_scale)
    );
  }

  /**
   * @brief Returns the transform that undoes this one.
   * @details Exact for a uniform scale; with a non-uniform scale the inverse is not a TRS
   * transform and this drops its shear, so invert to_matrix() instead.
   * @pre The rotation is a unit quaternion and no scale factor is zero.
   */
  [[nodiscard]] constexpr BasicTransform<T> inverse() const noexcept
  {
    auto const rotation = this->m_rotation.conjugate();
    auto const scale = BasicVector3<T>::splat(1.0).component_div(this->m_scale);
    return BasicTransform<
      T>(-scale.component_mul(rotation.rotate(this->m_position)), rotation, scale);
  }

  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  [[nodiscard]] static constexpr BasicTransform<T> identity() noexcept { return {}; }

  [[nodiscard]] static constexpr BasicTransform<T> from_position(
    BasicVector3<T> const position
  ) noexcept
  {
    return BasicTransform<T>(position);
  }

  [[nodiscard]] static constexpr BasicTransform<T> from_rotation(
    BasicQuaternion<T> const& rotation
  ) noexcept
  {
    return BasicTransform<T>(BasicVector3<T>::zero(), rotation);
  }

  [[nodiscard]] static constexpr BasicTransform<T> from_scale(BasicVector3<T> const scale) noexcept
  {
    return BasicTransform<T>(BasicVector3<T>::zero(), BasicQuaternion<T>::identity(), scale);
  }

  /**
   * @brief Splits the affine matrix @p matrix into position, rotation and scale, e.g. after a
   * gizmo has edited it.
   * @details The position is the last column. The scale factors are the lengths of the first three
   * columns after Gram–Schmidt orthogonalization in x, y, z order, so shear, which a TRS transform
   * cannot hold, is dropped. A mirroring matrix, with a negative determinant, gets a negative x
   * scale. Returns `std::nullopt` if @p matrix is not affine (its last row is not
   * `(0, 0, 0, 1)`), not finite, or scales an axis to zero.
   */
  [[nodiscard]] static std::optional<BasicTransform<T>> decompose(
    BasicMatrix4x4<T> const& matrix
  ) noexcept
  {
    if(not matrix.is_affine() or not matrix.is_finite())
      return std::nullopt;

    auto const linear = matrix.linear();
    auto const x = linear.column(0);
    auto const scale_x = x.length();
    if(is_null(scale_x))
      return std::nullopt;
    auto const unit_x = x / scale_x;

    auto const y = linear.column(1) - unit_x * unit_x.dot(linear.column(1));
    auto const scale_y = y.length();
    if(is_null(scale_y))
      return std::nullopt;
    auto const unit_y = y / scale_y;

    auto const z = linear.column(2) - unit_x * unit_x.dot(linear.column(2))
                 - unit_y * unit_y.dot(linear.column(2));
    auto const scale_z = z.length();
    if(is_null(scale_z))
      return std::nullopt;
    auto const unit_z = z / scale_z;

    // the orthonormal columns are a rotation unless the matrix mirrors, then flip x
    auto const mirrored = linear.determinant() < 0;
    auto const sign = mirrored ? T {-1} : T {1};
    return BasicTransform<T>(
      matrix.translation(),
      BasicQuaternion<T>::from_rotation_matrix(
        BasicMatrix3x3<T>::from_columns(unit_x * sign, unit_y, unit_z)
      ),
      {scale_x * sign, scale_y, scale_z}
    );
  }

  [[nodiscard]] constexpr bool operator==(BasicTransform<T> const&) const noexcept = default;

 private:
  BasicVector3<T> m_position;
  BasicQuaternion<T> m_rotation;
  BasicVector3<T> m_scale = BasicVector3<T>::splat(1.0);
};

/**
 * @brief Transform in single precision, the default.
 */
using Transform = BasicTransform<float>;

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicTransform<T> const& transform)
{
  return os << transform.to_string();
}

static_assert(std::is_standard_layout_v<Transform>);
static_assert(std::is_trivially_copyable_v<Transform>);
static_assert(sizeof(Transform) == 10 * sizeof(float));

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicTransform<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicTransform<T> const& transform) const noexcept
  {
    return w::common::_hash_impl::
      combine(transform.position(), transform.rotation(), transform.scale());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `{position (x, y, z), rotation (w, x, y, z), scale (x, y, z)}`; the format
 * spec, e.g. `{:.3f}`, applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicTransform<T>> : formatter<T>
{
  format_context::iterator format(
    w::common::BasicTransform<T> const& transform,
    format_context& ctx
  ) const
  {
    auto const position = transform.position();
    auto const rotation = transform.rotation();
    auto const scale = transform.scale();
    auto out = format_to(ctx.out(), "{{position ");
    ctx.advance_to(out);
    out = w::common::_format_impl::
      format_components<3, T>(*this, {position.x(), position.y(), position.z()}, ctx);
    out = format_to(out, ", rotation ");
    ctx.advance_to(out);
    out = w::common::_format_impl::
      format_components<4, T>(*this, {rotation.w(), rotation.x(), rotation.y(), rotation.z()}, ctx);
    out = format_to(out, ", scale ");
    ctx.advance_to(out);
    out = w::common::_format_impl::
      format_components<3, T>(*this, {scale.x(), scale.y(), scale.z()}, ctx);
    *out++ = '}';
    return out;
  }
};
#endif
