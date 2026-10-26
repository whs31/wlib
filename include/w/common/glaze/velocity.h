#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/velocity.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct VelocityFields
  {
    std::optional<Real> mps;
    std::optional<Real> kmph;
    std::optional<Real> mph;
  };

  inline auto constexpr velocity_reading = QuantityReading<Velocity, VelocityFields, 3, 3> {
    .fields =
      {
        field_unit(&VelocityFields::mps, &Velocity::from_mps),
        field_unit(&VelocityFields::kmph, &Velocity::from_kmph),
        field_unit(&VelocityFields::mph, &Velocity::from_mph),
      },
    .literals =
      {
        literal_unit("mps", &Velocity::from_mps),
        literal_unit("kmph", &Velocity::from_kmph),
        literal_unit("mph", &Velocity::from_mph),
      },
    .unknown_suffix = "a velocity literal needs one of the unit suffixes mps, kmph, mph",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Velocity: `{"mps": magnitude}`.
 * @details Reading also accepts the magnitude in `kmph` or `mph` instead. A text format also
 * reads a literal string, e.g. `"12.5mps"`, with the suffix of a C++ literal: `mps`, `kmph`,
 * `mph`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Velocity>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::velocity_reading>::value;
};
