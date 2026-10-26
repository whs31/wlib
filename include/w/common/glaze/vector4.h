#pragma once

#include <concepts>
#include <array>

#include <w/common/glaze/detail.h>
#include <w/common/vector4.h>

/**
 * @brief Glaze metadata of Vector4: an array `[x, y, z, w]`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicVector4<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicVector4<T>& vector, w::common::_glaze_impl::ExactArray<T, 4> const& xyzw) {
      vector = w::common::BasicVector4<T>::from_xyzw(xyzw[0], xyzw[1], xyzw[2], xyzw[3]);
    },
    [](w::common::BasicVector4<T> const& vector) { return vector.to_array(); }>;
};
