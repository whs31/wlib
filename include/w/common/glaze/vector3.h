#pragma once

#include <array>
#include <bit>

#include <w/common/glaze/detail.h>
#include <w/common/vector3.h>

/**
 * @brief Glaze metadata of Vector3: an array `[x, y, z]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Vector3>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::Vector3& vector,
      w::common::_glaze_impl::ExactArray<w::common::Real, 3> const& xyz
    ) { vector = std::bit_cast<w::common::Vector3>(xyz.values()); },
    [](w::common::Vector3 const& vector) {
      return std::bit_cast<std::array<w::common::Real, 3>>(vector);
    }>;
};
