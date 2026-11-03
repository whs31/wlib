#include <w/common/bytes.h>
#include <w/common/error.h>
#include <w/common/vector3.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <array>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using namespace w;

namespace
{
  enum class Kind : std::uint8_t
  {
    Box = 1,
    Sphere = 2,
  };

  // note: [whs31 : 2026-11-03]
  // trivially copyable, and with no default constructor: a value of it is read all the same
  struct Label
  {
    explicit Label(std::uint16_t const value)
      : id(value)
    {}

    std::uint16_t id;
  };

  template <typename T>
  [[nodiscard]] std::string bytes_of(T const& value)
  {
    auto bytes = std::string(sizeof(T), '\0');
    std::memcpy(bytes.data(), &value, sizeof(T));
    return bytes;
  }

  // a record as an asset file keeps one: values, counted vectors and text of every kind
  [[nodiscard]] std::string record()
  {
    auto out = ByteWriter();
    out.value(std::array {'W', 'R', 'E', 'C'});
    out.value(std::uint32_t {7});
    out.value(Kind::Sphere);
    out.value(true);
    out.value(-1.5F);
    out.value(Vector3(1.0F, 2.0F, 3.0F));
    out.text("wheel");
    out.text("");
    out.text(std::string_view("a\0b", 3));
    out.values(std::vector<Vector3> {{0.0F, 1.0F, 2.0F}, {3.0F, 4.0F, 5.0F}});
    out.values(std::vector<std::uint8_t> {});
    out.values(std::vector<std::uint32_t> {10, 20, 30});
    return out.release();
  }

  void read_record(ByteReader& in)
  {
    static_cast<void>(in.value<std::array<char, 4>>());
    static_cast<void>(in.value<std::uint32_t>());
    static_cast<void>(in.value<Kind>());
    static_cast<void>(in.value<bool>());
    static_cast<void>(in.value<float>());
    static_cast<void>(in.value<Vector3>());
    static_cast<void>(in.text());
    static_cast<void>(in.text());
    static_cast<void>(in.text());
    static_cast<void>(in.values<Vector3>());
    static_cast<void>(in.values<std::uint8_t>());
    static_cast<void>(in.values<std::uint32_t>());
  }
}  // namespace

TEST_CASE("Written values, vectors and text read back unchanged", "[bytes]")
{
  auto const bytes = record();
  auto in = ByteReader(bytes);

  CHECK(in.value<std::array<char, 4>>() == std::array {'W', 'R', 'E', 'C'});
  CHECK(in.value<std::uint32_t>() == 7);
  CHECK(in.value<Kind>() == Kind::Sphere);
  CHECK(in.value<bool>());
  CHECK(in.value<float>() == -1.5F);
  CHECK(in.value<Vector3>() == Vector3(1.0F, 2.0F, 3.0F));
  CHECK(in.text() == "wheel");
  CHECK(in.text().empty());
  CHECK(in.text() == std::string_view("a\0b", 3));
  CHECK(in.values<Vector3>() == std::vector<Vector3> {{0.0F, 1.0F, 2.0F}, {3.0F, 4.0F, 5.0F}});
  CHECK(in.values<std::uint8_t>().empty());
  CHECK(in.values<std::uint32_t>() == std::vector<std::uint32_t> {10, 20, 30});

  CHECK(in.done());
  CHECK(in.remaining() == 0);
}

