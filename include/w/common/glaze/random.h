#pragma once

#include <array>
#include <cstdint>

#include <w/common/glaze/detail.h>
#include <w/common/random.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  struct RandomStreamFields
  {
    std::array<std::uint64_t, 4> state {};
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of RandomStream: `{"state": [s0, s1, s2, s3]}`, e.g. for snapshots.
 * @details Reading fails with `constraint_violated` on an all-zero state, which the generator
 * cannot leave.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::RandomStream>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::RandomStream& stream,
      w::common::_glaze_impl::RandomStreamFields const& fields,
      glz::context& ctx
    ) {
      if(fields.state == std::array<std::uint64_t, 4> {}) {
        w::common::_glaze_impl::reject(ctx, "invalid random stream: all-zero state");
        return;
      }
      stream = w::common::RandomStream {.state = fields.state};
    },
    [](w::common::RandomStream const& stream) {
      return w::common::_glaze_impl::RandomStreamFields {.state = stream.state};
    }>;
};
