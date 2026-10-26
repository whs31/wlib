#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/resistance.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct ResistanceFields
  {
    std::optional<T> ohms;
    std::optional<T> milliohms;
  };

  template <std::floating_point T>
  inline auto constexpr resistance_reading = QuantityReading<
    BasicResistance<T>,
    ResistanceFields<T>,
    2,
    2> {
    .fields =
      {
        field_unit(&ResistanceFields<T>::ohms, &BasicResistance<T>::from_ohms),
        field_unit(&ResistanceFields<T>::milliohms, &BasicResistance<T>::from_milliohms),
      },
    .literals =
      {
        literal_unit("ohm", &BasicResistance<T>::from_ohms),
        literal_unit("mohm", &BasicResistance<T>::from_milliohms),
      },
    .unknown_suffix = "a resistance literal needs one of the unit suffixes ohm, mohm",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Resistance: `{"ohms": magnitude}`.
 * @details Reading also accepts the magnitude in `milliohms` instead. A text format also reads
 * a literal string, e.g. `"12.5ohm"`, with the suffix of a C++ literal: `ohm`, `mohm`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicResistance<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::resistance_reading<T>>::value;
};
