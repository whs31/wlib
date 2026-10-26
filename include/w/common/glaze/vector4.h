#pragma once

#include <array>

#include <w/common/glaze/detail.h>
#include <w/common/vector4.h>

/**
 * @brief Glaze metadata of Vector4: an array `[x, y, z, w]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Vector4>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::Vector4& vector,
      w::common::_glaze_impl::ExactArray<w::common::Real, 4> const& xyzw
    ) { vector = w::common::Vector4::from_xyzw(xyzw[0], xyzw[1], xyzw[2], xyzw[3]); },
    [](w::common::Vector4 const& vector) { return vector.to_array(); }>;
};
