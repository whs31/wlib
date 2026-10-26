#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/pressure.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct PressureFields
  {
    std::optional<T> pascals;
    std::optional<T> hectopascals;
    std::optional<T> kilopascals;
    std::optional<T> millibars;
    std::optional<T> inches_of_mercury;
  };

  template <std::floating_point T>
  inline auto constexpr pressure_reading = QuantityReading<
    BasicPressure<T>,
    PressureFields<T>,
    5,
    5> {
    .fields =
      {
        field_unit(&PressureFields<T>::pascals, &BasicPressure<T>::from_pascals),
        field_unit(&PressureFields<T>::hectopascals, &BasicPressure<T>::from_hectopascals),
        field_unit(&PressureFields<T>::kilopascals, &BasicPressure<T>::from_kilopascals),
        field_unit(&PressureFields<T>::millibars, &BasicPressure<T>::from_millibars),
        field_unit(
          &PressureFields<T>::inches_of_mercury,
          &BasicPressure<T>::from_inches_of_mercury
        ),
      },
    .literals =
      {
        literal_unit("pa", &BasicPressure<T>::from_pascals),
        literal_unit("hpa", &BasicPressure<T>::from_hectopascals),
        literal_unit("kpa", &BasicPressure<T>::from_kilopascals),
        literal_unit("mbar", &BasicPressure<T>::from_millibars),
        literal_unit("inhg", &BasicPressure<T>::from_inches_of_mercury),
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
template <std::floating_point T>
struct glz::meta<w::common::BasicPressure<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::pressure_reading<T>>::value;
};
