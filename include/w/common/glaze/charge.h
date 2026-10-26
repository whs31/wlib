#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/charge.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct ChargeFields
  {
    std::optional<T> coulombs;
    std::optional<T> ampere_hours;
    std::optional<T> milliampere_hours;
  };

  template <std::floating_point T>
  inline auto constexpr charge_reading = QuantityReading<BasicCharge<T>, ChargeFields<T>, 3, 2> {
    .fields =
      {
        field_unit(&ChargeFields<T>::coulombs, &BasicCharge<T>::from_coulombs),
        field_unit(&ChargeFields<T>::ampere_hours, &BasicCharge<T>::from_ampere_hours),
        field_unit(&ChargeFields<T>::milliampere_hours, &BasicCharge<T>::from_milliampere_hours),
      },
    .literals =
      {
        literal_unit("ah", &BasicCharge<T>::from_ampere_hours),
        literal_unit("mah", &BasicCharge<T>::from_milliampere_hours),
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
template <std::floating_point T>
struct glz::meta<w::common::BasicCharge<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::charge_reading<T>>::value;
};
