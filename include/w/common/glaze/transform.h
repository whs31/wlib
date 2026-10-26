#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/glaze/quaternion.h>
#include <w/common/glaze/vector3.h>
#include <w/common/transform.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct TransformFields
  {
    Vector3 position;
    Quaternion rotation;
    Vector3 scale;
  };

  // every key may be left out of a document, even a strictly read one, to keep the identity part
  struct TransformInput
  {
    std::optional<Vector3> position;
    std::optional<Quaternion> rotation;
    std::optional<Vector3> scale;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Transform: `{"position": [x, y, z], "rotation": [w, x, y, z],
 * "scale": [x, y, z]}`.
 * @details Every key may be missing, in any read, and keeps the identity: zero position, identity
 * rotation, unit scale. So a TOML document may write just `transform = { position = [0, 1, 0] }`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Transform>
{
  static auto constexpr value = glz::custom<
    [](w::common::Transform& transform, w::common::_glaze_impl::TransformInput const& input) {
      auto const identity = w::common::Transform::identity();
      transform = w::common::Transform(
        input.position.value_or(identity.position()),
        input.rotation.value_or(identity.rotation()),
        input.scale.value_or(identity.scale())
      );
    },
    [](w::common::Transform const& transform) {
      return w::common::_glaze_impl::TransformFields {
        .position = transform.position(),
        .rotation = transform.rotation(),
        .scale = transform.scale(),
      };
    }>;
};
