#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/moment_of_inertia.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct MomentOfInertiaFields
  {
    std::optional<T> kilogram_square_meters;
  };

  template <std::floating_point T>
  inline auto constexpr moment_of_inertia_reading = QuantityReading<
    BasicMomentOfInertia<T>,
    MomentOfInertiaFields<T>,
    1,
    1> {
    .fields =
      {
        field_unit(
          &MomentOfInertiaFields<T>::kilogram_square_meters,
          &BasicMomentOfInertia<T>::from_kilogram_square_meters
        ),
      },
    .literals =
      {
        literal_unit("kg_m2", &BasicMomentOfInertia<T>::from_kilogram_square_meters),
      },
    .unknown_suffix = "a moment of inertia literal needs one of the unit suffixes kg_m2",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of MomentOfInertia: `{"kilogram_square_meters": magnitude}`.
 * @details A text format also reads a literal string, e.g. `"12.5kg_m2"`, with the suffix of a
 * C++ literal: `kg_m2`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicMomentOfInertia<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::moment_of_inertia_reading<T>>::value;
};
