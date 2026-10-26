#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/resistance.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct ResistanceFields
  {
    std::optional<Real> ohms;
    std::optional<Real> milliohms;
  };

  inline auto constexpr resistance_reading = QuantityReading<Resistance, ResistanceFields, 2, 2> {
    .fields =
      {
        field_unit(&ResistanceFields::ohms, &Resistance::from_ohms),
        field_unit(&ResistanceFields::milliohms, &Resistance::from_milliohms),
      },
    .literals =
      {
        literal_unit("ohm", &Resistance::from_ohms),
        literal_unit("mohm", &Resistance::from_milliohms),
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
template <>
struct glz::meta<w::common::Resistance>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::resistance_reading>::value;
};
