#pragma once

#include <concepts>
#include <array>
#include <bit>

#include <w/common/glaze/detail.h>
#include <w/common/matrix4x4.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  using Matrix4x4Rows = std::array<std::array<T, 4>, 4>;
  template <std::floating_point T>
  using Matrix4x4Input = ExactArray<ExactArray<T, 4>, 4>;
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Matrix4x4: an array of four rows, e.g. `[[1, 0, 0, 2], [0, 1, 0, 0],
 * [0, 0, 1, 0], [0, 0, 0, 1]]` for a translation by 2 along x.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicMatrix4x4<T>>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::BasicMatrix4x4<T>& matrix,
      w::common::_glaze_impl::Matrix4x4Input<T> const& rows
    ) { matrix = std::bit_cast<w::common::BasicMatrix4x4<T>>(rows.values()); },
    [](w::common::BasicMatrix4x4<T> const& matrix) {
      return std::bit_cast<w::common::_glaze_impl::Matrix4x4Rows<T>>(matrix);
    }>;
};
