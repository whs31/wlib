#pragma once

#include <array>
#include <bit>

#include <w/common/clip_space.h>
#include <w/common/detail/namespace.h>
#include <w/common/euler_angles.h>
#include <w/common/matrix3x3.h>
#include <w/common/matrix4x4.h>
#include <w/common/precision.h>
#include <w/common/projection.h>
#include <w/common/quaternion.h>
#include <w/common/vector3.h>
#include <w/common/vector4.h>

/**
 * @page conventions Linear algebra conventions
 *
 * The conventions below are settled for every WLib math type. The `static_assert`s at the end of
 * `<w/common/conventions.h>` pin each one down, so a change that breaks a convention does not
 * compile.
 *
 * @section conventions_matrices Matrices: row-major storage, column vectors
 * Matrix3x3 and Matrix4x4 store their rows contiguously and multiply column vectors, `M * v`:
 * `(m * v).x() == m.row(0).dot(v)`, the translation of an affine transform is the last column, and
 * `a * b` applies `b` first, so `projection * view * model * point` reads right to left. Matrices
 * are built with `from_rows()` and `from_columns()`, so helisim's `Matrix3x3::from_rows()` code
 * keeps its meaning.
 *
 * glm stores columns, so its `m[c][r]` is WLib's `m.at(r, c)` and its memory is the transpose of
 * WLib's; the glm comparison tests convert accordingly.
 *
 * @par Shaders
 * Every shader is compiled with Diligent's `SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR` and
 * multiplies as `mul(M, v)`. The flag makes HLSL read a `float4x4` constant row by row, exactly as
 * Matrix4x4 stores it, so matrices are copied into constant buffers as they are, with no
 * transpose anywhere. It is one flag in one place rather than a `row_major` qualifier on every
 * declaration, so a new constant buffer cannot forget it; never mix it with explicit `row_major`
 * or `column_major` qualifiers.
 *
 * @section conventions_quaternions Quaternions: w, x, y, z
 * Quaternion stores `w, x, y, z`, the Hamilton convention, and rotates actively and right-handedly.
 * Jolt, HLSL and glm store `x, y, z, w`; Quaternion::from_xyzw() and Quaternion::to_xyzw()
 * convert at those edges, member by member, which a Jolt `Quat` (a 16-byte SIMD type) needs
 * anyway. Keeping `w` first keeps helisim's reflected and recorded data unchanged.
 *
 * @section conventions_frames Handedness, up axis and frames
 * The math types are frame-agnostic: a Vector3 does not know whether it is in NED or in a y-up
 * world. Frames belong to the code that uses them and to variable names (`velocity_ned`,
 * `position_world`). Functions that need a handedness take one: look_at_rh(), look_at_lh(),
 * perspective(), orthographic() and Quaternion::look_rotation(). Projections map depth to
 * `[0, 1]`, optionally reversed (ClipDepth).
 *
 * - The game is right-handed with y up: x right, y up, the camera looking along −z.
 * - helisim stays in NED through LocalTangentPlane and LocalTangentPlane-relative vectors.
 *
 * @section conventions_euler Euler angles name their order
 * There is no unqualified "Euler angles" conversion. Quaternion::from_yaw_pitch_roll() and
 * Quaternion::to_yaw_pitch_roll() are the aerospace ZYX sequence of EulerAngles (helisim);
 * Quaternion::from_euler() and Quaternion::to_euler() take the EulerOrder as a template argument,
 * e.g. EulerOrder::YXZ for the editor's y-up display angles (the game). The angles are passed by
 * axis, so changing the order never reshuffles arguments.
 *
 * @section conventions_layout Layout
 * Vector3 is three floats, 12 bytes, with no SIMD padding; offset-based reflection depends on
 * that. A hot loop that needs SIMD gets batch functions over `std::span`, not padding.
 */

W_COMMON_NAMESPACE_BEGIN

namespace _conventions_impl  // NOLINT(*-identifier-naming)
{
  inline auto constexpr translation = Matrix4x4::from_translation({1.0, 2.0, 3.0});
  inline auto constexpr scale = Matrix4x4::from_scale({2.0, 2.0, 2.0});
}  // namespace _conventions_impl

// matrices: row-major storage, the translation in the last column
static_assert(Matrix4x4::from_rows({0, 1, 2, 3}, {}, {}, {}).to_array()[3] == 3);
static_assert(_conventions_impl::translation.to_array()[3] == 1.0F);
static_assert(_conventions_impl::translation.to_array()[7] == 2.0F);
static_assert(_conventions_impl::translation.at(2, 3) == 3.0F);
static_assert(
  std::bit_cast<std::array<Real, 9>>(Matrix3x3::from_rows({1, 2, 3}, {4, 5, 6}, {7, 8, 9}))[1]
  == 2.0F
);

// column vectors: M * v, and a * b applies b first
static_assert(
  _conventions_impl::translation.transform_point(Vector3::zero()) == Vector3(1.0, 2.0, 3.0)
);
static_assert(
  (_conventions_impl::translation * _conventions_impl::scale).transform_point({1.0, 1.0, 1.0})
  == Vector3(3.0, 4.0, 5.0)
);
static_assert(
  (Matrix3x3::from_rows({0, -1, 0}, {1, 0, 0}, {0, 0, 1}) * Vector3::unit_x()) == Vector3::unit_y()
);

// a point translates, a direction does not
static_assert(
  _conventions_impl::translation.transform_direction(Vector3::unit_x()) == Vector3::unit_x()
);
static_assert((_conventions_impl::translation * Vector4::from_point(Vector3::zero())).w() == 1.0F);

// quaternions: w, x, y, z storage, Hamilton product, right-handed
static_assert(
  std::bit_cast<std::array<Real, 4>>(Quaternion::from_xyzw(1, 2, 3, 4))
  == std::array<Real, 4> {4, 1, 2, 3}
);
static_assert(Quaternion::from_xyzw(1, 2, 3, 4).to_xyzw() == std::array<Real, 4> {1, 2, 3, 4});
static_assert(Quaternion(0, 1, 0, 0) * Quaternion(0, 0, 1, 0) == Quaternion(0, 0, 0, 1));
static_assert(Vector3::unit_x().cross(Vector3::unit_y()) == Vector3::unit_z());
// a half turn about z maps x to -x, and the matrix agrees
static_assert(Quaternion(0, 0, 0, 1).rotate(Vector3::unit_x()) == -Vector3::unit_x());
static_assert(
  Quaternion(0, 0, 0, 1).to_rotation_matrix() * Vector3::unit_y() == -Vector3::unit_y()
);

// projections: depth in [0, 1], the near plane at 0, or at 1 when reversed
static_assert(
  orthographic(-1, 1, -1, 1, 1, 3, Handedness::Right).project_point({0, 0, -1}).z() == 0.0F
);
static_assert(
  orthographic(-1, 1, -1, 1, 1, 3, Handedness::Right).project_point({0, 0, -3}).z() == 1.0F
);
static_assert(
  orthographic(-1, 1, -1, 1, 1, 3, Handedness::Left).project_point({0, 0, 1}).z() == 0.0F
);
static_assert(
  orthographic(-1, 1, -1, 1, 1, 3, Handedness::Right, ClipDepth::Reversed)
    .project_point({0, 0, -1})
    .z()
  == 1.0F
);

// layout: Vector3 is three floats without SIMD padding
static_assert(sizeof(Vector3) == 12);
static_assert(sizeof(Real) == sizeof(float));

W_COMMON_NAMESPACE_END
