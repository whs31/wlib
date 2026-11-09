#pragma once

#include <array>

#include <fmt/format.h>

#include <w/common/stdint.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

namespace _format_impl  // NOLINT(*-identifier-naming)
{
  /**
   * @brief Formats `values` as `(a, b, ...)`, applying the parsed spec of `formatter`, e.g.
   * `{:.3f}`, to every component.
   */
  template <usize N, typename T>
  fmt::format_context::iterator format_components(
    fmt::formatter<T> const& formatter,
    std::array<T, N> const& values,
    fmt::format_context& ctx
  )
  {
    auto out = ctx.out();
    *out++ = '(';
    auto first = true;
    for(auto const value : values) {
      if(not first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
      ctx.advance_to(out);
      out = formatter.format(value, ctx);
    }
    *out++ = ')';
    return out;
  }
}  // namespace _format_impl

W_COMMON_NAMESPACE_END
