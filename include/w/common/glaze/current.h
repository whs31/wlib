#pragma once

#include <optional>

#include <w/common/glaze/detail.h>
#include <w/common/current.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct CurrentFields
  {
    std::optional<Real> amperes;
    std::optional<Real> milliamperes;
  };

  inline auto constexpr current_reading = QuantityReading<Current, CurrentFields, 2, 2> {
    .fields =
      {
        field_unit(&CurrentFields::amperes, &Current::from_amperes),
        field_unit(&CurrentFields::milliamperes, &Current::from_milliamperes),
      },
    .literals =
      {
        literal_unit("a", &Current::from_amperes),
        literal_unit("ma", &Current::from_milliamperes),
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
template <>
struct glz::meta<w::common::Current>
{
  static auto constexpr value = w::common::_glaze_impl::QuantityAdapter<
    w::common::_glaze_impl::current_reading>::value;
};
