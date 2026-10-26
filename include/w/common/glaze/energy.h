#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/energy.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct EnergyFields
  {
    std::optional<T> joules;
    std::optional<T> watt_hours;
    std::optional<T> kilowatt_hours;
  };

  template <std::floating_point T>
  inline auto constexpr energy_reading = QuantityReading<BasicEnergy<T>, EnergyFields<T>, 3, 3> {
    .fields =
      {
        field_unit(&EnergyFields<T>::joules, &BasicEnergy<T>::from_joules),
        field_unit(&EnergyFields<T>::watt_hours, &BasicEnergy<T>::from_watt_hours),
        field_unit(&EnergyFields<T>::kilowatt_hours, &BasicEnergy<T>::from_kilowatt_hours),
      },
    .literals =
      {
        literal_unit("j", &BasicEnergy<T>::from_joules),
        literal_unit("wh", &BasicEnergy<T>::from_watt_hours),
        literal_unit("kwh", &BasicEnergy<T>::from_kilowatt_hours),
      },
    .unknown_suffix = "a energy literal needs one of the unit suffixes j, wh, kwh",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Energy: `{"joules": magnitude}`.
 * @details Reading also accepts the magnitude in `watt_hours` or `kilowatt_hours` instead. A
 * text format also reads a literal string, e.g. `"12.5j"`, with the suffix of a C++ literal:
 * `j`, `wh`, `kwh`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicEnergy<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::energy_reading<T>>::value;
};
