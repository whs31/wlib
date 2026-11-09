#pragma once

#include <w/common/stdint.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Handedness of a view space, which look_at(), perspective(), orthographic() and
 * Quaternion::look_rotation() take, since the math types themselves are frame-agnostic.
 * @details In both, x points right and y points up on the screen:
 * - **Right**: the camera looks along −z, as in OpenGL, glm's `*RH` functions and Blender;
 * - **Left**: the camera looks along +z, as in Direct3D and Unity.
 *
 * The game picks Right with y up; helisim keeps NED through LocalTangentPlane. See @ref
 * conventions.
 * @ingroup core_math
 */
enum class Handedness : u8
{
  Right,
  Left,
};

/**
 * @brief How a projection maps view depth into the `[0, 1]` depth range of Direct3D, Vulkan and
 * Diligent.
 * @details
 * - **Standard**: the near plane maps to 0 and the far plane to 1; the depth test is `less`.
 * - **Reversed**: the near plane maps to 1 and the far plane to 0; the depth test is `greater` and
 *   the depth buffer is cleared to 0. Float depth then keeps its precision far from the camera,
 *   and perspective() accepts an infinite far plane.
 * @ingroup core_math
 */
enum class ClipDepth : u8
{
  Standard,
  Reversed,
};

W_COMMON_NAMESPACE_END
