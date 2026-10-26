#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/power.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct PowerFields
  {
    std::optional<T> watts;
    std::optional<T> kilowatts;
  };

  template <std::floating_point T>
  inline auto constexpr power_reading = QuantityReading<BasicPower<T>, PowerFields<T>, 2, 2> {
    .fields =
      {
        field_unit(&PowerFields<T>::watts, &BasicPower<T>::from_watts),
        field_unit(&PowerFields<T>::kilowatts, &BasicPower<T>::from_kilowatts),
      },
    .literals =
      {
        literal_unit("w", &BasicPower<T>::from_watts),
        literal_unit("kw", &BasicPower<T>::from_kilowatts),
      },
    .unknown_suffix = "a power literal needs one of the unit suffixes w, kw",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Power: `{"watts": magnitude}`.
 * @details Reading also accepts the magnitude in `kilowatts` instead. A text format also reads
 * a literal string, e.g. `"12.5w"`, with the suffix of a C++ literal: `w`, `kw`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicPower<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::power_reading<T>>::value;
};
