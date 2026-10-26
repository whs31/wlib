#pragma once

#include <w/common/glaze/detail.h>
#include <w/common/point2.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct Point2Fields
  {
    Real x = 0.0;
    Real y = 0.0;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Point2: `{"x": x, "y": y}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Point2>
{
  static auto constexpr value = glz::custom<
    [](w::common::Point2& point2, w::common::_glaze_impl::Point2Fields const& fields) {
      point2 = w::common::Point2::from_xy(fields.x, fields.y);
    },
    [](w::common::Point2 const& point2) {
      return w::common::_glaze_impl::Point2Fields {
        .x = point2.x(),
        .y = point2.y(),
      };
    }>;
};
