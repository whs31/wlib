#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/magnetic_flux_density.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct MagneticFluxDensityFields
  {
    std::optional<Real> teslas;
    std::optional<Real> gauss;
    std::optional<Real> microteslas;
    std::optional<Real> nanoteslas;
  };

  inline auto constexpr magnetic_flux_density_reading = QuantityReading<
    MagneticFluxDensity,
    MagneticFluxDensityFields,
    4,
    3> {
    .fields =
      {
        field_unit(&MagneticFluxDensityFields::teslas, &MagneticFluxDensity::from_teslas),
        field_unit(&MagneticFluxDensityFields::gauss, &MagneticFluxDensity::from_gauss),
        field_unit(&MagneticFluxDensityFields::microteslas, &MagneticFluxDensity::from_microteslas),
        field_unit(&MagneticFluxDensityFields::nanoteslas, &MagneticFluxDensity::from_nanoteslas),
      },
    .literals =
      {
        literal_unit("gauss", &MagneticFluxDensity::from_gauss),
        literal_unit("ut", &MagneticFluxDensity::from_microteslas),
        literal_unit("nt", &MagneticFluxDensity::from_nanoteslas),
      },
    .unknown_suffix =
      "a magnetic flux density literal needs one of the unit suffixes gauss, ut, "
      "nt",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of MagneticFluxDensity: `{"teslas": magnitude}`.
 * @details Reading also accepts the magnitude in `gauss`, `microteslas` or `nanoteslas`
 * instead. A text format also reads a literal string, e.g. `"12.5gauss"`, with the suffix of a
 * C++ literal: `gauss`, `ut`, `nt`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::MagneticFluxDensity>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::magnetic_flux_density_reading>::value;
};
