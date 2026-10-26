#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/angular_velocity.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct AngularVelocityFields
  {
    std::optional<T> radians_per_second;
    std::optional<T> degrees_per_second;
    std::optional<T> rpm;
  };

  template <std::floating_point T>
  inline auto constexpr angular_velocity_reading = QuantityReading<
    BasicAngularVelocity<T>,
    AngularVelocityFields<T>,
    3,
    3> {
    .fields =
      {
        field_unit(
          &AngularVelocityFields<T>::radians_per_second,
          &BasicAngularVelocity<T>::from_radians_per_second
        ),
        field_unit(
          &AngularVelocityFields<T>::degrees_per_second,
          &BasicAngularVelocity<T>::from_degrees_per_second
        ),
        field_unit(&AngularVelocityFields<T>::rpm, &BasicAngularVelocity<T>::from_rpm),
      },
    .literals =
      {
        literal_unit("rad_per_s", &BasicAngularVelocity<T>::from_radians_per_second),
        literal_unit("deg_per_s", &BasicAngularVelocity<T>::from_degrees_per_second),
        literal_unit("rpm", &BasicAngularVelocity<T>::from_rpm),
      },
    .unknown_suffix =
      "a angular velocity literal needs one of the unit suffixes rad_per_s, deg_per_s, rpm",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of AngularVelocity: `{"radians_per_second": magnitude}`.
 * @details Reading also accepts the magnitude in `degrees_per_second` or `rpm` instead. A text
 * format also reads a literal string, e.g. `"12.5rad_per_s"`, with the suffix of a C++ literal:
 * `rad_per_s`, `deg_per_s`, `rpm`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicAngularVelocity<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::angular_velocity_reading<T>>::value;
};
