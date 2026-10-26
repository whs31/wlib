#pragma once

#include <array>

#include <w/common/frustum.h>
#include <w/common/glaze/detail.h>
#include <w/common/glaze/plane.h>

/**
 * @brief Glaze metadata of Frustum: an array of its six planes, left, right, bottom, top, near,
 * far, each `[a, b, c, d]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Frustum>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::Frustum& frustum,
      w::common::_glaze_impl::ExactArray<w::common::Plane, 6> const& planes
    ) { frustum = w::common::Frustum(planes.values()); },
    [](w::common::Frustum const& frustum) { return frustum.planes(); }>;
};
