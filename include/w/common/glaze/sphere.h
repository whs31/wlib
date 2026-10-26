#pragma once

#include <concepts>
#include <w/common/glaze/detail.h>
#include <w/common/glaze/vector3.h>
#include <w/common/sphere.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct SphereFields
  {
    BasicVector3<T> center;
    T radius = 0.0;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Sphere: `{"center": [x, y, z], "radius": r}`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicSphere<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicSphere<T>& sphere, w::common::_glaze_impl::SphereFields<T> const& fields) {
      sphere = w::common::BasicSphere<T>(fields.center, fields.radius);
    },
    [](w::common::BasicSphere<T> const& sphere) {
      return w::common::_glaze_impl::SphereFields<T> {
        .center = sphere.center(),
        .radius = sphere.radius(),
      };
    }>;
};
