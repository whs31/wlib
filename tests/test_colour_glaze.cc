#include <w/common/glaze/colour.h>
#include <w/common/glaze/read.h>

#include <map>
#include <string>
#include <vector>

#include <glaze/beve.hpp>
#include <glaze/toml.hpp>

#include <catch2/catch_test_macros.hpp>

#include "glaze_support.h"

using namespace w;
using namespace glaze_support;

// glaze reflects on the names of the members, which clang rejects for a type without linkage, so
// this stays out of the anonymous namespace
namespace colour_glaze_test
{
  struct Palette  // NOLINT(misc-use-internal-linkage)
  {
    Rgba8 sand;
    Rgba8 sea;
    Rgba sky;
  };
}  // namespace colour_glaze_test

using colour_glaze_test::Palette;

TEST_CASE("Rgba8 glaze adapter writes hex strings and reads bytes too", "[glaze][colour]")
{
  check_round_trip("#e4a672"_rgba8, R"("#e4a672")");
  check_round_trip("#e4a67280"_rgba8, R"("#e4a67280")");
  CHECK(read<Rgba8>(R"("#E4A")") == "#ee44aa"_rgba8);
  CHECK(read<Rgba8>("[228,166,114]") == "#e4a672"_rgba8);
  CHECK(read<Rgba8>("[228,166,114,128]") == "#e4a67280"_rgba8);

  CHECK(read_error<Rgba8>(R"("#e4a67")") == glz::error_code::constraint_violated);
  CHECK(read_error<Rgba8>(R"("sand")") == glz::error_code::constraint_violated);
  CHECK(read_error<Rgba8>("[228,166]") == glz::error_code::constraint_violated);
  CHECK(read_error<Rgba8>("[256,0,0]") == glz::error_code::constraint_violated);
  CHECK(read_error<Rgba8>("[-1,0,0]") == glz::error_code::constraint_violated);
  CHECK(read_error<Rgba8>("228") != glz::error_code::none);
}

TEST_CASE("Rgba glaze adapter writes linear arrays and reads hex too", "[glaze][colour]")
{
  check_round_trip(Rgba(1.0F, 0.5F, 0.0F, 0.25F), "[1,0.5,0,0.25]");
  CHECK(read<Rgba>("[1,0.5,0]") == Rgba(1.0F, 0.5F, 0.0F));
  // a hex string is sRGB and is decoded to linear light
  CHECK(read<Rgba>(R"("#e4a672")") == Rgba::from_srgb8("#e4a672"_rgba8));

  CHECK(read_error<Rgba>("[1,0.5]") == glz::error_code::constraint_violated);
  CHECK(read_error<Rgba>("[1,0.5,0,1,1]") == glz::error_code::constraint_violated);
  CHECK(read_error<Rgba>(R"("#xyz")") == glz::error_code::constraint_violated);
}

TEST_CASE("Palettes live in TOML", "[glaze][colour]")
{
  auto const palette = read_glaze<Palette, strict_toml>(
    "sand = \"#e4a672\"\n"
    "sea = [16, 64, 128]\n"
    "sky = \"#87ceeb\"\n"
  );
  CHECK(palette.sand == "#e4a672"_rgba8);
  CHECK(palette.sea == Rgba8(16, 64, 128));
  CHECK(palette.sky.to_srgb8() == "#87ceeb"_rgba8);

  auto const named = read_glaze<std::map<std::string, Rgba8>, strict_toml>(
    "grass = \"#4c9a2a\"\n"
    "shadow = \"#00000080\"\n"
  );
  CHECK(named.at("grass") == "#4c9a2a"_rgba8);
  CHECK(named.at("shadow").a() == 0x80);

  CHECK_THROWS_AS(
    (read_glaze<Palette, strict_toml>("sand = \"#e4a6z\"\nsea = \"#000\"\nsky = [1, 0, 0]\n")),
    DocumentError
  );
}

TEST_CASE("Colours round-trip through BEVE", "[glaze][colour]")
{
  auto const palette = std::vector {"#e4a672"_rgba8, "#00000080"_rgba8};
  auto const buffer = glz::write_beve(palette);
  REQUIRE(buffer.has_value());
  auto back = std::vector<Rgba8> {};
  REQUIRE_FALSE(glz::read_beve(back, *buffer));
  CHECK(back == palette);

  auto const linear = Rgba(0.25F, 0.5F, 0.75F, 1.0F);
  auto const linear_buffer = glz::write_beve(linear);
  REQUIRE(linear_buffer.has_value());
  auto linear_back = Rgba {};
  REQUIRE_FALSE(glz::read_beve(linear_back, *linear_buffer));
  CHECK(linear_back == linear);
}
