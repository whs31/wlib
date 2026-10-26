#pragma once

#include <w/common/glaze/detail.h>
#include <w/common/point3.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct Point3Fields
  {
    Real x = 0.0;
    Real y = 0.0;
    Real z = 0.0;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Point3: `{"x": x, "y": y, "z": z}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Point3>
{
  static auto constexpr value = glz::custom<
    [](w::common::Point3& point3, w::common::_glaze_impl::Point3Fields const& fields) {
      point3 = w::common::Point3::from_xyz(fields.x, fields.y, fields.z);
    },
    [](w::common::Point3 const& point3) {
      return w::common::_glaze_impl::Point3Fields {
        .x = point3.x(),
        .y = point3.y(),
        .z = point3.z(),
      };
    }>;
};
