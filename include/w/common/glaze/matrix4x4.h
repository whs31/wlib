#pragma once

#include <array>
#include <bit>

#include <w/common/glaze/detail.h>
#include <w/common/matrix4x4.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  using Matrix4x4Rows = std::array<std::array<Real, 4>, 4>;
  using Matrix4x4Input = ExactArray<ExactArray<Real, 4>, 4>;
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Matrix4x4: an array of four rows, e.g. `[[1, 0, 0, 2], [0, 1, 0, 0],
 * [0, 0, 1, 0], [0, 0, 0, 1]]` for a translation by 2 along x.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Matrix4x4>
{
  static auto constexpr value = glz::custom<
    [](w::common::Matrix4x4& matrix, w::common::_glaze_impl::Matrix4x4Input const& rows) {
      matrix = std::bit_cast<w::common::Matrix4x4>(rows.values());
    },
    [](w::common::Matrix4x4 const& matrix) {
      return std::bit_cast<w::common::_glaze_impl::Matrix4x4Rows>(matrix);
    }>;
};
