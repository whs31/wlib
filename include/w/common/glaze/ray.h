#pragma once

#include <w/common/glaze/detail.h>
#include <w/common/glaze/vector3.h>
#include <w/common/ray.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct RayFields
  {
    Vector3 origin;
    Vector3 direction;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Ray: `{"origin": [x, y, z], "direction": [x, y, z]}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Ray>
{
  static auto constexpr value = glz::custom<
    [](w::common::Ray& ray, w::common::_glaze_impl::RayFields const& fields) {
      ray = w::common::Ray(fields.origin, fields.direction);
    },
    [](w::common::Ray const& ray) {
      return w::common::_glaze_impl::RayFields {
        .origin = ray.origin(),
        .direction = ray.direction(),
      };
    }>;
};
