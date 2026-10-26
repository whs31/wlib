#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/acceleration.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct AccelerationFields
  {
    std::optional<T> meters_per_second_squared;
    std::optional<T> standard_gravity;
  };

  template <std::floating_point T>
  inline auto constexpr acceleration_reading = QuantityReading<
    BasicAcceleration<T>,
    AccelerationFields<T>,
    2,
    2> {
    .fields =
      {
        field_unit(
          &AccelerationFields<T>::meters_per_second_squared,
          &BasicAcceleration<T>::from_mps2
        ),
        field_unit(
          &AccelerationFields<T>::standard_gravity,
          &BasicAcceleration<T>::from_standard_gravity
        ),
      },
    .literals =
      {
        literal_unit("mps2", &BasicAcceleration<T>::from_mps2),
        literal_unit("gn", &BasicAcceleration<T>::from_standard_gravity),
      },
    .unknown_suffix = "a acceleration literal needs one of the unit suffixes mps2, gn",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Acceleration: `{"meters_per_second_squared": magnitude}`.
 * @details Reading also accepts the magnitude in `standard_gravity` instead. A text format also
 * reads a literal string, e.g. `"12.5mps2"`, with the suffix of a C++ literal: `mps2`, `gn`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicAcceleration<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::acceleration_reading<T>>::value;
};
