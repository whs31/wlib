#pragma once

#include <concepts>
#include <w/common/glaze/detail.h>
#include <w/common/glaze/vector3.h>
#include <w/common/ray.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct RayFields
  {
    BasicVector3<T> origin;
    BasicVector3<T> direction;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Ray: `{"origin": [x, y, z], "direction": [x, y, z]}`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicRay<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicRay<T>& ray, w::common::_glaze_impl::RayFields<T> const& fields) {
      ray = w::common::BasicRay<T>(fields.origin, fields.direction);
    },
    [](w::common::BasicRay<T> const& ray) {
      return w::common::_glaze_impl::RayFields<T> {
        .origin = ray.origin(),
        .direction = ray.direction(),
      };
    }>;
};
