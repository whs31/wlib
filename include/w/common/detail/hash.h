#pragma once

#include <cstddef>
#include <functional>

#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

namespace _hash_impl  // NOLINT(*-identifier-naming)
{
  /**
   * @brief Combines the `std::hash` of every value in @p values, in order, boost-style.
   */
  template <typename... T>
  [[nodiscard]] std::size_t combine(T const&... values) noexcept
  {
    auto constexpr golden = static_cast<std::size_t>(0x9e3779b97f4a7c15ULL);
    auto hash = std::size_t {0};
    ((hash ^= std::hash<T> {}(values) + golden + (hash << 6U) + (hash >> 2U)), ...);
    return hash;
  }
}  // namespace _hash_impl

W_COMMON_NAMESPACE_END
