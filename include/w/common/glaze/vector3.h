#pragma once

#include <w/common/glaze/detail.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct Vector3Fields
  {
    Real x = 0.0;
    Real y = 0.0;
    Real z = 0.0;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Vector3: `{"x": x, "y": y, "z": z}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Vector3>
{
  static auto constexpr value = glz::custom<
    [](w::common::Vector3& vector3, w::common::_glaze_impl::Vector3Fields const& fields) {
      vector3 = w::common::Vector3::from_xyz(fields.x, fields.y, fields.z);
    },
    [](w::common::Vector3 const& vector3) {
      return w::common::_glaze_impl::Vector3Fields {
        .x = vector3.x(),
        .y = vector3.y(),
        .z = vector3.z(),
      };
    }>;
};
