#pragma once

#include <concepts>
#include <array>
#include <bit>

#include <w/common/glaze/detail.h>
#include <w/common/quaternion.h>

/**
 * @brief Glaze metadata of Quaternion: an array `[w, x, y, z]`, scalar first as it is stored.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicQuaternion<T>>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::BasicQuaternion<T>& quaternion,
      w::common::_glaze_impl::ExactArray<T, 4> const& wxyz
    ) { quaternion = std::bit_cast<w::common::BasicQuaternion<T>>(wxyz.values()); },
    [](w::common::BasicQuaternion<T> const& quaternion) {
      return std::bit_cast<std::array<T, 4>>(quaternion);
    }>;
};
