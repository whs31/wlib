#include <w/common/geodesy.h>
#include <w/common/latitude.h>
#include <w/common/longitude.h>
#include <w/common/glaze/distance.h>
#include <w/common/glaze/latitude.h>
#include <w/common/glaze/longitude.h>

#include <string>
#include <vector>

#include <glaze/beve.hpp>
#include <glaze/json.hpp>
#include <glaze/toml.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "glaze_support.h"
#include "text_support.h"

using namespace w;
using namespace glaze_support;
using Catch::Matchers::WithinAbs;

TEST_CASE("Coordinates write as their signed degrees", "[glaze][coordinates]")
{
  check_round_trip(59.771_north, "59.771");
  check_round_trip(33.86_south, "-33.86");
  check_round_trip(157.86_west, "-157.86");
  check_round_trip(Longitude::antimeridian(), "-180");
}

TEST_CASE("Coordinates read numbers and coordinate strings", "[glaze][coordinates]")
{
  CHECK(read<Latitude>("59.771") == 59.771_north);
  CHECK(read<Latitude>("-90") == Latitude::south_pole());
  CHECK(read<Latitude>(R"("59.771N")") == 59.771_north);
  CHECK(read<Latitude>(R"("50north")") == 50_north);
  CHECK(read<Latitude>(R"("33.86 South")") == 33.86_south);
  CHECK(read<Latitude>(R"("59.771deg")") == 59.771_north);
  CHECK(read<Latitude>(R"("-33.86")") == 33.86_south);
  CHECK_THAT(
    read<Latitude>(test::with_degree_signs(R"("59^46'15.6\"N")")).degrees(),
    WithinAbs(59.771, 1e-12)
  );

  CHECK(read<Longitude>("-157.86") == 157.86_west);
  CHECK(read<Longitude>("180") == Longitude::antimeridian());
  CHECK(read<Longitude>(R"("157.86W")") == 157.86_west);
  CHECK(read<Longitude>(R"("30east")") == 30_east);
  CHECK(read<Longitude>(R"("W 157 51.6")") == Longitude::parse("157.86W"));
}

TEST_CASE("Coordinates reject malformed and out-of-range documents", "[glaze][coordinates]")
{
  auto const latitude = GENERATE(
    std::string("90.5"),
    "-91",
    R"("91N")",
    R"("30E")",
    R"("50east")",
    R"("north")",
    R"("0.5rad")"
  );
  CAPTURE(latitude);
  CHECK(read_error<Latitude>(latitude) == glz::error_code::constraint_violated);

  auto const longitude = GENERATE(std::string("180.5"), "-200", R"("190E")", R"("30N")");
  CAPTURE(longitude);
  CHECK(read_error<Longitude>(longitude) == glz::error_code::constraint_violated);

  // the objects of an Angle are no coordinates
  auto const object = GENERATE(
    std::string(R"({"angle":59.771,"units":"degrees"})"),
    R"({"degrees":59.771})",
    R"({})",
    "true"
  );
  CAPTURE(object);
  CHECK(read_error<Latitude>(object) != glz::error_code::none);
}

TEST_CASE("Coordinates round-trip through a binary format", "[glaze][coordinates][beve]")
{
  auto const position = std::vector {59.771_north, 33.86_south};
  auto buffer = std::string();
  REQUIRE_FALSE(glz::write_beve(position, buffer));
  auto read_back = std::vector<Latitude>();
  REQUIRE_FALSE(glz::read_beve(read_back, buffer));
  CHECK(read_back == position);
}

TEST_CASE(
  "Geodetic positions read coordinates from TOML in any notation",
  "[glaze][coordinates][toml]"
)
{
  auto position = GeodeticPosition {};
  auto const toml = test::with_degree_signs(R"(latitude = "59^46'15.6\"N"
longitude = 30.326
altitude_msl = "70m"
)");
  auto const error = glz::read_toml(position, toml);
  INFO(glz::format_error(error, toml));
  REQUIRE_FALSE(error);
  CHECK_THAT(position.latitude.degrees(), WithinAbs(59.771, 1e-12));
  CHECK(position.longitude == 30.326_east);
  CHECK(position.altitude_msl == 70_meters);
}
