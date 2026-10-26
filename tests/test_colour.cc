#include <w/common/angle.h>
#include <w/common/colour.h>

#include <cmath>
#include <cstdint>
#include <functional>
#include <sstream>

#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>

using namespace w;

namespace
{
  [[nodiscard]] bool near(float const actual, float const expected, float const tolerance = 1e-5F)
  {
    return std::abs(actual - expected) <= tolerance;
  }
}  // namespace

TEST_CASE("sRGB transfer function matches the standard", "[colour]")
{
  CHECK(srgb_to_linear(0.0F) == 0.0F);
  CHECK(srgb_to_linear(1.0F) == 1.0F);
  CHECK(linear_to_srgb(0.0F) == 0.0F);
  CHECK(near(linear_to_srgb(1.0F), 1.0F, 1e-6F));
  CHECK(near(srgb_to_linear(0.5F), 0.214041F));
  CHECK(near(srgb_to_linear(0.04045F), 0.0031308F, 1e-6F));
  CHECK(near(linear_to_srgb(0.0031308F), 0.04045F, 1e-5F));
  for(auto step = 0; step <= 255; ++step) {
    auto const encoded = static_cast<float>(step) / 255.0F;
    CHECK(near(linear_to_srgb(srgb_to_linear(encoded)), encoded, 1e-5F));
  }
}

