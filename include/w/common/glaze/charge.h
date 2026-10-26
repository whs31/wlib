#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/charge.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct ChargeFields
  {
    std::optional<Real> coulombs;
    std::optional<Real> ampere_hours;
    std::optional<Real> milliampere_hours;
  };

  inline auto constexpr charge_reading = QuantityReading<Charge, ChargeFields, 3, 2> {
    .fields =
      {
        field_unit(&ChargeFields::coulombs, &Charge::from_coulombs),
        field_unit(&ChargeFields::ampere_hours, &Charge::from_ampere_hours),
        field_unit(&ChargeFields::milliampere_hours, &Charge::from_milliampere_hours),
      },
    .literals =
      {
        literal_unit("ah", &Charge::from_ampere_hours),
        literal_unit("mah", &Charge::from_milliampere_hours),
      },
    .unknown_suffix = "a charge literal needs one of the unit suffixes ah, mah",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Charge: `{"coulombs": magnitude}`.
 * @details Reading also accepts the magnitude in `ampere_hours` or `milliampere_hours` instead.
 * A text format also reads a literal string, e.g. `"12.5ah"`, with the suffix of a C++ literal:
 * `ah`, `mah`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Charge>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::charge_reading>::value;
};
