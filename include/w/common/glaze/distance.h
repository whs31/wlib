#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/distance.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct DistanceFields
  {
    std::optional<T> meters;
    std::optional<T> nanometers;
    std::optional<T> micrometers;
    std::optional<T> millimeters;
    std::optional<T> centimeters;
    std::optional<T> decimeters;
    std::optional<T> hectometers;
    std::optional<T> kilometers;
    std::optional<T> inches;
    std::optional<T> feet;
    std::optional<T> yards;
    std::optional<T> furlongs;
    std::optional<T> miles;
  };

  template <std::floating_point T>
  inline auto constexpr distance_reading = QuantityReading<
    BasicDistance<T>,
    DistanceFields<T>,
    13,
    6> {
    .fields =
      {
        field_unit(&DistanceFields<T>::meters, &BasicDistance<T>::from_meters),
        field_unit(&DistanceFields<T>::nanometers, &BasicDistance<T>::from_nanometers),
        field_unit(&DistanceFields<T>::micrometers, &BasicDistance<T>::from_micrometers),
        field_unit(&DistanceFields<T>::millimeters, &BasicDistance<T>::from_millimeters),
        field_unit(&DistanceFields<T>::centimeters, &BasicDistance<T>::from_centimeters),
        field_unit(&DistanceFields<T>::decimeters, &BasicDistance<T>::from_decimeters),
        field_unit(&DistanceFields<T>::hectometers, &BasicDistance<T>::from_hectometers),
        field_unit(&DistanceFields<T>::kilometers, &BasicDistance<T>::from_kilometers),
        field_unit(&DistanceFields<T>::inches, &BasicDistance<T>::from_inches),
        field_unit(&DistanceFields<T>::feet, &BasicDistance<T>::from_feet),
        field_unit(&DistanceFields<T>::yards, &BasicDistance<T>::from_yards),
        field_unit(&DistanceFields<T>::furlongs, &BasicDistance<T>::from_furlongs),
        field_unit(&DistanceFields<T>::miles, &BasicDistance<T>::from_miles),
      },
    .literals =
      {
        literal_unit("meters", &BasicDistance<T>::from_meters),
        literal_unit("kilometers", &BasicDistance<T>::from_kilometers),
        literal_unit("m", &BasicDistance<T>::from_meters),
        literal_unit("km", &BasicDistance<T>::from_kilometers),
        literal_unit("cm", &BasicDistance<T>::from_centimeters),
        literal_unit("mm", &BasicDistance<T>::from_millimeters),
      },
    .unknown_suffix =
      "a distance literal needs one of the unit suffixes meters, kilometers, m, km, cm, mm",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Distance: `{"meters": magnitude}`.
 * @details Reading also accepts the magnitude in `nanometers`, `micrometers`, `millimeters`,
 * `centimeters`, `decimeters`, `hectometers`, `kilometers`, `inches`, `feet`, `yards`,
 * `furlongs` or `miles` instead. A text format also reads a literal string, e.g.
 * `"12.5meters"`, with the suffix of a C++ literal: `meters`, `kilometers`, `m`, `km`, `cm`,
 * `mm`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicDistance<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::distance_reading<T>>::value;
};
