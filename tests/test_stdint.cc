#include <w/common/stdint.h>

#include <concepts>
#include <cstddef>
#include <cstdint>

#include <catch2/catch_test_macros.hpp>

#ifdef W_DECLARE_INT_LITERAL
#  error W_DECLARE_INT_LITERAL must not leak from stdint.h
#endif

using namespace w;
using namespace w::literals;

TEST_CASE("Fixed-width scalar aliases name their standard types", "[stdint]")
{
  STATIC_CHECK(std::same_as<u8, std::uint8_t>);
  STATIC_CHECK(std::same_as<u16, std::uint16_t>);
  STATIC_CHECK(std::same_as<u32, std::uint32_t>);
  STATIC_CHECK(std::same_as<u64, std::uint64_t>);
  STATIC_CHECK(std::same_as<i8, std::int8_t>);
  STATIC_CHECK(std::same_as<i16, std::int16_t>);
  STATIC_CHECK(std::same_as<i32, std::int32_t>);
  STATIC_CHECK(std::same_as<i64, std::int64_t>);
  STATIC_CHECK(std::same_as<f32, float>);
  STATIC_CHECK(std::same_as<f64, double>);
  STATIC_CHECK(std::same_as<usize, std::size_t>);
  STATIC_CHECK(std::same_as<isize, std::ptrdiff_t>);
  STATIC_CHECK(std::same_as<iptr, std::intptr_t>);
  STATIC_CHECK(std::same_as<uptr, std::uintptr_t>);
  STATIC_CHECK(std::same_as<imax, std::intmax_t>);
  STATIC_CHECK(std::same_as<umax, std::uintmax_t>);
}

TEST_CASE("Integer literals preserve their requested width", "[stdint]")
{
  STATIC_CHECK(std::same_as<decltype(255_u8), u8>);
  STATIC_CHECK(std::same_as<decltype(65'535_u16), u16>);
  STATIC_CHECK(std::same_as<decltype(4'294'967'295_u32), u32>);
  STATIC_CHECK(std::same_as<decltype(18'446'744'073'709'551'615_u64), u64>);
  STATIC_CHECK(std::same_as<decltype(127_i8), i8>);
  STATIC_CHECK(std::same_as<decltype(32'767_i16), i16>);
  STATIC_CHECK(std::same_as<decltype(2'147'483'647_i32), i32>);
  STATIC_CHECK(std::same_as<decltype(9'223'372'036'854'775'807_i64), i64>);
}
