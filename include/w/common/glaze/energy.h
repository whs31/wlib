#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/energy.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct EnergyFields
  {
    std::optional<Real> joules;
    std::optional<Real> watt_hours;
    std::optional<Real> kilowatt_hours;
  };

  inline auto constexpr energy_reading = QuantityReading<Energy, EnergyFields, 3, 3> {
    .fields =
      {
        field_unit(&EnergyFields::joules, &Energy::from_joules),
        field_unit(&EnergyFields::watt_hours, &Energy::from_watt_hours),
        field_unit(&EnergyFields::kilowatt_hours, &Energy::from_kilowatt_hours),
      },
    .literals =
      {
        literal_unit("j", &Energy::from_joules),
        literal_unit("wh", &Energy::from_watt_hours),
        literal_unit("kwh", &Energy::from_kilowatt_hours),
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
template <>
struct glz::meta<w::common::Energy>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::energy_reading>::value;
};
