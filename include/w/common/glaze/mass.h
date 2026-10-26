#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/mass.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct MassFields
  {
    std::optional<T> kilograms;
    std::optional<T> grams;
  };

  template <std::floating_point T>
  inline auto constexpr mass_reading = QuantityReading<BasicMass<T>, MassFields<T>, 2, 2> {
    .fields =
      {
        field_unit(&MassFields<T>::kilograms, &BasicMass<T>::from_kilograms),
        field_unit(&MassFields<T>::grams, &BasicMass<T>::from_grams),
      },
    .literals =
      {
        literal_unit("kg", &BasicMass<T>::from_kilograms),
        literal_unit("g", &BasicMass<T>::from_grams),
      },
    .unknown_suffix = "a mass literal needs one of the unit suffixes kg, g",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Mass: `{"kilograms": magnitude}`.
 * @details Reading also accepts the magnitude in `grams` instead. A text format also reads a
 * literal string, e.g. `"12.5kg"`, with the suffix of a C++ literal: `kg`, `g`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicMass<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::mass_reading<T>>::value;
};
