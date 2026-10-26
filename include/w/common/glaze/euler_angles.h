#pragma once

#include <w/common/euler_angles.h>
#include <w/common/glaze/angle.h>
#include <w/common/glaze/detail.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct EulerAnglesFields
  {
    Angle roll;
    Angle pitch;
    Angle yaw;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of EulerAngles: `{"roll": angle, "pitch": angle, "yaw": angle}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::EulerAngles>
{
  static auto constexpr value = glz::custom<
    [](w::common::EulerAngles& angles, w::common::_glaze_impl::EulerAnglesFields const& fields) {
      angles = w::common::EulerAngles(fields.roll, fields.pitch, fields.yaw);
    },
    [](w::common::EulerAngles const& angles) {
      return w::common::_glaze_impl::EulerAnglesFields {
        .roll = angles.roll(),
        .pitch = angles.pitch(),
        .yaw = angles.yaw(),
      };
    }>;
};
