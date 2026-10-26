#pragma once

#include <concepts>
#include <array>

#include <w/common/glaze/detail.h>
#include <w/common/vector2.h>

/**
 * @brief Glaze metadata of Vector2: an array `[x, y]`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicVector2<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicVector2<T>& vector, w::common::_glaze_impl::ExactArray<T, 2> const& xy) {
      vector = w::common::BasicVector2<T>::from_xy(xy[0], xy[1]);
    },
    [](w::common::BasicVector2<T> const& vector) { return std::array {vector.x(), vector.y()}; }>;
};
