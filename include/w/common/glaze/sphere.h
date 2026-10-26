#pragma once

#include <w/common/glaze/detail.h>
#include <w/common/glaze/vector3.h>
#include <w/common/sphere.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct SphereFields
  {
    Vector3 center;
    Real radius = 0.0;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Sphere: `{"center": [x, y, z], "radius": r}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Sphere>
{
  static auto constexpr value = glz::custom<
    [](w::common::Sphere& sphere, w::common::_glaze_impl::SphereFields const& fields) {
      sphere = w::common::Sphere(fields.center, fields.radius);
    },
    [](w::common::Sphere const& sphere) {
      return w::common::_glaze_impl::SphereFields {
        .center = sphere.center(),
        .radius = sphere.radius(),
      };
    }>;
};
