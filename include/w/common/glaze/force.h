#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/force.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct ForceFields
  {
    std::optional<T> newtons;
    std::optional<T> kilonewtons;
    std::optional<T> kilograms_force;
  };

  template <std::floating_point T>
  inline auto constexpr force_reading = QuantityReading<BasicForce<T>, ForceFields<T>, 3, 3> {
    .fields =
      {
        field_unit(&ForceFields<T>::newtons, &BasicForce<T>::from_newtons),
        field_unit(&ForceFields<T>::kilonewtons, &BasicForce<T>::from_kilonewtons),
        field_unit(&ForceFields<T>::kilograms_force, &BasicForce<T>::from_kilograms_force),
      },
    .literals =
      {
        literal_unit("n", &BasicForce<T>::from_newtons),
        literal_unit("kn", &BasicForce<T>::from_kilonewtons),
        literal_unit("kgf", &BasicForce<T>::from_kilograms_force),
      },
    .unknown_suffix = "a force literal needs one of the unit suffixes n, kn, kgf",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Force: `{"newtons": magnitude}`.
 * @details Reading also accepts the magnitude in `kilonewtons` or `kilograms_force` instead. A
 * text format also reads a literal string, e.g. `"12.5n"`, with the suffix of a C++ literal:
 * `n`, `kn`, `kgf`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicForce<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::force_reading<T>>::value;
};
