#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/capacitance.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct CapacitanceFields
  {
    std::optional<Real> farads;
    std::optional<Real> microfarads;
  };

  inline auto constexpr capacitance_reading = QuantityReading<
    Capacitance,
    CapacitanceFields,
    2,
    2> {
    .fields =
      {
        field_unit(&CapacitanceFields::farads, &Capacitance::from_farads),
        field_unit(&CapacitanceFields::microfarads, &Capacitance::from_microfarads),
      },
    .literals =
      {
        literal_unit("f", &Capacitance::from_farads),
        literal_unit("uf", &Capacitance::from_microfarads),
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
template <>
struct glz::meta<w::common::Capacitance>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::capacitance_reading>::value;
};
