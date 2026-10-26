#pragma once

#include <concepts>
#include <w/common/euler_angles.h>
#include <w/common/glaze/angle.h>
#include <w/common/glaze/detail.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct EulerAnglesFields
  {
    BasicAngle<T> roll;
    BasicAngle<T> pitch;
    BasicAngle<T> yaw;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of EulerAngles: `{"roll": angle, "pitch": angle, "yaw": angle}`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicEulerAngles<T>>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::BasicEulerAngles<T>& angles,
      w::common::_glaze_impl::EulerAnglesFields<T> const& fields
    ) { angles = w::common::BasicEulerAngles<T>(fields.roll, fields.pitch, fields.yaw); },
    [](w::common::BasicEulerAngles<T> const& angles) {
      return w::common::_glaze_impl::EulerAnglesFields<T> {
        .roll = angles.roll(),
        .pitch = angles.pitch(),
        .yaw = angles.yaw(),
      };
    }>;
};
