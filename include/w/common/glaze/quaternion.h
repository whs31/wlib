#pragma once

#include <array>
#include <bit>

#include <w/common/glaze/detail.h>
#include <w/common/quaternion.h>

/**
 * @brief Glaze metadata of Quaternion: an array `[w, x, y, z]`, scalar first as it is stored.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Quaternion>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::Quaternion& quaternion,
      w::common::_glaze_impl::ExactArray<w::common::Real, 4> const& wxyz
    ) { quaternion = std::bit_cast<w::common::Quaternion>(wxyz.values()); },
    [](w::common::Quaternion const& quaternion) {
      return std::bit_cast<std::array<w::common::Real, 4>>(quaternion);
    }>;
};
