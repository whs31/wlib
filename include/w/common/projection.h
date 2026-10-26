#pragma once

#include <cmath>
#include <utility>

#include <w/common/angle.h>
#include <w/common/clip_space.h>
#include <w/common/detail/namespace.h>
#include <w/common/error.h>
#include <w/common/math.h>
#include <w/common/matrix3x3.h>
#include <w/common/matrix4x4.h>
#include <w/common/precision.h>
#include <w/common/quaternion.h>
#include <w/common/vector2.h>
#include <w/common/vector3.h>

/**
 * @file
 * @brief View and projection matrices with depth in `[0, 1]`, for Direct3D, Vulkan and Diligent.
 * @details Every function takes the Handedness of the view space and returns a Matrix4x4 for
 * column vectors, `clip = projection * view * Vector4::from_point(world)`. See @ref conventions.
 */

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Returns the perspective projection of the vertical field of view @p fov_y.
 * @details Maps the view frustum to `x, y ∈ [-1, 1]` and depth `z ∈ [0, 1]`, as @p depth
 * prescribes, after the division by `w`, which is the distance in front of the camera. @p z_far may
 * be infinite, which keeps every distant object, most usefully with ClipDepth::Reversed.
 *
 * Matches glm's `perspectiveRH_ZO` and `perspectiveLH_ZO`, and `infinitePerspectiveRH_ZO` and
 * `infinitePerspectiveLH_ZO` for an infinite @p z_far; reversing the depth is the same as swapping
 * @p z_near and @p z_far.
 * @param fov_y Vertical field of view, in `(0°, 180°)`.
 * @param aspect Width over height of the viewport, positive.
 * @param z_near Distance of the near plane, positive.
 * @param z_far Distance of the far plane, greater than @p z_near, or infinite.
 * @param handedness Whether the camera looks along −z or +z.
 * @param depth Whether the near plane maps to depth 0 or 1.
 * @ingroup core_math
 */
[[nodiscard]] inline Matrix4x4 perspective(
  Angle const fov_y,
  Real const aspect,
  Real const z_near,
  Real const z_far,
  Handedness const handedness,
  ClipDepth const depth = ClipDepth::Standard
) noexcept
{
  auto const focal = 1 / (fov_y / 2).tan();
  auto const [mapped_to_0, mapped_to_1] = depth == ClipDepth::Standard ? std::pair(z_near, z_far)
                                                                       : std::pair(z_far, z_near);

  // depth = (scale z + offset) / w in right-handed view space, where w = -z; the left-handed
  // projection flips the sign of z and w
  auto scale = Real {0};
  auto offset = Real {0};
  if(std::isinf(mapped_to_1)) {
    scale = -1;
    offset = -mapped_to_0;
  }
  else if(std::isinf(mapped_to_0)) {
    scale = 0;
    offset = mapped_to_1;
  }
  else {
    scale = mapped_to_1 / (mapped_to_0 - mapped_to_1);
    offset = mapped_to_0 * mapped_to_1 / (mapped_to_0 - mapped_to_1);
  }
  auto const forward = handedness == Handedness::Right ? Real {-1} : Real {1};
  return Matrix4x4::from_rows(
    {focal / aspect, 0.0, 0.0, 0.0},
    {0.0, focal, 0.0, 0.0},
    {0.0, 0.0, -forward * scale, offset},
    {0.0, 0.0, forward, 0.0}
  );
}

/**
 * @brief Returns the orthographic projection of the view-space box from @p left to @p right,
 * @p bottom to @p top, and @p z_near to @p z_far in front of the camera.
 * @details Maps the box to `x, y ∈ [-1, 1]` and depth `z ∈ [0, 1]` as @p depth prescribes. Matches
 * glm's `orthoRH_ZO` and `orthoLH_ZO`; reversing the depth is the same as swapping @p z_near and
 * @p z_far.
 * @pre `left != right`, `bottom != top`, and `z_near != z_far`, all finite.
 * @ingroup core_math
 */
[[nodiscard]] constexpr Matrix4x4 orthographic(
  Real const left,
  Real const right,
  Real const bottom,
  Real const top,
  Real const z_near,
  Real const z_far,
  Handedness const handedness,
  ClipDepth const depth = ClipDepth::Standard
) noexcept
{
  auto const [mapped_to_0, mapped_to_1] = depth == ClipDepth::Standard ? std::pair(z_near, z_far)
                                                                       : std::pair(z_far, z_near);
  auto const forward = handedness == Handedness::Right ? Real {-1} : Real {1};
  auto const range = mapped_to_1 - mapped_to_0;
  return Matrix4x4::from_rows(
    {2 / (right - left), 0.0, 0.0, -(right + left) / (right - left)},
    {0.0, 2 / (top - bottom), 0.0, -(top + bottom) / (top - bottom)},
    {0.0, 0.0, forward / range, -mapped_to_0 / range},
    Vector4::unit_w()
  );
}

