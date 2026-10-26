#pragma once

#include <concepts>
#include <array>
#include <bit>

#include <w/common/glaze/detail.h>
#include <w/common/vector3.h>

/**
 * @brief Glaze metadata of Vector3: an array `[x, y, z]`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicVector3<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicVector3<T>& vector, w::common::_glaze_impl::ExactArray<T, 3> const& xyz) {
      vector = std::bit_cast<w::common::BasicVector3<T>>(xyz.values());
    },
    [](w::common::BasicVector3<T> const& vector) {
      return std::bit_cast<std::array<T, 3>>(vector);
    }>;
};
