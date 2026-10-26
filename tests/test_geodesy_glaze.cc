#include <w/common/geodesy.h>
#include <w/common/glaze/distance.h>
#include <w/common/glaze/latitude.h>
#include <w/common/glaze/longitude.h>

#include <string>

#include <glaze/json.hpp>
#include <glaze/toml.hpp>

#include <catch2/catch_test_macros.hpp>

#include "glaze_support.h"

using namespace w;
using namespace glaze_support;

TEST_CASE("Geodetic positions glaze-reflect into their quantities", "[glaze][geodesy]")
{
  check_round_trip(
    GeodeticPosition {
      .latitude = 45_north,
      .longitude = 30.5_west,
      .altitude_msl = 120_meters,
    },
    R"({"latitude":45,"longitude":-30.5,"altitude_msl":{"meters":120}})"
  );
  REQUIRE(
    strict_read_error<GeodeticPosition>(R"({"latitude":60,"longitude":30})")
    == glz::error_code::missing_key
  );
}

TEST_CASE("Geodetic positions read from TOML in natural units", "[glaze][geodesy][toml]")
{
  auto position = GeodeticPosition {};
  auto const toml = std::string(R"(latitude = "59.771deg"
longitude = "30.326east"
altitude_msl = "70m"
)");
  auto const error = glz::read_toml(position, toml);
  INFO(glz::format_error(error, toml));
  REQUIRE_FALSE(error);
  REQUIRE(
    position
    == GeodeticPosition {
      .latitude = 59.771_north,
      .longitude = 30.326_east,
      .altitude_msl = 70_meters,
    }
  );
}