/**
 * @brief Returns the view matrix of a camera at @p eye looking at @p target, its up axis leaning
 * toward @p up.
 * @details The view matrix maps world space to view space, where x is right and y is up and the
 * camera looks along −z for Handedness::Right or +z for Handedness::Left. It is the inverse of the
 * camera transform with Quaternion::look_rotation(). Matches glm's `lookAtRH` and `lookAtLH`.
 * Where @p up is parallel to the line of sight, the roll is arbitrary rather than undefined.
 * @throws NumericError if @p target is @p eye or either is not finite.
 * @ingroup core_math
 * @see look_at_rh
 * @see look_at_lh
 */
[[nodiscard]] inline Matrix4x4 look_at(
  Vector3 const eye,
  Vector3 const target,
  Vector3 const up,
  Handedness const handedness
)
{
  auto const forward = (target - eye).try_normalized();
  if(not forward.has_value())
    throw NumericError("cannot look from {} at {}", eye, target);

  // the camera axes in world space are the columns of the camera rotation
  auto const z = handedness == Handedness::Right ? -*forward : *forward;
  auto const x = up.cross(z).try_normalized();
  auto const camera = x.has_value()
                      ? Matrix3x3::from_columns(*x, z.cross(*x), z)
                      : Quaternion::look_rotation(*forward, up, handedness).to_rotation_matrix();
  auto const world_to_view = camera.transposed();
  return Matrix4x4::from_affine(world_to_view, -(world_to_view * eye));
}

/**
 * @brief Returns the right-handed view matrix, the camera looking along −z: look_at() with
 * Handedness::Right.
 * @throws NumericError if @p target is @p eye or either is not finite.
 * @ingroup core_math
 */
[[nodiscard]] inline Matrix4x4 look_at_rh(Vector3 const eye, Vector3 const target, Vector3 const up)
{
  return look_at(eye, target, up, Handedness::Right);
}

/**
 * @brief Returns the left-handed view matrix, the camera looking along +z: look_at() with
 * Handedness::Left.
 * @throws NumericError if @p target is @p eye or either is not finite.
 * @ingroup core_math
 */
[[nodiscard]] inline Matrix4x4 look_at_lh(Vector3 const eye, Vector3 const target, Vector3 const up)
{
  return look_at(eye, target, up, Handedness::Left);
}

/**
 * @brief A camera position snapped to the texel grid of a low-resolution render target, and the
 * part of the move the snap took away.
 * @ingroup core_math
 * @see snap_to_texel
 */
struct TexelSnap
{
  /**
   * @brief The camera position to render from, in world space: whole texels along the camera's
   * x and y axes.
   */
  Vector3 position;

  /**
   * @brief How far the true camera is from the snapped one along its x (right) and y (up) axes,
   * in texels, each in `[-0.5, 0.5]`.
   * @details Static geometry renders identically every frame from the snapped position, so it
   * does not shimmer. Shifting the upscaled image by `-offset_texels` texels (screen x right, y
   * up; for a y-down screen, `(-x, +y)`) restores the smooth motion of the true camera.
   */
  Vector2 offset_texels;

  [[nodiscard]] constexpr bool operator==(TexelSnap const&) const noexcept = default;
};

/**
 * @brief Snaps the camera @p position to the texel grid of an orthographic, low-resolution render
 * target, for a pixel-art camera that moves smoothly by whole texels plus a subpixel image shift.
 * @details The grid is aligned with the camera, so @p position is snapped along the x and y axes
 * of @p orientation, the camera-to-world rotation, e.g. Quaternion::look_rotation(); its depth
 * along the line of sight is kept.
 * @param position Camera position in world space.
 * @param orientation Camera-to-world rotation, a unit quaternion.
 * @param texel_size World size of one texel of the render target, e.g. `(top - bottom) / height`
 * of its orthographic(); positive.
 * @ingroup core_math
 */
[[nodiscard]] constexpr TexelSnap snap_to_texel(
  Vector3 const position,
  Quaternion const& orientation,
  Real const texel_size
) noexcept
{
  auto const camera = orientation.inverse_rotate(position);
  auto const texels = camera.xy() / texel_size;
  auto const snapped = Vector2(floor(texels.x() + Real {0.5}), floor(texels.y() + Real {0.5}));
  return {
    .position = orientation.rotate({snapped * texel_size, camera.z()}),
    .offset_texels = texels - snapped,
  };
}

W_COMMON_NAMESPACE_END
