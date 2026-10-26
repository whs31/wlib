#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/velocity.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct VelocityFields
  {
    std::optional<T> mps;
    std::optional<T> kmph;
    std::optional<T> mph;
  };

  template <std::floating_point T>
  inline auto constexpr velocity_reading = QuantityReading<
    BasicVelocity<T>,
    VelocityFields<T>,
    3,
    3> {
    .fields =
      {
        field_unit(&VelocityFields<T>::mps, &BasicVelocity<T>::from_mps),
        field_unit(&VelocityFields<T>::kmph, &BasicVelocity<T>::from_kmph),
        field_unit(&VelocityFields<T>::mph, &BasicVelocity<T>::from_mph),
      },
    .literals =
      {
        literal_unit("mps", &BasicVelocity<T>::from_mps),
        literal_unit("kmph", &BasicVelocity<T>::from_kmph),
        literal_unit("mph", &BasicVelocity<T>::from_mph),
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
template <std::floating_point T>
struct glz::meta<w::common::BasicVelocity<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::velocity_reading<T>>::value;
};
