#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/density.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct DensityFields
  {
    std::optional<T> kilograms_per_cubic_meter;
  };

  template <std::floating_point T>
  inline auto constexpr density_reading = QuantityReading<BasicDensity<T>, DensityFields<T>, 1, 1> {
    .fields =
      {
        field_unit(
          &DensityFields<T>::kilograms_per_cubic_meter,
          &BasicDensity<T>::from_kilograms_per_cubic_meter
        ),
      },
    .literals =
      {
        literal_unit("kg_per_m3", &BasicDensity<T>::from_kilograms_per_cubic_meter),
      },
    .unknown_suffix = "a density literal needs one of the unit suffixes kg_per_m3",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Density: `{"kilograms_per_cubic_meter": magnitude}`.
 * @details A text format also reads a literal string, e.g. `"12.5kg_per_m3"`, with the suffix
 * of a C++ literal: `kg_per_m3`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicDensity<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::density_reading<T>>::value;
};
