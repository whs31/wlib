#include <w/common/angle.h>
#include <w/common/geodesy.h>
#include <w/common/latitude.h>
#include <w/common/longitude.h>
#include <w/common/math.h>
#include <w/common/wgs84.h>

#include <chrono>
#include <cmath>
#include <concepts>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>

#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "precision_support.h"
#include "text_support.h"

using namespace w;
using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;
using test::with_degree_signs;

namespace
{
  // an Angle in float carries a difference to 6e-8 of itself
  [[nodiscard]] double angle_tolerance_deg(double const degrees)
  {
    return test::tolerance(1e-12, 1e-6) * std::max(1.0, std::abs(degrees));
  }
}  // namespace

TEST_CASE("Coordinates are double whatever the precision of the library", "[coordinates]")
{
  STATIC_CHECK(sizeof(Latitude) == sizeof(double));
  STATIC_CHECK(sizeof(Longitude) == sizeof(double));
  STATIC_CHECK(std::is_trivially_copyable_v<Latitude>);
  STATIC_CHECK(std::is_trivially_copyable_v<Longitude>);
  STATIC_CHECK(std::same_as<decltype(Latitude().degrees()), double>);

  // coordinates are positions, not quantities: they neither add up nor scale
  STATIC_CHECK_FALSE(Quantity<Latitude>);
  STATIC_CHECK_FALSE(Quantity<Longitude>);
  STATIC_CHECK_FALSE(std::is_invocable_v<std::plus<>, Latitude, Latitude>);
  STATIC_CHECK_FALSE(std::is_invocable_v<std::multiplies<>, Latitude, double>);
  STATIC_CHECK_FALSE(std::is_invocable_v<std::plus<>, Longitude, Longitude>);
  STATIC_CHECK_FALSE(std::is_invocable_v<std::less<>, Longitude, Longitude>);
  STATIC_CHECK_FALSE(std::is_convertible_v<Latitude, Longitude>);
  STATIC_CHECK_FALSE(std::is_convertible_v<double, Latitude>);
  STATIC_CHECK_FALSE(std::is_convertible_v<Angle, Latitude>);

  // the literal keeps the digits a float would round off: 1e-7 deg is about a centimetre
  auto constexpr latitude = 60.195'964'1_lat;
  STATIC_CHECK(latitude.degrees() == 60.195'964'1);
  STATIC_CHECK(latitude.degrees() - 60.195'964 > 0.9e-7);
}

TEST_CASE("Latitudes are created from degrees, radians, DMS and angles", "[coordinates]")
{
  STATIC_CHECK(Latitude().degrees() == 0.0);
  STATIC_CHECK(Latitude::equator() == 0_lat);
  STATIC_CHECK(Latitude::north_pole() == 90_north);
  STATIC_CHECK(Latitude::south_pole() == 90_south);
  STATIC_CHECK(33.86_south == -33.86_lat);
  STATIC_CHECK(59.771_north == Latitude::from_degrees(59.771));
  CHECK_THAT(Latitude::from_radians(std::numbers::pi / 4).degrees(), WithinAbs(45.0, 1e-14));
  CHECK_THAT((45_north).radians(), WithinAbs(std::numbers::pi / 4, 1e-15));

  auto constexpr dms = Latitude::Dms {
    .degrees = 59,
    .minutes = 46,
    .seconds = 15.6,
    .hemisphere = Latitude::Hemisphere::North,
  };
  CHECK_THAT(Latitude::from_dms(dms).degrees(), WithinAbs(59.771, 1e-12));
  auto const south = Latitude::from_dms(
    {.degrees = 33, .minutes = 51, .hemisphere = Latitude::Hemisphere::South}
  );
  CHECK_THAT(south.degrees(), WithinAbs(-33.85, 1e-12));

  auto const round_trip = (59.771_north).to_dms();
  CHECK(round_trip.degrees == 59);
  CHECK(round_trip.minutes == 46);
  CHECK_THAT(round_trip.seconds, WithinAbs(15.6, 1e-9));
  CHECK(round_trip.hemisphere == Latitude::Hemisphere::North);
  CHECK(
    (33.5_south).to_dms()
    == Latitude::Dms {.degrees = 33, .minutes = 30, .hemisphere = Latitude::Hemisphere::South}
  );

  CHECK_THAT(Latitude::from_angle(30_deg).degrees(), WithinAbs(30.0, angle_tolerance_deg(30.0)));
  CHECK_THAT(
    static_cast<double>((59.771_north).to_angle().degrees()),
    WithinAbs(59.771, angle_tolerance_deg(59.771))
  );
  CHECK_THAT(
    static_cast<double>((60_north).colatitude().degrees()),
    WithinAbs(30.0, angle_tolerance_deg(30.0))
  );
  CHECK_THAT((30_north).sin(), WithinAbs(0.5, 1e-15));
}

TEST_CASE("Latitudes know their hemisphere and their validity", "[coordinates]")
{
  STATIC_CHECK((10_north).hemisphere() == Latitude::Hemisphere::North);
  STATIC_CHECK((10_south).hemisphere() == Latitude::Hemisphere::South);
  STATIC_CHECK(Latitude::equator().hemisphere() == Latitude::Hemisphere::North);
  STATIC_CHECK((-0.0_lat).hemisphere() == Latitude::Hemisphere::North);
  STATIC_CHECK((10_south).hemisphere_letter() == 'S');
  STATIC_CHECK((10_north).hemisphere_letter() == 'N');

  STATIC_CHECK((90_north).is_valid());
  STATIC_CHECK((90_south).is_valid());
  STATIC_CHECK_FALSE((90.000'001_north).is_valid());
  STATIC_CHECK_FALSE(Latitude::from_degrees(-91).is_valid());
  CHECK_FALSE(Latitude::from_degrees(NaN).is_valid());
  CHECK_FALSE(Latitude::from_degrees(NaN).is_finite());
  CHECK(Latitude::from_degrees(1e300).is_finite());

  STATIC_CHECK(10_south < 10_north);
  STATIC_CHECK(80_north > 70_north);
  STATIC_CHECK((33_south).abs() == 33_north);
  STATIC_CHECK(-(33_south) == 33_north);
}

TEST_CASE("Latitudes clamp at the poles or fold over them", "[coordinates]")
{
  STATIC_CHECK((100_north).clamped() == Latitude::north_pole());
  STATIC_CHECK((100_south).clamped() == Latitude::south_pole());
  STATIC_CHECK((45_north).clamped() == 45_north);

  struct Fold
  {
    double degrees;
    double folded;
    bool beyond_pole;
  };

  auto const fold = GENERATE(
    Fold {45.0, 45.0, false},
    Fold {90.0, 90.0, false},
    Fold {-90.0, -90.0, false},
    Fold {100.0, 80.0, true},
    Fold {-100.0, -80.0, true},
    Fold {180.0, 0.0, true},
    Fold {200.0, -20.0, true},
    Fold {270.0, -90.0, false},
    Fold {290.0, -70.0, false},
    Fold {360.0, 0.0, false},
    Fold {450.0, 90.0, false},
    Fold {460.0, 80.0, true},
    Fold {-200.0, 20.0, true}
  );
  CAPTURE(fold.degrees);
  auto const latitude = Latitude::from_degrees(fold.degrees);
  CHECK_THAT(latitude.folded().degrees(), WithinAbs(fold.folded, 1e-12));
  CHECK(latitude.is_beyond_pole() == fold.beyond_pole);
  CHECK(latitude.folded().is_valid());

  CHECK_FALSE(Latitude::from_degrees(NaN).folded().is_finite());
  CHECK_FALSE(Latitude::from_degrees(NaN).is_beyond_pole());
  // a latitude between the poles keeps its exact digits
  STATIC_CHECK((59.771_north).folded().degrees() == 59.771);
}

TEST_CASE("Latitudes move along meridians by angles", "[coordinates]")
{
  auto latitude = 59.771_north + 1_deg;
  CHECK_THAT(latitude.degrees(), WithinAbs(60.771, angle_tolerance_deg(1.0)));
  latitude -= 2_deg;
  CHECK_THAT(latitude.degrees(), WithinAbs(58.771, angle_tolerance_deg(2.0)));
  CHECK_THAT((10_deg + 5_north).degrees(), WithinAbs(15.0, angle_tolerance_deg(10.0)));
  CHECK_THAT((5_north - 10_deg).degrees(), WithinAbs(-5.0, angle_tolerance_deg(10.0)));

  // a difference is an angle, positive northward; small ones keep their precision in float
  STATIC_CHECK(std::same_as<decltype(59.771_north - 33.86_south), Angle>);
  CHECK_THAT(
    static_cast<double>((59.771_north - 33.86_south).degrees()),
    WithinAbs(93.631, angle_tolerance_deg(93.631))
  );
  CHECK_THAT(
    static_cast<double>((59.771'000'1_north - 59.771_north).degrees()),
    WithinRel(1e-7, test::tolerance(1e-6, 1e-5))
  );

  STATIC_CHECK((10_north).lerp(20_north, 0.25) == 12.5_north);
  STATIC_CHECK((10_south).midpoint(30_north) == 10_north);
}

TEST_CASE("Latitudes convert to geocentric and parametric latitudes", "[coordinates]")
{
  // the geocentric latitude falls short of the geodetic by up to 0.19° at 45°
  CHECK_THAT((45_north).geocentric().degrees(), WithinAbs(44.807'576'784'018, 1e-9));
  CHECK_THAT((45_north).parametric().degrees(), WithinAbs(44.903'787'849'420, 1e-9));
  CHECK((0_north).geocentric() == 0_north);
  CHECK_THAT((90_north).geocentric().degrees(), WithinAbs(90.0, 1e-12));

  auto const degrees = GENERATE(-89.0, -60.0, -12.5, 0.0, 33.3, 59.771, 89.9);
  auto const latitude = Latitude::from_degrees(degrees);
  CHECK_THAT(Latitude::from_geocentric(latitude.geocentric()).degrees(), WithinAbs(degrees, 1e-12));
  CHECK_THAT(Latitude::from_parametric(latitude.parametric()).degrees(), WithinAbs(degrees, 1e-12));
  CHECK(latitude.geocentric().abs() <= latitude.parametric().abs());
  CHECK(latitude.parametric().abs() <= latitude.abs());
}

TEST_CASE("Latitudes give the WGS84 radii of curvature", "[coordinates]")
{
  auto const tolerance = test::tolerance(1e-6, 1.0);
  CHECK_THAT(
    (0_north).prime_vertical_radius().as_m(),
    WithinAbs(wgs84::semi_major_axis, tolerance)
  );
  CHECK_THAT(
    (0_north).meridional_radius().as_m(),
    WithinAbs(wgs84::semi_major_axis * (1.0 - wgs84::eccentricity_squared), tolerance)
  );
  // at the poles both radii are a² / b
  auto const polar = wgs84::semi_major_axis * wgs84::semi_major_axis / wgs84::semi_minor_axis;
  CHECK_THAT((90_north).prime_vertical_radius().as_m(), WithinAbs(polar, tolerance));
  CHECK_THAT((90_south).meridional_radius().as_m(), WithinAbs(polar, tolerance));
  CHECK_THAT((0_north).parallel_radius().as_m(), WithinAbs(wgs84::semi_major_axis, tolerance));
  CHECK_THAT((90_north).parallel_radius().as_m(), WithinAbs(0.0, 1e-6));
  // a degree of latitude at 45° is 111.132 km
  CHECK_THAT(
    (45_north).meridional_radius().as_m() * std::numbers::pi / 180.0,
    WithinAbs(111'131.777, test::tolerance(1e-3, 0.1))
  );
}

TEST_CASE("Latitudes give their UTM band", "[coordinates]")
{
  STATIC_CHECK((80_south).utm_band() == 'C');
  STATIC_CHECK((79.9_south).utm_band() == 'C');
  STATIC_CHECK((0_north).utm_band() == 'N');
  STATIC_CHECK((0.1_south).utm_band() == 'M');
  STATIC_CHECK((59.771_north).utm_band() == 'V');
  STATIC_CHECK((72_north).utm_band() == 'X');
  STATIC_CHECK((84_north).utm_band() == 'X');
  STATIC_CHECK_FALSE((84.1_north).utm_band().has_value());
  STATIC_CHECK_FALSE((80.1_south).utm_band().has_value());
  CHECK_FALSE(Latitude::from_degrees(NaN).utm_band().has_value());
}

TEST_CASE("Latitudes print in decimal degrees and DMS", "[coordinates]")
{
  CHECK((59.771_north).to_string() == with_degree_signs("59.771^N"));
  CHECK((33.86_south).to_string() == with_degree_signs("33.86^S"));
  CHECK(Latitude::equator().to_string() == with_degree_signs("0^"));
  CHECK(fmt::format("{}", 90_south) == with_degree_signs("90^S"));
  CHECK(Latitude::from_degrees(NaN).to_string() == with_degree_signs("nan^"));
  auto stream = std::ostringstream();
  stream << 45_north;
  CHECK(stream.str() == with_degree_signs("45^N"));

  CHECK((59.771_north).to_dms_string() == with_degree_signs("59^46'15.60\"N"));
  CHECK((59.771_north).to_dms_string(0) == with_degree_signs("59^46'16\"N"));
  CHECK((59.771_north).to_dms_string(4) == with_degree_signs("59^46'15.6000\"N"));
  CHECK((33.5_south).to_dms_string(1) == with_degree_signs("33^30'00.0\"S"));
  // rounding carries the seconds into the minutes and the degrees
  CHECK((59.999'999_north).to_dms_string() == with_degree_signs("60^00'00.00\"N"));
  CHECK((1e-9_south).to_dms_string() == with_degree_signs("0^00'00.00\""));
  // decimals are limited to 0..9
  CHECK((1_north).to_dms_string(-3) == with_degree_signs("1^00'00\"N"));
  CHECK((1_north).to_dms_string(20) == with_degree_signs("1^00'00.000000000\"N"));
}

TEST_CASE("Latitudes parse decimal degrees and DMS", "[coordinates]")
{
  struct Case
  {
    std::string text;
    double degrees;
  };

  auto const parsed = GENERATE(
    Case {"59.771", 59.771},
    Case {"-33.86", -33.86},
    Case {"+33.86", 33.86},
    Case {"  12  ", 12.0},
    Case {".5", 0.5},
    Case {"59.771N", 59.771},
    Case {"59.771 n", 59.771},
    Case {"33.86S", -33.86},
    Case {"33.86 s", -33.86},
    Case {"S33.86", -33.86},
    Case {"N 59.771", 59.771},
    Case {"59.771^N", 59.771},
    Case {"59.771^", 59.771},
    Case {"59.771deg", 59.771},
    Case {"59.771 deg N", 59.771},
    Case {"33^51.6'S", -33.86},
    Case {"59^46'15.6\"N", 59.771},
    Case {"59^ 46' 15.6\" N", 59.771},
    Case {"59 46 15.6 N", 59.771},
    Case {"59:46:15.6N", 59.771},
    Case {"59d46m15.6s N", 59.771},
    Case {"59d46m15.6sN", 59.771},
    Case {"-59 46 15.6", -59.771},
    Case {
      "59\xc2\xb0"
      "46\xe2\x80\xb2"
      "15.6\xe2\x80\xb3N",
      59.771
    },
    Case {
      "59\xc2\xba"
      "46\xe2\x80\x99"
      "15.6\xe2\x80\x9dN",
      59.771
    },
    Case {"59^46'15.6''N", 59.771},
    Case {"90S", -90.0},
    Case {"50north", 50.0},
    Case {"50 North", 50.0},
    Case {"NORTH 50", 50.0},
    Case {"33.86south", -33.86},
    Case {"south33.86", -33.86},
    Case {"33^51.6'south", -33.86},
    Case {"59 46 15.6south", -59.771},
    Case {"59d46m15.6s south", -59.771},
    Case {"59d46m15.6sS", -59.771},
    Case {"0", 0.0}
  );
  auto const text = with_degree_signs(parsed.text);
  CAPTURE(text);
  auto const latitude = Latitude::parse(text);
  REQUIRE(latitude.has_value());
  CHECK_THAT(
    latitude.value_or(Latitude::from_degrees(NaN)).degrees(),
    WithinAbs(parsed.degrees, 1e-12)
  );
}

TEST_CASE("Latitudes reject malformed and out-of-range text", "[coordinates]")
{
  auto const text = with_degree_signs(GENERATE(
    "",
    "   ",
    "N",
    "abc",
    "59.771E",
    "W59",
    "90.5",
    "-91",
    "100N",
    "-33.86S",
    "S-33.86",
    "+33.86N",
    "N59.771S",
    "59.5^46'",
    "59^60'",
    "59^46'60\"",
    "59^46.5'10\"",
    "59^-46'",
    "59.771x",
    "59.771 N extra",
    "1e1",
    "nan",
    "inf",
    "59'46",
    "50east",
    "west 50",
    "50northeast",
    "50 nort",
    "north -50",
    "-50north",
    "north50south"
  ));
  CAPTURE(text);
  CHECK_FALSE(Latitude::parse(text).has_value());
}

TEST_CASE("Longitudes wrap into [-180, 180)", "[coordinates]")
{
  STATIC_CHECK(Longitude().degrees() == 0.0);
  STATIC_CHECK(Longitude::prime_meridian() == 0_lon);
  STATIC_CHECK(Longitude::antimeridian().degrees() == -180.0);
  STATIC_CHECK((180_east).degrees() == -180.0);
  STATIC_CHECK((180_west).degrees() == -180.0);
  STATIC_CHECK((190_east).degrees() == -170.0);
  STATIC_CHECK((540_east).degrees() == -180.0);
  STATIC_CHECK((-190.0_lon).degrees() == 170.0);
  STATIC_CHECK((720_east).degrees() == 0.0);
  STATIC_CHECK(Longitude::from_degrees(-1e-30).degrees() == -1e-30);
  // in range, a longitude keeps its exact digits
  STATIC_CHECK((30.326_east).degrees() == 30.326);
  STATIC_CHECK((179.999'999'999_east).degrees() == 179.999'999'999);
  STATIC_CHECK((157.86_west) == -157.86_lon);

  auto const degrees = GENERATE(-1e6, -725.5, -360.0, -180.0, -0.5, 0.0, 179.0, 180.0, 359.9, 1e9);
  auto const longitude = Longitude::from_degrees(degrees);
  CAPTURE(degrees);
  CHECK(longitude.degrees() >= -180.0);
  CHECK(longitude.degrees() < 180.0);
  CHECK_THAT(std::remainder(longitude.degrees() - degrees, 360.0), WithinAbs(0.0, 1e-6));
  CHECK(longitude.degrees_east() >= 0.0);
  CHECK(longitude.degrees_east() < 360.0);

  CHECK_FALSE(Longitude::from_degrees(NaN).is_finite());
  CHECK_FALSE(Longitude::from_degrees(Inf).is_finite());
  CHECK_THAT(Longitude::from_radians(-std::numbers::pi).degrees(), WithinAbs(-180.0, 1e-12));
  CHECK_THAT(
    std::abs(Longitude::from_radians(std::numbers::pi).degrees()),
    WithinAbs(180.0, 1e-12)
  );
}

TEST_CASE("Longitudes know their hemisphere and their opposite meridian", "[coordinates]")
{
  STATIC_CHECK((30_east).hemisphere() == Longitude::Hemisphere::East);
  STATIC_CHECK((30_west).hemisphere() == Longitude::Hemisphere::West);
  STATIC_CHECK(Longitude::prime_meridian().hemisphere() == Longitude::Hemisphere::East);
  STATIC_CHECK(Longitude::antimeridian().hemisphere() == Longitude::Hemisphere::West);
  STATIC_CHECK((30_west).hemisphere_letter() == 'W');
  STATIC_CHECK((30_east).hemisphere_letter() == 'E');

  STATIC_CHECK((30_east).opposite() == 150_west);
  STATIC_CHECK((150_west).opposite() == 30_east);
  STATIC_CHECK(Longitude::prime_meridian().opposite() == Longitude::antimeridian());
  STATIC_CHECK(-(30_east) == 30_west);
  STATIC_CHECK(-Longitude::antimeridian() == Longitude::antimeridian());

  STATIC_CHECK((350_east).degrees_east() == 350.0);
  STATIC_CHECK((10_west).degrees_east() == 350.0);
  STATIC_CHECK((10_east).degrees_east() == 10.0);
}

TEST_CASE("Longitudes are created from DMS and angles", "[coordinates]")
{
  auto const longitude = Longitude::from_dms(
    {.degrees = 157, .minutes = 51, .seconds = 36.0, .hemisphere = Longitude::Hemisphere::West}
  );
  CHECK_THAT(longitude.degrees(), WithinAbs(-157.86, 1e-12));
  auto const dms = (30.326_east).to_dms();
  CHECK(dms.degrees == 30);
  CHECK(dms.minutes == 19);
  CHECK_THAT(dms.seconds, WithinAbs(33.6, 1e-9));
  CHECK(dms.hemisphere == Longitude::Hemisphere::East);
  CHECK(
    (1.5_west).to_dms()
    == Longitude::Dms {.degrees = 1, .minutes = 30, .hemisphere = Longitude::Hemisphere::West}
  );
  CHECK_FALSE(std::isfinite(Longitude::from_degrees(NaN).to_dms().seconds));

  CHECK_THAT(
    Longitude::from_angle(270_deg).degrees(),
    WithinAbs(-90.0, angle_tolerance_deg(270.0))
  );
  CHECK_THAT(
    static_cast<double>((30.326_east).to_angle().degrees()),
    WithinAbs(30.326, angle_tolerance_deg(30.326))
  );
}

TEST_CASE("Longitudes move along parallels by angles and wrap", "[coordinates]")
{
  CHECK_THAT((179_east + 3_deg).degrees(), WithinAbs(-178.0, angle_tolerance_deg(180.0)));
  CHECK_THAT((179_west - 3_deg).degrees(), WithinAbs(178.0, angle_tolerance_deg(180.0)));
  CHECK_THAT((10_deg + 175_east).degrees(), WithinAbs(-175.0, angle_tolerance_deg(180.0)));
  auto longitude = 30_east;
  longitude += 720_deg;
  CHECK_THAT(longitude.degrees(), WithinAbs(30.0, angle_tolerance_deg(720.0)));
  longitude -= 60_deg;
  CHECK_THAT(longitude.degrees(), WithinAbs(-30.0, angle_tolerance_deg(720.0)));

  // a difference is the shortest turn, positive eastward
  auto const tokyo = 139.69_east;
  auto const honolulu = 157.86_west;
  CHECK_THAT(
    static_cast<double>((honolulu - tokyo).degrees()),
    WithinAbs(62.45, angle_tolerance_deg(62.45))
  );
  CHECK_THAT(
    static_cast<double>((tokyo - honolulu).degrees()),
    WithinAbs(-62.45, angle_tolerance_deg(62.45))
  );
  CHECK_THAT(
    static_cast<double>(tokyo.angle_to(honolulu).degrees()),
    WithinAbs(62.45, angle_tolerance_deg(62.45))
  );
  CHECK_THAT(
    static_cast<double>((10_east - 10_west).degrees()),
    WithinAbs(20.0, angle_tolerance_deg(20.0))
  );
  // opposite meridians are half a turn west
  CHECK_THAT(
    static_cast<double>((30_east).angle_to(150_west).degrees()),
    WithinAbs(-180.0, angle_tolerance_deg(180.0))
  );
  CHECK_THAT(
    static_cast<double>((150_west).angle_to(30_east).degrees()),
    WithinAbs(-180.0, angle_tolerance_deg(180.0))
  );
}

TEST_CASE("Longitudes compare along the shorter arc", "[coordinates]")
{
  STATIC_CHECK((179_east).is_west_of(179_west));
  STATIC_CHECK((179_west).is_east_of(179_east));
  STATIC_CHECK((10_east).is_east_of(10_west));
  STATIC_CHECK_FALSE((10_east).is_east_of(10_east));
  STATIC_CHECK_FALSE((10_east).is_west_of(10_east));
  STATIC_CHECK((0_east).is_west_of(180_east));
  STATIC_CHECK((180_east).is_west_of(0_east));

  // the arc east from 170°E to 170°W crosses the antimeridian
  STATIC_CHECK((175_east).is_between(170_east, 170_west));
  STATIC_CHECK((180_east).is_between(170_east, 170_west));
  STATIC_CHECK((175_west).is_between(170_east, 170_west));
  STATIC_CHECK((170_east).is_between(170_east, 170_west));
  STATIC_CHECK((170_west).is_between(170_east, 170_west));
  STATIC_CHECK_FALSE((0_east).is_between(170_east, 170_west));
  STATIC_CHECK((0_east).is_between(170_west, 170_east));
  STATIC_CHECK_FALSE((175_east).is_between(170_west, 170_east));
  STATIC_CHECK((30_east).is_between(30_east, 30_east));
  STATIC_CHECK_FALSE((31_east).is_between(30_east, 30_east));

  STATIC_CHECK((170_east).midpoint(170_west) == Longitude::antimeridian());
  STATIC_CHECK((10_west).midpoint(30_east) == 10_east);
  STATIC_CHECK((170_east).lerp(170_west, 0.25) == 175_east);
}

TEST_CASE("Longitudes give solar time, nautical time zones and UTM zones", "[coordinates]")
{
  using namespace std::chrono_literals;

  STATIC_CHECK((15_east).mean_solar_time_offset() == 1h);
  STATIC_CHECK((90_west).mean_solar_time_offset() == -6h);
  STATIC_CHECK((0.25_east).mean_solar_time_offset() == 1min);

  STATIC_CHECK((0_east).nautical_time_zone() == 0h);
  STATIC_CHECK((7.4_east).nautical_time_zone() == 0h);
  STATIC_CHECK((7.5_east).nautical_time_zone() == 1h);
  STATIC_CHECK((7.5_west).nautical_time_zone() == 0h);
  STATIC_CHECK((7.6_west).nautical_time_zone() == -1h);
  STATIC_CHECK((139.69_east).nautical_time_zone() == 9h);
  STATIC_CHECK((179_east).nautical_time_zone() == 12h);
  STATIC_CHECK(Longitude::antimeridian().nautical_time_zone() == -12h);
  CHECK(Longitude::from_degrees(NaN).nautical_time_zone() == 0h);

  STATIC_CHECK(Longitude::antimeridian().utm_zone() == 1);
  STATIC_CHECK((174.1_west).utm_zone() == 1);
  STATIC_CHECK((174_west).utm_zone() == 2);
  STATIC_CHECK((0_east).utm_zone() == 31);
  STATIC_CHECK((3_west).utm_zone() == 30);
  STATIC_CHECK((139.69_east).utm_zone() == 54);
  STATIC_CHECK((179.999_east).utm_zone() == 60);
  CHECK_FALSE(Longitude::from_degrees(NaN).utm_zone().has_value());
}

TEST_CASE("Longitudes print in decimal degrees and DMS", "[coordinates]")
{
  CHECK((30.326_east).to_string() == with_degree_signs("30.326^E"));
  CHECK((157.86_west).to_string() == with_degree_signs("157.86^W"));
  CHECK(Longitude::prime_meridian().to_string() == with_degree_signs("0^"));
  CHECK(Longitude::antimeridian().to_string() == with_degree_signs("180^"));
  CHECK(fmt::format("{}", 45_west) == with_degree_signs("45^W"));

  CHECK((30.326_east).to_dms_string() == with_degree_signs("30^19'33.60\"E"));
  CHECK((157.86_west).to_dms_string(0) == with_degree_signs("157^51'36\"W"));
  // within rounding of the antimeridian there is no hemisphere
  CHECK((179.999'999'99_east).to_dms_string() == with_degree_signs("180^00'00.00\""));
  CHECK((179.999'999'99_west).to_dms_string() == with_degree_signs("180^00'00.00\""));
}

TEST_CASE("Longitudes parse decimal degrees and DMS", "[coordinates]")
{
  CHECK(Longitude::parse("30.326E") == 30.326_east);
  CHECK(Longitude::parse("30.326 e") == 30.326_east);
  CHECK(Longitude::parse("W 157.86") == 157.86_west);
  CHECK(Longitude::parse("-157.86") == 157.86_west);
  CHECK(Longitude::parse("180") == Longitude::antimeridian());
  CHECK(Longitude::parse("180W") == Longitude::antimeridian());
  CHECK(Longitude::parse(with_degree_signs("30.326^E")) == 30.326_east);
  auto const dms = Longitude::parse(with_degree_signs("157^51'36\"W"));
  REQUIRE(dms.has_value());
  CHECK_THAT(dms.value_or(Longitude::from_degrees(NaN)).degrees(), WithinAbs(-157.86, 1e-12));
  auto const decimal_minutes = Longitude::parse("157 51.6 W");
  REQUIRE(decimal_minutes.has_value());
  CHECK_THAT(
    decimal_minutes.value_or(Longitude::from_degrees(NaN)).degrees(),
    WithinAbs(-157.86, 1e-12)
  );

  CHECK(Longitude::parse("157.86west") == 157.86_west);
  CHECK(Longitude::parse("East 30.326") == 30.326_east);
  CHECK(Longitude::parse("30.326 EAST") == 30.326_east);

  CHECK_FALSE(Longitude::parse("30N").has_value());
  CHECK_FALSE(Longitude::parse("30north").has_value());
  CHECK_FALSE(Longitude::parse("190west").has_value());
  CHECK_FALSE(Longitude::parse("S30").has_value());
  CHECK_FALSE(Longitude::parse("180.5").has_value());
  CHECK_FALSE(Longitude::parse("190E").has_value());
  CHECK_FALSE(Longitude::parse("-30E").has_value());
  CHECK_FALSE(Longitude::parse("30E5").has_value());
}

TEST_CASE("Coordinates hash by value", "[coordinates]")
{
  auto const latitudes = std::unordered_set<Latitude> {10_north, 10_north, 10_south};
  CHECK(latitudes.size() == 2);
  // every spelling of a meridian is the same longitude
  auto const longitudes = std::unordered_set<Longitude> {180_east, 180_west, 540_east, 10_east};
  CHECK(longitudes.size() == 2);
  CHECK(std::hash<Latitude> {}(0_north) == std::hash<Latitude> {}(-0.0_lat));
}

TEST_CASE("Geodetic positions know their quadrant", "[coordinates][geodesy]")
{
  STATIC_CHECK(quadrant(59.771_north, 30.326_east) == Quadrant::NorthEast);
  STATIC_CHECK(quadrant(40.71_north, 74.01_west) == Quadrant::NorthWest);
  STATIC_CHECK(quadrant(22.91_south, 43.17_west) == Quadrant::SouthWest);
  STATIC_CHECK(quadrant(33.86_south, 151.21_east) == Quadrant::SouthEast);
  STATIC_CHECK(quadrant(Latitude::equator(), Longitude::prime_meridian()) == Quadrant::NorthEast);
  STATIC_CHECK(quadrant(Latitude::equator(), Longitude::antimeridian()) == Quadrant::NorthWest);
  STATIC_CHECK(
    GeodeticPosition {.latitude = 33.86_south, .longitude = 151.21_east}.quadrant()
    == Quadrant::SouthEast
  );
}

TEST_CASE(
  "Geodetic positions normalize over the poles and find their antipode",
  "[coordinates][geodesy]"
)
{
  auto constexpr past_pole = GeodeticPosition {
    .latitude = 100_north,
    .longitude = 30_east,
    .altitude_msl = 70_meters,
  };
  STATIC_CHECK_FALSE(past_pole.is_valid());
  STATIC_CHECK(past_pole.is_finite());
  auto constexpr normalized = past_pole.normalized();
  STATIC_CHECK(normalized.latitude == 80_north);
  STATIC_CHECK(normalized.longitude == 150_west);
  STATIC_CHECK(normalized.altitude_msl == 70_meters);
  STATIC_CHECK(normalized.is_valid());

  auto constexpr around = GeodeticPosition {.latitude = 200_north, .longitude = 30_east};
  STATIC_CHECK(around.normalized().latitude == 20_south);
  STATIC_CHECK(around.normalized().longitude == 150_west);

  auto constexpr valid = GeodeticPosition {.latitude = 59.771_north, .longitude = 30.326_east};
  STATIC_CHECK(valid.normalized() == valid);

  auto constexpr antipode = valid.antipode();
  STATIC_CHECK(antipode.latitude == 59.771_south);
  CHECK_THAT(antipode.longitude.degrees(), WithinAbs(30.326 - 180.0, 1e-12));
  CHECK_THAT(antipode.antipode().longitude.degrees(), WithinAbs(30.326, 1e-12));
  STATIC_CHECK(antipode.antipode().latitude == valid.latitude);
  STATIC_CHECK(antipode.quadrant() == Quadrant::SouthWest);

  CHECK_FALSE(GeodeticPosition {.latitude = Latitude::from_degrees(NaN)}.is_valid());
}

TEST_CASE("Geodetic positions give their UTM zone", "[coordinates][geodesy]")
{
  auto const zone = [](double const latitude, double const longitude) {
    return GeodeticPosition {
      .latitude = Latitude::from_degrees(latitude),
      .longitude = Longitude::from_degrees(longitude),
    }
      .utm_zone();
  };

  CHECK(zone(59.771, 30.326) == UtmZone {.number = 36, .band = 'V'});
  CHECK(zone(59.771, 30.326).value_or(UtmZone {}).to_string() == "36V");
  CHECK(fmt::format("{}", zone(35.68, 139.69).value_or(UtmZone {})) == "54S");
  CHECK(zone(33.86, -151.21) == UtmZone {.number = 5, .band = 'S'});
  CHECK(zone(-33.86, 151.21) == UtmZone {.number = 56, .band = 'H'});
  // south-western Norway belongs to the widened zone 32
  CHECK(zone(60.39, 5.32) == UtmZone {.number = 32, .band = 'V'});
  CHECK(zone(60.39, 2.9) == UtmZone {.number = 31, .band = 'V'});
  CHECK(zone(55.0, 5.32) == UtmZone {.number = 31, .band = 'U'});
  // Svalbard has the odd zones 31, 33, 35 and 37 only
  CHECK(zone(78.22, 8.9) == UtmZone {.number = 31, .band = 'X'});
  CHECK(zone(78.22, 15.65) == UtmZone {.number = 33, .band = 'X'});
  CHECK(zone(78.22, 25.0) == UtmZone {.number = 35, .band = 'X'});
  CHECK(zone(78.22, 40.0) == UtmZone {.number = 37, .band = 'X'});
  CHECK(zone(78.22, 42.0) == UtmZone {.number = 38, .band = 'X'});
  CHECK(zone(70.0, 15.65) == UtmZone {.number = 33, .band = 'W'});
  // the poles are UPS
  CHECK_FALSE(zone(85.0, 0.0).has_value());
  CHECK_FALSE(zone(-81.0, 0.0).has_value());
}
