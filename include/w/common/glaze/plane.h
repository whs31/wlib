#pragma once

#include <concepts>
#include <array>

#include <w/common/glaze/detail.h>
#include <w/common/plane.h>

/**
 * @brief Glaze metadata of Plane: an array `[a, b, c, d]`, the normal and the distance.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicPlane<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicPlane<T>& plane, w::common::_glaze_impl::ExactArray<T, 4> const& abcd) {
      plane = w::common::BasicPlane<T>({abcd[0], abcd[1], abcd[2]}, abcd[3]);
    },
    [](w::common::BasicPlane<T> const& plane) { return plane.to_vector4().to_array(); }>;
};
