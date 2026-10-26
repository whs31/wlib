#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/power.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct PowerFields
  {
    std::optional<Real> watts;
    std::optional<Real> kilowatts;
  };

  inline auto constexpr power_reading = QuantityReading<Power, PowerFields, 2, 2> {
    .fields =
      {
        field_unit(&PowerFields::watts, &Power::from_watts),
        field_unit(&PowerFields::kilowatts, &Power::from_kilowatts),
      },
    .literals =
      {
        literal_unit("w", &Power::from_watts),
        literal_unit("kw", &Power::from_kilowatts),
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
template <>
struct glz::meta<w::common::Power>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::power_reading>::value;
};
