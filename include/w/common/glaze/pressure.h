#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/pressure.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct PressureFields
  {
    std::optional<Real> pascals;
    std::optional<Real> hectopascals;
    std::optional<Real> kilopascals;
    std::optional<Real> millibars;
    std::optional<Real> inches_of_mercury;
  };

  inline auto constexpr pressure_reading = QuantityReading<Pressure, PressureFields, 5, 5> {
    .fields =
      {
        field_unit(&PressureFields::pascals, &Pressure::from_pascals),
        field_unit(&PressureFields::hectopascals, &Pressure::from_hectopascals),
        field_unit(&PressureFields::kilopascals, &Pressure::from_kilopascals),
        field_unit(&PressureFields::millibars, &Pressure::from_millibars),
        field_unit(&PressureFields::inches_of_mercury, &Pressure::from_inches_of_mercury),
      },
    .literals =
      {
        literal_unit("pa", &Pressure::from_pascals),
        literal_unit("hpa", &Pressure::from_hectopascals),
        literal_unit("kpa", &Pressure::from_kilopascals),
        literal_unit("mbar", &Pressure::from_millibars),
        literal_unit("inhg", &Pressure::from_inches_of_mercury),
      },
    .unknown_suffix = "a pressure literal needs one of the unit suffixes pa, hpa, kpa, mbar, inhg",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Pressure: `{"pascals": magnitude}`.
 * @details Reading also accepts the magnitude in `hectopascals`, `kilopascals`, `millibars` or
 * `inches_of_mercury` instead. A text format also reads a literal string, e.g. `"12.5pa"`, with
 * the suffix of a C++ literal: `pa`, `hpa`, `kpa`, `mbar`, `inhg`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Pressure>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::pressure_reading>::value;
};
