#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/capacitance.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct CapacitanceFields
  {
    std::optional<T> farads;
    std::optional<T> microfarads;
  };

  template <std::floating_point T>
  inline auto constexpr capacitance_reading = QuantityReading<
    BasicCapacitance<T>,
    CapacitanceFields<T>,
    2,
    2> {
    .fields =
      {
        field_unit(&CapacitanceFields<T>::farads, &BasicCapacitance<T>::from_farads),
        field_unit(&CapacitanceFields<T>::microfarads, &BasicCapacitance<T>::from_microfarads),
      },
    .literals =
      {
        literal_unit("f", &BasicCapacitance<T>::from_farads),
        literal_unit("uf", &BasicCapacitance<T>::from_microfarads),
      },
    .unknown_suffix = "a capacitance literal needs one of the unit suffixes f, uf",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Capacitance: `{"farads": magnitude}`.
 * @details Reading also accepts the magnitude in `microfarads` instead. A text format also
 * reads a literal string, e.g. `"12.5f"`, with the suffix of a C++ literal: `f`, `uf`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicCapacitance<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::capacitance_reading<T>>::value;
};
