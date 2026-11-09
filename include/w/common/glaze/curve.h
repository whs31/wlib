#pragma once

#include <vector>

#include <w/common/stdint.h>
#include <w/common/curve.h>
#include <w/common/error.h>
#include <w/common/glaze/detail.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <CurveAxis X, CurveAxis Y>
  struct CurveFields
  {
    std::vector<X> arguments;
    std::vector<Y> values;
  };
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Curve: `{"arguments": [...], "values": [...]}`.
 * @details Reading validates the points and fails with `constraint_violated` if they do not form a
 * valid curve. Quantity axes need their own glaze adapters, e.g. `w/common/glaze/angle.h`.
 * @ingroup core_glaze
 */
template <w::common::CurveAxis X, w::common::CurveAxis Y, w::usize MaxPoints>
  requires(MaxPoints > 0)
struct glz::meta<w::common::Curve<X, Y, MaxPoints>>
{
  using Curve = w::common::Curve<X, Y, MaxPoints>;
  using Fields = w::common::_glaze_impl::CurveFields<X, Y>;

  static auto constexpr value = glz::custom<
    [](Curve& curve, Fields const& fields, glz::context& ctx) {
      try {
        curve = Curve(fields.arguments, fields.values);
      } catch(w::common::Error const&) {
        w::common::_glaze_impl::reject(ctx, "invalid curve points");
      }
    },
    [](Curve const& curve) {
      return Fields {
        .arguments = std::vector<X>(curve.arguments().begin(), curve.arguments().end()),
        .values = std::vector<Y>(curve.values().begin(), curve.values().end()),
      };
    }>;
};
