#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <algorithm>
#include <array>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

#include <w/common/bytes.h>
#include <w/common/error.h>
#include <w/common/stdint.h>

using namespace w;

namespace
{
  enum class Kind : u8
  {
    Box = 1,
    Sphere = 2,
  };

  struct Label
  {
    explicit Label(u16 const value)
      : id(value)
    {}

    u16 id;
  };
}  // namespace

TEST_CASE("Values and byte spans read back unchanged", "[bytes]")
{
  auto out = ByteWriter();
  auto const tail = std::array {std::byte {0xAA}, std::byte {0xBB}};
  out.write(u32 {7}).write(Kind::Sphere).write(true).write(-1.5F).append(tail);

  auto const bytes = std::move(out).finish();
  auto in = ByteReader(bytes);

  CHECK(in.read<u32>() == 7);
  CHECK(in.read<Kind>() == Kind::Sphere);
  CHECK(in.read<bool>());
  CHECK(in.read<float>() == -1.5F);
  CHECK(std::ranges::equal(in.take(tail.size()), tail));
  CHECK(in.remaining() == 0);
  CHECK_NOTHROW(in.expect_end());
}

TEST_CASE("A value without a default constructor reads back", "[bytes]")
{
  STATIC_CHECK(std::is_trivially_copyable_v<Label>);
  STATIC_CHECK_FALSE(std::is_default_constructible_v<Label>);

  auto out = ByteWriter();
  out.write(Label(42));
  auto const bytes = std::move(out).finish();

  CHECK(ByteReader(bytes).read<Label>().id == 42);
}

TEST_CASE("Writer and reader align to the same byte offset", "[bytes]")
{
  auto out = ByteWriter();
  out.write(u8 {1}).align<4>().write(u16 {0x0203});
  auto const bytes = std::move(out).finish();

  REQUIRE(bytes.size() == 6);
  CHECK(bytes.at(1) == std::byte {0});
  CHECK(bytes.at(2) == std::byte {0});
  CHECK(bytes.at(3) == std::byte {0});

  auto in = ByteReader(bytes);
  CHECK(in.read<u8>() == 1);
  in.align<4>();
  CHECK(in.read<u16>() == 0x0203);
  CHECK_NOTHROW(in.expect_end());
}

TEST_CASE("A reader refuses a read past the end", "[bytes][error]")
{
  auto const bytes = std::array {std::byte {1}, std::byte {2}, std::byte {3}};
  auto in = ByteReader(bytes);

  CHECK_THROWS_AS(in.read<u32>(), ByteReadError);
  CHECK_THROWS_WITH(
    in.read<u32>(),
    Catch::Matchers::ContainsSubstring("input ends early: 4 bytes needed, 3 left")
  );
  CHECK(in.remaining() == bytes.size());
}

TEST_CASE("A reader refuses alignment past the end", "[bytes][error]")
{
  auto const bytes = std::array {std::byte {1}, std::byte {0}};
  auto in = ByteReader(bytes);
  CHECK(in.read<u8>() == 1);

  CHECK_THROWS_AS(in.align<4>(), ByteReadError);
  CHECK(in.remaining() == 1);
}

TEST_CASE("A reader refuses trailing bytes", "[bytes][error]")
{
  auto const bytes = std::array {std::byte {1}, std::byte {2}};
  auto in = ByteReader(bytes);
  CHECK(in.read<u8>() == 1);

  CHECK_THROWS_AS(in.expect_end(), ByteReadError);
  CHECK_THROWS_WITH(
    in.expect_end(),
    Catch::Matchers::ContainsSubstring("input has 1 trailing bytes")
  );
}

TEST_CASE("A bool is read only from zero or one", "[bytes][error]")
{
  auto const bytes = std::array {std::byte {0}, std::byte {1}, std::byte {2}};
  auto in = ByteReader(bytes);

  CHECK_FALSE(in.read<bool>());
  CHECK(in.read<bool>());
  CHECK_THROWS_WITH(
    in.read<bool>(),
    Catch::Matchers::ContainsSubstring("a bool is stored as the byte 2, not 0 or 1")
  );
}

TEST_CASE("A byte read error is a library error", "[bytes][error]")
{
  STATIC_CHECK(std::is_base_of_v<Error, ByteReadError>);

  auto in = ByteReader(std::span<std::byte const>());
  try {
    static_cast<void>(in.read<u8>());
    FAIL("the read must throw");
  } catch(Error const& error) {
    CHECK(std::string_view(error.what()) == "input ends early: 1 bytes needed, 0 left");
  }
}
