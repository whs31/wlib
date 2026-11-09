#pragma once

#include <bit>

#include <w/common/stdint.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Returns the bit representation of a trivially copyable value.
 * @tparam T Value type, whose size must match a supported unsigned integer type.
 * @param value Value to inspect.
 * @return Unsigned integer containing the object's bit representation.
 * @ingroup core_math
 */
template <typename T>
  requires(
    std::is_trivially_copyable_v<T>
    and (sizeof(T) == sizeof(u8) or sizeof(T) == sizeof(u16) or sizeof(T) == sizeof(u32) or sizeof(T) == sizeof(u64))
  )
[[nodiscard]] constexpr auto bits_of(T const value) noexcept
{
  if constexpr(sizeof(T) == sizeof(u8))
    return std::bit_cast<u8>(value);
  else if constexpr(sizeof(T) == sizeof(u16))
    return std::bit_cast<u16>(value);
  else if constexpr(sizeof(T) == sizeof(u32))
    return std::bit_cast<u32>(value);
  else
    return std::bit_cast<u64>(value);
}

W_COMMON_NAMESPACE_END
