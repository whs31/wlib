#pragma once

#include <w/common/aabb.h>
#include <w/common/glaze/detail.h>
#include <w/common/glaze/vector3.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct AABBFields
  {
    Vector3 min;
    Vector3 max;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of AABB: `{"min": [x, y, z], "max": [x, y, z]}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::AABB>
{
  static auto constexpr value = glz::custom<
    [](w::common::AABB& box, w::common::_glaze_impl::AABBFields const& fields) {
      box = w::common::AABB(fields.min, fields.max);
    },
    [](w::common::AABB const& box) {
      return w::common::_glaze_impl::AABBFields {
        .min = box.min(),
        .max = box.max(),
      };
    }>;
};
