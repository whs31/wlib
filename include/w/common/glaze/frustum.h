#pragma once

#include <concepts>
#include <array>

#include <w/common/frustum.h>
#include <w/common/glaze/detail.h>
#include <w/common/glaze/plane.h>

/**
 * @brief Glaze metadata of Frustum: an array of its six planes, left, right, bottom, top, near,
 * far, each `[a, b, c, d]`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicFrustum<T>>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::BasicFrustum<T>& frustum,
      w::common::_glaze_impl::ExactArray<w::common::BasicPlane<T>, 6> const& planes
    ) { frustum = w::common::BasicFrustum<T>(planes.values()); },
    [](w::common::BasicFrustum<T> const& frustum) { return frustum.planes(); }>;
};
