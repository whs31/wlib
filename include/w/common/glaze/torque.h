#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/torque.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct TorqueFields
  {
    std::optional<T> newton_meters;
  };

  template <std::floating_point T>
  inline auto constexpr torque_reading = QuantityReading<BasicTorque<T>, TorqueFields<T>, 1, 1> {
    .fields =
      {
        field_unit(&TorqueFields<T>::newton_meters, &BasicTorque<T>::from_newton_meters),
      },
    .literals =
      {
        literal_unit("nm", &BasicTorque<T>::from_newton_meters),
      },
    .unknown_suffix = "a torque literal needs one of the unit suffixes nm",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Torque: `{"newton_meters": magnitude}`.
 * @details A text format also reads a literal string, e.g. `"12.5nm"`, with the suffix of a C++
 * literal: `nm`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicTorque<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::torque_reading<T>>::value;
};