TEST_CASE("Rgba8 parses and writes hex strings", "[colour]")
{
  STATIC_CHECK(Rgba8::from_hex("#e4a672") == Rgba8(0xe4, 0xa6, 0x72));
  STATIC_CHECK(Rgba8::from_hex("E4A672") == Rgba8(0xe4, 0xa6, 0x72));
  STATIC_CHECK(Rgba8::from_hex("#e4a67280") == Rgba8(0xe4, 0xa6, 0x72, 0x80));
  STATIC_CHECK(Rgba8::from_hex("#e4a") == Rgba8(0xee, 0x44, 0xaa));
  STATIC_CHECK(Rgba8::from_hex("#e4a8") == Rgba8(0xee, 0x44, 0xaa, 0x88));
  STATIC_CHECK(not Rgba8::from_hex("").has_value());
  STATIC_CHECK(not Rgba8::from_hex("#").has_value());
  STATIC_CHECK(not Rgba8::from_hex("#e4a67").has_value());
  STATIC_CHECK(not Rgba8::from_hex("#e4a6720").has_value());
  STATIC_CHECK(not Rgba8::from_hex("#g4a672").has_value());
  STATIC_CHECK(not Rgba8::from_hex("##e4a672").has_value());

  STATIC_CHECK("#e4a672"_rgba8 == Rgba8(0xe4, 0xa6, 0x72));
  STATIC_CHECK("#fff"_rgba8 == Rgba8(255, 255, 255));
  STATIC_CHECK("#e4a67280"_rgba8.to_rgba32() == 0xe4a6'7280U);
  STATIC_CHECK(Rgba8::from_rgba32(0x1020'30ffU) == Rgba8(0x10, 0x20, 0x30));

  CHECK(Rgba8(0xe4, 0xa6, 0x72).to_hex() == "#e4a672");
  CHECK(Rgba8(0xe4, 0xa6, 0x72, 0x80).to_hex() == "#e4a67280");
  CHECK(fmt::format("{}", Rgba8(0, 15, 255)) == "#000fff");
  CHECK(fmt::format("{:>9}", Rgba8(0, 15, 255)) == "  #000fff");
  auto stream = std::ostringstream();
  stream << Rgba8(1, 2, 3, 4);
  CHECK(stream.str() == "#01020304");
  CHECK(std::hash<Rgba8> {}(Rgba8(1, 2, 3)) != std::hash<Rgba8> {}(Rgba8(3, 2, 1)));
}

TEST_CASE("Rgba is linear and converts to and from sRGB", "[colour]")
{
  auto const sand = Rgba::from_srgb8("#e4a672"_rgba8);
  CHECK(near(sand.r(), srgb_to_linear(228.0F / 255.0F)));
  CHECK(near(sand.g(), srgb_to_linear(166.0F / 255.0F)));
  CHECK(near(sand.b(), srgb_to_linear(114.0F / 255.0F)));
  CHECK(sand.a() == 1.0F);
  CHECK(sand.to_srgb8() == "#e4a672"_rgba8);
  CHECK(Rgba::from_hex("#e4a672") == sand);
  CHECK_FALSE(Rgba::from_hex("sand").has_value());

  // every 8-bit value survives decoding and encoding
  for(auto value = 0; value <= 255; ++value) {
    auto const byte = static_cast<std::uint8_t>(value);
    auto const colour = Rgba8(byte, byte, byte, byte);
    CHECK(colour.to_linear().to_srgb8() == colour);
  }

  STATIC_CHECK(Rgba(0.5F, 0.25F, 1.0F, 0.5F).premultiplied() == Rgba(0.25F, 0.125F, 0.5F, 0.5F));
  STATIC_CHECK(Rgba(2.0F, -1.0F, 0.5F, 3.0F).clamped() == Rgba(1.0F, 0.0F, 0.5F, 1.0F));
  STATIC_CHECK(Rgba::black().lerp(Rgba::white(), 0.25F) == Rgba(0.25F, 0.25F, 0.25F));
  STATIC_CHECK(Rgba::white().with_alpha(0.0F) == Rgba(1.0F, 1.0F, 1.0F, 0.0F));
  STATIC_CHECK(Rgba::transparent().a() == 0.0F);
  CHECK(near(Rgba::white().luminance(), 1.0F));
  CHECK(near(Rgba(0.0F, 1.0F, 0.0F).luminance(), 0.7152F));
  // values beyond [0, 1] encode clamped
  CHECK(
    Rgba(2.0F, -1.0F, 0.5F).to_srgb8()
    == Rgba8(255, 0, static_cast<std::uint8_t>(std::lround(linear_to_srgb(0.5F) * 255.0F)))
  );
  CHECK(fmt::format("{}", Rgba(1.0F, 0.5F, 0.0F)) == "(1, 0.5, 0, 1)");
  CHECK(Rgba(1.0F, 0.5F, 0.0F).to_string() == "(1, 0.5, 0, 1)");
  CHECK(std::hash<Rgba> {}(Rgba::black()) != std::hash<Rgba> {}(Rgba::white()));
}

TEST_CASE("HSV describes the sRGB encoding", "[colour]")
{
  auto const red = Rgba8(255, 0, 0).to_hsv();
  CHECK(near(red.hue.degrees(), 0.0F));
  CHECK(red.saturation == 1.0F);
  CHECK(red.value == 1.0F);
  CHECK(red.alpha == 1.0F);
  CHECK(near(Rgba8(0, 255, 0).to_hsv().hue.degrees(), 120.0F, 1e-3F));
  CHECK(near(Rgba8(0, 0, 255).to_hsv().hue.degrees(), 240.0F, 1e-3F));
  CHECK(Rgba8(128, 128, 128).to_hsv().saturation == 0.0F);

  CHECK(
    Rgba8::from_hsv({.hue = Angle::from_degrees(60), .saturation = 1.0F, .value = 1.0F})
    == Rgba8(255, 255, 0)
  );
  // a hue past a full turn, or negative, wraps
  CHECK(
    Rgba8::from_hsv({.hue = Angle::from_degrees(-120), .saturation = 1.0F, .value = 1.0F})
    == Rgba8(0, 0, 255)
  );
  CHECK(
    Rgba8::from_hsv({.hue = Angle::from_degrees(480), .saturation = 1.0F, .value = 1.0F})
    == Rgba8(0, 255, 0)
  );

  // the 8-bit colours survive a round trip through HSV
  for(auto const colour :
      {"#e4a672"_rgba8, "#123456"_rgba8, "#ffffff"_rgba8, "#000000"_rgba8, "#7f00ff80"_rgba8}) {
    CHECK(Rgba8::from_hsv(colour.to_hsv()) == colour);
    auto const linear = Rgba::from_srgb8(colour);
    CHECK(Rgba::from_hsv(linear.to_hsv()).to_srgb8() == colour);
  }
  CHECK(
    fmt::format("{}", Hsv {.hue = Angle::from_degrees(30), .saturation = 0.5F, .value = 1.0F})
    == "hsv(30°, 0.5, 1, 1)"
  );
}
