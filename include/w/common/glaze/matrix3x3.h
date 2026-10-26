#pragma once

#include <array>
#include <cstddef>

#include <w/common/glaze/detail.h>
#include <w/common/matrix3x3.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  using Matrix3x3Rows = std::array<std::array<Real, 3>, 3>;
  using Matrix3x3Input = ExactArray<ExactArray<Real, 3>, 3>;
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Matrix3x3: an array of three rows, e.g. `[[1, 0, 0], [0, 1, 0], [0, 0,
 * 1]]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Matrix3x3>
{
  static auto constexpr value = glz::custom<
    [](w::common::Matrix3x3& matrix, w::common::_glaze_impl::Matrix3x3Input const& rows) {
      auto const row = [&rows](std::size_t const index) {
        auto const [x, y, z] = rows.values().at(index).values();
        return w::common::Vector3(x, y, z);
      };
      matrix = w::common::Matrix3x3::from_rows(row(0), row(1), row(2));
    },
    [](w::common::Matrix3x3 const& matrix) {
      auto rows = w::common::_glaze_impl::Matrix3x3Rows {};
      for(auto index = std::size_t {0}; index < rows.size(); ++index) {
        auto const row = matrix.row(index);
        rows.at(index) = {row.x(), row.y(), row.z()};
      }
      return rows;
    }>;
};
