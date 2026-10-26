#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/magnetic_flux_density.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct MagneticFluxDensityFields
  {
    std::optional<T> teslas;
    std::optional<T> gauss;
    std::optional<T> microteslas;
    std::optional<T> nanoteslas;
  };

  template <std::floating_point T>
  inline auto constexpr magnetic_flux_density_reading = QuantityReading<
    BasicMagneticFluxDensity<T>,
    MagneticFluxDensityFields<T>,
    4,
    3> {
    .fields =
      {
        field_unit(
          &MagneticFluxDensityFields<T>::teslas,
          &BasicMagneticFluxDensity<T>::from_teslas
        ),
        field_unit(&MagneticFluxDensityFields<T>::gauss, &BasicMagneticFluxDensity<T>::from_gauss),
        field_unit(
          &MagneticFluxDensityFields<T>::microteslas,
          &BasicMagneticFluxDensity<T>::from_microteslas
        ),
        field_unit(
          &MagneticFluxDensityFields<T>::nanoteslas,
          &BasicMagneticFluxDensity<T>::from_nanoteslas
        ),
      },
    .literals =
      {
        literal_unit("gauss", &BasicMagneticFluxDensity<T>::from_gauss),
        literal_unit("ut", &BasicMagneticFluxDensity<T>::from_microteslas),
        literal_unit("nt", &BasicMagneticFluxDensity<T>::from_nanoteslas),
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
template <std::floating_point T>
struct glz::meta<w::common::BasicMagneticFluxDensity<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::magnetic_flux_density_reading<T>>::value;
};
