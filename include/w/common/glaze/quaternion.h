#pragma once

#include <w/common/glaze/detail.h>
#include <w/common/quaternion.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct QuaternionFields
  {
    Real w = 0.0;
    Real x = 0.0;
    Real y = 0.0;
    Real z = 0.0;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Quaternion: `{"w": w, "x": x, "y": y, "z": z}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Quaternion>
{
  static auto constexpr value = glz::custom<
    [](w::common::Quaternion& quaternion, w::common::_glaze_impl::QuaternionFields const& fields) {
      quaternion = w::common::Quaternion::from_wxyz(fields.w, fields.x, fields.y, fields.z);
    },
    [](w::common::Quaternion const& quaternion) {
      return w::common::_glaze_impl::QuaternionFields {
        .w = quaternion.w(),
        .x = quaternion.x(),
        .y = quaternion.y(),
        .z = quaternion.z(),
      };
    }>;
};
