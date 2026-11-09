#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>

#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

using iptr = std::intptr_t;
using uptr = std::uintptr_t;

using imax = std::intmax_t;
using umax = std::uintmax_t;

using std::byte;

inline namespace literals
{
  namespace _literals_impl                                 // NOLINT(*-identifier-naming)
  {
    template <typename T>
    consteval T checked_literal(unsigned long long value)  // NOLINT(*-runtime-int)
    {
      if(not std::in_range<T>(value))
        throw "Integer literal out of range";              // NOLINT(*-std-exception-baseclass)
      return static_cast<T>(value);
    }
  }  // namespace _literals_impl

#define W_DECLARE_INT_LITERAL(TYPE)                                   \
  consteval TYPE operator""_## TYPE(unsigned long long const value)   \
  {                                                                   \
    return _literals_impl::checked_literal<TYPE>(value);              \
  }

  W_DECLARE_INT_LITERAL(u8)
  W_DECLARE_INT_LITERAL(u16)
  W_DECLARE_INT_LITERAL(u32)
  W_DECLARE_INT_LITERAL(u64)
  W_DECLARE_INT_LITERAL(i8)
  W_DECLARE_INT_LITERAL(i16)
  W_DECLARE_INT_LITERAL(i32)
  W_DECLARE_INT_LITERAL(i64)

#undef W_DECLARE_INT_LITERAL

}  // namespace literals

W_COMMON_NAMESPACE_END
