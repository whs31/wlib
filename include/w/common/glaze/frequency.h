#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/frequency.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct FrequencyFields
  {
    std::optional<T> hertz;
    std::optional<T> kilohertz;
  };

  template <std::floating_point T>
  inline auto constexpr frequency_reading = QuantityReading<
    BasicFrequency<T>,
    FrequencyFields<T>,
    2,
    2> {
    .fields =
      {
        field_unit(&FrequencyFields<T>::hertz, &BasicFrequency<T>::from_hertz),
        field_unit(&FrequencyFields<T>::kilohertz, &BasicFrequency<T>::from_kilohertz),
      },
    .literals =
      {
        literal_unit("hz", &BasicFrequency<T>::from_hertz),
        literal_unit("khz", &BasicFrequency<T>::from_kilohertz),
      },
    .unknown_suffix = "a frequency literal needs one of the unit suffixes hz, khz",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Frequency: `{"hertz": magnitude}`.
 * @details Reading also accepts the magnitude in `kilohertz` instead. A text format also reads
 * a literal string, e.g. `"12.5hz"`, with the suffix of a C++ literal: `hz`, `khz`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicFrequency<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::frequency_reading<T>>::value;
};
