#pragma once

#include <array>

#include <w/common/glaze/detail.h>
#include <w/common/vector2.h>

/**
 * @brief Glaze metadata of Vector2: an array `[x, y]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Vector2>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::Vector2& vector,
      w::common::_glaze_impl::ExactArray<w::common::Real, 2> const& xy
    ) { vector = w::common::Vector2::from_xy(xy[0], xy[1]); },
    [](w::common::Vector2 const& vector) { return std::array {vector.x(), vector.y()}; }>;
};
