#pragma once

#include <concepts>
#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/current.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct CurrentFields
  {
    std::optional<T> amperes;
    std::optional<T> milliamperes;
  };

  template <std::floating_point T>
  inline auto constexpr current_reading = QuantityReading<BasicCurrent<T>, CurrentFields<T>, 2, 2> {
    .fields =
      {
        field_unit(&CurrentFields<T>::amperes, &BasicCurrent<T>::from_amperes),
        field_unit(&CurrentFields<T>::milliamperes, &BasicCurrent<T>::from_milliamperes),
      },
    .literals =
      {
        literal_unit("a", &BasicCurrent<T>::from_amperes),
        literal_unit("ma", &BasicCurrent<T>::from_milliamperes),
      },
    .unknown_suffix = "a current literal needs one of the unit suffixes a, ma",
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Current: `{"amperes": magnitude}`.
 * @details Reading also accepts the magnitude in `milliamperes` instead. A text format also
 * reads a literal string, e.g. `"12.5a"`, with the suffix of a C++ literal: `a`, `ma`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicCurrent<T>>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::current_reading<T>>::value;
};
