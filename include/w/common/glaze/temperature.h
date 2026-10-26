#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/temperature.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct TemperatureFields
  {
    std::optional<T> kelvin;
    std::optional<T> celsius;
    std::optional<T> fahrenheit;
  };

  template <std::floating_point T>
  inline auto constexpr temperature_reading = QuantityReading<
    BasicTemperature<T>,
    TemperatureFields<T>,
    3,
    3> {
    .fields =
      {
        field_unit(&TemperatureFields<T>::kelvin, &BasicTemperature<T>::from_kelvin),
        field_unit(&TemperatureFields<T>::celsius, &BasicTemperature<T>::from_celsius),
        field_unit(&TemperatureFields<T>::fahrenheit, &BasicTemperature<T>::from_fahrenheit),
      },
    .literals =
      {
        literal_unit("k", &BasicTemperature<T>::from_kelvin),
        literal_unit("degc", &BasicTemperature<T>::from_celsius),
        literal_unit("degf", &BasicTemperature<T>::from_fahrenheit),
      },
    .unknown_suffix = "a temperature literal needs one of the unit suffixes k, degc, degf",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Temperature: `{"kelvin": magnitude}`.
 * @details Reading also accepts the magnitude in `celsius` or `fahrenheit` instead. A text
 * format also reads a literal string, e.g. `"12.5k"`, with the suffix of a C++ literal: `k`,
 * `degc`, `degf`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicTemperature<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::temperature_reading<T>>::value;
};
