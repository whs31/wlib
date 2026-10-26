#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/conductance.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct ConductanceFields
  {
    std::optional<T> siemens;
  };

  template <std::floating_point T>
  inline auto constexpr conductance_reading = QuantityReading<
    BasicConductance<T>,
    ConductanceFields<T>,
    1,
    1> {
    .fields =
      {
        field_unit(&ConductanceFields<T>::siemens, &BasicConductance<T>::from_siemens),
      },
    .literals =
      {
        literal_unit("siemens", &BasicConductance<T>::from_siemens),
      },
    .unknown_suffix = "a conductance literal needs one of the unit suffixes siemens",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Conductance: `{"siemens": magnitude}`.
 * @details A text format also reads a literal string, e.g. `"12.5siemens"`, with the suffix of
 * a C++ literal: `siemens`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicConductance<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::conductance_reading<T>>::value;
};
