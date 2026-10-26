#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/voltage.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct VoltageFields
  {
    std::optional<T> volts;
    std::optional<T> millivolts;
  };

  template <std::floating_point T>
  inline auto constexpr voltage_reading = QuantityReading<BasicVoltage<T>, VoltageFields<T>, 2, 2> {
    .fields =
      {
        field_unit(&VoltageFields<T>::volts, &BasicVoltage<T>::from_volts),
        field_unit(&VoltageFields<T>::millivolts, &BasicVoltage<T>::from_millivolts),
      },
    .literals =
      {
        literal_unit("v", &BasicVoltage<T>::from_volts),
        literal_unit("mv", &BasicVoltage<T>::from_millivolts),
      },
    .unknown_suffix = "a voltage literal needs one of the unit suffixes v, mv",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Voltage: `{"volts": magnitude}`.
 * @details Reading also accepts the magnitude in `millivolts` instead. A text format also reads
 * a literal string, e.g. `"12.5v"`, with the suffix of a C++ literal: `v`, `mv`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicVoltage<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::voltage_reading<T>>::value;
};
