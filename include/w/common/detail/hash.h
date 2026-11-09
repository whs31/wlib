#pragma once

#include <functional>

#include <w/common/stdint.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

namespace _hash_impl  // NOLINT(*-identifier-naming)
{
  /**
   * @brief Combines the `std::hash` of every value in @p values, in order, boost-style.
   */
  template <typename... T>
  [[nodiscard]] usize combine(T const&... values) noexcept
  {
    auto constexpr golden = 0x9e3779b97f4a7c15uz;
    auto hash = 0uz;
    ((hash ^= std::hash<T> {}(values) + golden + (hash << 6u) + (hash >> 2u)), ...);
    return hash;
  }
}  // namespace _hash_impl

W_COMMON_NAMESPACE_END
