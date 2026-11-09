#pragma once

#include <concepts>
#include <array>

#include <w/common/stdint.h>
#include <w/common/glaze/detail.h>
#include <w/common/matrix3x3.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  using Matrix3x3Rows = std::array<std::array<T, 3>, 3>;

  template <std::floating_point T>
  using Matrix3x3Input = ExactArray<ExactArray<T, 3>, 3>;
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Matrix3x3: an array of three rows, e.g. `[[1, 0, 0], [0, 1, 0], [0, 0,
 * 1]]`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicMatrix3x3<T>>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::BasicMatrix3x3<T>& matrix,
      w::common::_glaze_impl::Matrix3x3Input<T> const& rows
    ) {
      auto const row = [&rows](w::usize const index) {
        auto const [x, y, z] = rows.values().at(index).values();
        return w::common::BasicVector3<T>(x, y, z);
      };
      matrix = w::common::BasicMatrix3x3<T>::from_rows(row(0), row(1), row(2));
    },
    [](w::common::BasicMatrix3x3<T> const& matrix) {
      auto rows = w::common::_glaze_impl::Matrix3x3Rows<T> {};
      for(auto index = 0uz; index < rows.size(); ++index) {
        auto const row = matrix.row(index);
        rows.at(index) = {row.x(), row.y(), row.z()};
      }
      return rows;
    }>;
};