TEST_CASE("A value is written as it is in memory, a vector after its count", "[bytes]")
{
  auto out = ByteWriter();
  CHECK(out.size() == 0);
  CHECK(out.data().empty());

  out.value(std::uint32_t {0x0102'0304});
  CHECK(out.data() == bytes_of(std::uint32_t {0x0102'0304}));

  out.values(std::vector<float> {1.0F, 2.0F});
  CHECK(out.size() == 2 * sizeof(std::uint32_t) + 2 * sizeof(float));
  CHECK(
    out.data()
    == bytes_of(std::uint32_t {0x0102'0304}) + bytes_of(std::uint32_t {2}) + bytes_of(1.0F)
         + bytes_of(2.0F)
  );

  out.text("ab");
  CHECK(out.data().ends_with(bytes_of(std::uint32_t {2}) + "ab"));
}

TEST_CASE("Vectors are written from any contiguous range of values", "[bytes]")
{
  auto const items = std::vector<std::int16_t> {-1, 2, 300};
  auto from_vector = ByteWriter();
  from_vector.values(items);

  auto from_array = ByteWriter();
  from_array.values(std::array<std::int16_t, 3> {-1, 2, 300});
  auto from_span = ByteWriter();
  from_span.values(std::span<std::int16_t const>(items));

  CHECK(from_array.data() == from_vector.data());
  CHECK(from_span.data() == from_vector.data());
  CHECK(ByteReader(from_vector.data()).values<std::int16_t>() == items);
}

TEST_CASE("A value of a type without a default constructor reads back", "[bytes]")
{
  auto out = ByteWriter();
  out.value(Label(42));
  auto const bytes = out.release();
  CHECK(ByteReader(bytes).value<Label>().id == 42);
}

TEST_CASE("Releasing the writer hands over its bytes and starts again", "[bytes]")
{
  auto out = ByteWriter();
  out.text("first");
  auto const first = out.release();
  CHECK_FALSE(first.empty());
  CHECK(out.size() == 0);

  out.text("second");
  CHECK(ByteReader(first).text() == "first");
  CHECK(ByteReader(out.data()).text() == "second");
}

TEST_CASE("A reader of a part of the bytes reads that part", "[bytes]")
{
  auto out = ByteWriter();
  out.value(std::uint8_t {1});
  out.value(std::uint8_t {2});
  auto const bytes = out.release();

  auto in = ByteReader(std::string_view(bytes).substr(1));
  CHECK(in.remaining() == 1);
  CHECK_FALSE(in.done());
  CHECK(in.value<std::uint8_t>() == 2);
  CHECK(in.done());
}

TEST_CASE("A reader refuses a read past the end of its input", "[bytes]")
{
  SECTION("of a value")
  {
    auto const bytes = std::string(3, '\x01');
    auto in = ByteReader(bytes);
    CHECK_THROWS_AS(in.value<std::uint32_t>(), ByteReadError);
    CHECK_THROWS_WITH(
      in.value<std::uint32_t>(),
      Catch::Matchers::ContainsSubstring("input ends early: 4 bytes needed, 3 left")
    );
  }

  SECTION("of no bytes at all")
  {
    auto in = ByteReader(std::string_view());
    CHECK(in.done());
    CHECK_THROWS_AS(in.value<std::uint8_t>(), ByteReadError);
    CHECK_THROWS_AS(in.text(), ByteReadError);
    CHECK_THROWS_AS(in.count(), ByteReadError);
  }

  SECTION("of a text that is longer than the bytes left")
  {
    auto out = ByteWriter();
    out.value(std::uint32_t {10});
    out.value(std::array {'o', 'n', 'l', 'y'});
    auto const bytes = out.release();
    auto in = ByteReader(bytes);
    CHECK_THROWS_WITH(
      in.text(),
      Catch::Matchers::ContainsSubstring("input ends early: 10 bytes needed, 4 left")
    );
  }

  SECTION("after any number of its bytes")
  {
    auto const bytes = record();
    {
      auto in = ByteReader(bytes);
      REQUIRE_NOTHROW(read_record(in));
      REQUIRE(in.done());
    }
    for(auto length = std::size_t {0}; length < bytes.size(); ++length) {
      INFO("the input is cut after " << length << " of " << bytes.size() << " bytes");
      auto in = ByteReader(std::string_view(bytes).substr(0, length));
      CHECK_THROWS_AS(read_record(in), ByteReadError);
    }
  }
}

TEST_CASE("A reader refuses a count that the bytes left cannot hold", "[bytes]")
{
  auto out = ByteWriter();
  out.value(std::numeric_limits<std::uint32_t>::max());
  out.value(std::uint32_t {0});
  auto const bytes = out.release();

  SECTION("of a vector, without allocating it")
  {
    auto in = ByteReader(bytes);
    CHECK_THROWS_AS(in.values<Vector3>(), ByteReadError);
  }

  SECTION("of anything")
  {
    auto in = ByteReader(bytes);
    CHECK_THROWS_WITH(
      in.count(),
      Catch::Matchers::ContainsSubstring("a count of 4294967295 items of 1 bytes or more")
        and Catch::Matchers::ContainsSubstring("does not fit in the 4 bytes left")
    );
  }
}

TEST_CASE("A count is at most what the bytes left hold of items of the given size", "[bytes]")
{
  auto out = ByteWriter();
  out.count(2);
  out.value(std::array<std::uint8_t, 16> {});
  auto const bytes = out.release();

  CHECK(ByteReader(bytes).count() == 2);
  CHECK(ByteReader(bytes).count(8) == 2);
  CHECK_THROWS_AS(ByteReader(bytes).count(9), ByteReadError);
  // items that take no bytes cannot be checked, so they are taken for one byte
  CHECK(ByteReader(bytes).count(0) == 2);
}

TEST_CASE("A bool is read from a byte that is 0 or 1", "[bytes]")
{
  auto out = ByteWriter();
  out.value(std::uint8_t {0});
  out.value(std::uint8_t {1});
  out.value(std::uint8_t {2});
  auto const bytes = out.release();

  auto in = ByteReader(bytes);
  CHECK_FALSE(in.value<bool>());
  CHECK(in.value<bool>());
  CHECK_THROWS_WITH(
    in.value<bool>(),
    Catch::Matchers::ContainsSubstring("a bool is stored as the byte 2, not 0 or 1")
  );
}

TEST_CASE("A writer refuses a count that does not fit in 32 bits", "[bytes]")
{
  auto out = ByteWriter();
  CHECK_NOTHROW(out.count(std::numeric_limits<std::uint32_t>::max()));
  CHECK(out.size() == sizeof(std::uint32_t));

  if constexpr(sizeof(std::size_t) > sizeof(std::uint32_t)) {
    auto const too_many = static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max())
                        + static_cast<std::size_t>(1);
    CHECK_THROWS_AS(out.count(too_many), InvalidArgumentError);
    CHECK(out.size() == sizeof(std::uint32_t));
  }
}

TEST_CASE("A reader does not take a temporary string it would read freed", "[bytes]")
{
  STATIC_CHECK(std::is_constructible_v<ByteReader, std::string&>);
  STATIC_CHECK(std::is_constructible_v<ByteReader, std::string const&>);
  STATIC_CHECK(std::is_constructible_v<ByteReader, std::string_view>);
  STATIC_CHECK(std::is_constructible_v<ByteReader, char const*>);
  STATIC_CHECK_FALSE(std::is_constructible_v<ByteReader, std::string>);
  STATIC_CHECK_FALSE(std::is_constructible_v<ByteReader, std::string&&>);
}

TEST_CASE("A read error is an Error", "[bytes][error]")
{
  STATIC_CHECK(std::is_base_of_v<Error, ByteReadError>);

  auto in = ByteReader(std::string_view());
  try {
    static_cast<void>(in.value<std::uint32_t>());
    FAIL("the read must throw");
  } catch(Error const& error) {
    CHECK(std::string_view(error.what()) == "input ends early: 4 bytes needed, 0 left");
  }
}
