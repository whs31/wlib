#pragma once

#include <concepts>
#include <w/common/aabb.h>
#include <w/common/glaze/detail.h>
#include <w/common/glaze/vector3.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct AABBFields
  {
    BasicVector3<T> min;
    BasicVector3<T> max;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of AABB: `{"min": [x, y, z], "max": [x, y, z]}`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicAABB<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicAABB<T>& box, w::common::_glaze_impl::AABBFields<T> const& fields) {
      box = w::common::BasicAABB<T>(fields.min, fields.max);
    },
    [](w::common::BasicAABB<T> const& box) {
      return w::common::_glaze_impl::AABBFields<T> {
        .min = box.min(),
        .max = box.max(),
      };
    }>;
};
