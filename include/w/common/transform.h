#pragma once

#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/detail/export.h>
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
 * scale, ten contiguous Real values, with no padding. The class is standard layout and trivially
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
class W_COMMON_API Transform
{
 public:
  /**
   * @brief Creates the identity transform.
   */
  constexpr Transform() noexcept = default;

  explicit constexpr Transform(
    Vector3 const position,
    Quaternion const rotation = {},
    Vector3 const scale = Vector3::splat(1.0)
  ) noexcept
    : m_position(position)
    , m_rotation(rotation)
    , m_scale(scale)
  {}

  [[nodiscard]] constexpr Vector3 position() const noexcept { return this->m_position; }

  [[nodiscard]] constexpr Quaternion rotation() const noexcept { return this->m_rotation; }

  [[nodiscard]] constexpr Vector3 scale() const noexcept { return this->m_scale; }

  constexpr void set_position(Vector3 const position) noexcept { this->m_position = position; }

  constexpr void set_rotation(Quaternion const& rotation) noexcept { this->m_rotation = rotation; }

  constexpr void set_scale(Vector3 const scale) noexcept { this->m_scale = scale; }

  [[nodiscard]] constexpr Transform with_position(Vector3 const position) const noexcept
  {
    return Transform(position, this->m_rotation, this->m_scale);
  }

  [[nodiscard]] constexpr Transform with_rotation(Quaternion const& rotation) const noexcept
  {
    return Transform(this->m_position, rotation, this->m_scale);
  }

  [[nodiscard]] constexpr Transform with_scale(Vector3 const scale) const noexcept
  {
    return Transform(this->m_position, this->m_rotation, scale);
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
  [[nodiscard]] constexpr Matrix4x4 to_matrix() const noexcept
  {
    auto const rotation = this->m_rotation.to_rotation_matrix();
    auto const linear = Matrix3x3::from_columns(
      rotation.column(0) * this->m_scale.x(),
      rotation.column(1) * this->m_scale.y(),
      rotation.column(2) * this->m_scale.z()
    );
    return Matrix4x4::from_affine(linear, this->m_position);
  }

  /**
   * @brief Transforms the position @p point: scale, rotation, then translation.
   */
  [[nodiscard]] constexpr Vector3 transform_point(Vector3 const point) const noexcept
  {
    return this->m_position + this->transform_direction(point);
  }

  /**
   * @brief Transforms the direction @p direction: scale and rotation, no translation.
   */
  [[nodiscard]] constexpr Vector3 transform_direction(Vector3 const direction) const noexcept
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
  [[nodiscard]] constexpr Transform compose(Transform const& child) const noexcept
  {
    return Transform(
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
  [[nodiscard]] constexpr Transform inverse() const noexcept
  {
    auto const rotation = this->m_rotation.conjugate();
    auto const scale = Vector3::splat(1.0).component_div(this->m_scale);
    return Transform(-scale.component_mul(rotation.rotate(this->m_position)), rotation, scale);
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] static constexpr Transform identity() noexcept { return {}; }

  [[nodiscard]] static constexpr Transform from_position(Vector3 const position) noexcept
  {
    return Transform(position);
  }

  [[nodiscard]] static constexpr Transform from_rotation(Quaternion const& rotation) noexcept
  {
    return Transform(Vector3::zero(), rotation);
  }

  [[nodiscard]] static constexpr Transform from_scale(Vector3 const scale) noexcept
  {
    return Transform(Vector3::zero(), Quaternion::identity(), scale);
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
  [[nodiscard]] static std::optional<Transform> decompose(Matrix4x4 const& matrix) noexcept
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
    auto const sign = mirrored ? Real {-1} : Real {1};
    return Transform(
      matrix.translation(),
      Quaternion::from_rotation_matrix(Matrix3x3::from_columns(unit_x * sign, unit_y, unit_z)),
      {scale_x * sign, scale_y, scale_z}
    );
  }

  [[nodiscard]] constexpr bool operator==(Transform const&) const noexcept = default;

 private:
  Vector3 m_position;
  Quaternion m_rotation;
  Vector3 m_scale = Vector3::splat(1.0);
};

W_COMMON_API std::ostream& operator<<(std::ostream& os, Transform const& transform);

static_assert(std::is_standard_layout_v<Transform>);
static_assert(std::is_trivially_copyable_v<Transform>);
static_assert(sizeof(Transform) == 10 * sizeof(Real));

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Transform>
{
  [[nodiscard]] std::size_t operator()(w::common::Transform const& transform) const noexcept
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
template <>
struct fmt::formatter<w::common::Transform> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Transform const& transform, format_context& ctx) const
  {
    auto const position = transform.position();
    auto const rotation = transform.rotation();
    auto const scale = transform.scale();
    auto out = fmt::format_to(ctx.out(), "{{position ");
    ctx.advance_to(out);
    out = w::common::_format_impl::format_components<
      3>(*this, {position.x(), position.y(), position.z()}, ctx);
    out = fmt::format_to(out, ", rotation ");
    ctx.advance_to(out);
    out = w::common::_format_impl::format_components<
      4>(*this, {rotation.w(), rotation.x(), rotation.y(), rotation.z()}, ctx);
    out = fmt::format_to(out, ", scale ");
    ctx.advance_to(out);
    out = w::common::_format_impl::format_components<
      3>(*this, {scale.x(), scale.y(), scale.z()}, ctx);
    *out++ = '}';
    return out;
  }
};
#endif
