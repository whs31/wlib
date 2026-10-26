#pragma once

#include <array>

#include <w/common/glaze/detail.h>
#include <w/common/plane.h>

/**
 * @brief Glaze metadata of Plane: an array `[a, b, c, d]`, the normal and the distance.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Plane>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::Plane& plane,
      w::common::_glaze_impl::ExactArray<w::common::Real, 4> const& abcd
    ) { plane = w::common::Plane({abcd[0], abcd[1], abcd[2]}, abcd[3]); },
    [](w::common::Plane const& plane) { return plane.to_vector4().to_array(); }>;
};
