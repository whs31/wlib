#pragma once

#include <w/common/glaze/detail.h>
#include <w/common/vector2.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct Vector2Fields
  {
    Real x = 0.0;
    Real y = 0.0;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Vector2: `{"x": x, "y": y}`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Vector2>
{
  static auto constexpr value = glz::custom<
    [](w::common::Vector2& vector2, w::common::_glaze_impl::Vector2Fields const& fields) {
      vector2 = w::common::Vector2::from_xy(fields.x, fields.y);
    },
    [](w::common::Vector2 const& vector2) {
      return w::common::_glaze_impl::Vector2Fields {
        .x = vector2.x(),
        .y = vector2.y(),
      };
    }>;
};
