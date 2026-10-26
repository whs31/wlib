#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/glaze/quaternion.h>
#include <w/common/glaze/vector3.h>
#include <w/common/transform.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct TransformFields
  {
    BasicVector3<T> position;
    BasicQuaternion<T> rotation;
    BasicVector3<T> scale;
  };

  // every key may be left out of a document, even a strictly read one, to keep the identity part
  template <std::floating_point T>
  struct TransformInput
  {
    std::optional<BasicVector3<T>> position;
    std::optional<BasicQuaternion<T>> rotation;
    std::optional<BasicVector3<T>> scale;
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
template <std::floating_point T>
struct glz::meta<w::common::BasicTransform<T>>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::BasicTransform<T>& transform,
      w::common::_glaze_impl::TransformInput<T> const& input
    ) {
      auto const identity = w::common::BasicTransform<T>::identity();
      transform = w::common::BasicTransform<T>(
        input.position.value_or(identity.position()),
        input.rotation.value_or(identity.rotation()),
        input.scale.value_or(identity.scale())
      );
    },
    [](w::common::BasicTransform<T> const& transform) {
      return w::common::_glaze_impl::TransformFields<T> {
        .position = transform.position(),
        .rotation = transform.rotation(),
        .scale = transform.scale(),
      };
    }>;
};
