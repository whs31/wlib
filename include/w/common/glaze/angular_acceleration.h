#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/angular_acceleration.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct AngularAccelerationFields
  {
    std::optional<T> radians_per_second_squared;
    std::optional<T> degrees_per_second_squared;
  };

  template <std::floating_point T>
  inline auto constexpr angular_acceleration_reading = QuantityReading<
    BasicAngularAcceleration<T>,
    AngularAccelerationFields<T>,
    2,
    2> {
    .fields =
      {
        field_unit(
          &AngularAccelerationFields<T>::radians_per_second_squared,
          &BasicAngularAcceleration<T>::from_radians_per_second_squared
        ),
        field_unit(
          &AngularAccelerationFields<T>::degrees_per_second_squared,
          &BasicAngularAcceleration<T>::from_degrees_per_second_squared
        ),
      },
    .literals =
      {
        literal_unit("rad_per_s2", &BasicAngularAcceleration<T>::from_radians_per_second_squared),
        literal_unit("deg_per_s2", &BasicAngularAcceleration<T>::from_degrees_per_second_squared),
      },
    .unknown_suffix =
      "a angular acceleration literal needs one of the unit suffixes rad_per_s2, deg_per_s2",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of AngularAcceleration: `{"radians_per_second_squared": magnitude}`.
 * @details Reading also accepts the magnitude in `degrees_per_second_squared` instead. A text
 * format also reads a literal string, e.g. `"12.5rad_per_s2"`, with the suffix of a C++
 * literal: `rad_per_s2`, `deg_per_s2`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicAngularAcceleration<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::angular_acceleration_reading<T>>::value;
};
