#include <w/common/math.h>
#include <w/common/vector2.h>
#include <w/common/vector3.h>

#include <cmath>
#include <limits>
#include <sstream>
#include <unordered_set>

#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>

#include "precision_support.h"

using namespace w::common;
using test::near;

TEST_CASE("Geometry values support constexpr vector math", "[geometry]")
{
  STATIC_CHECK(Vector2::from_xy(3.0, 4.0).length_squared() == 25.0);
  STATIC_CHECK(Vector2::from_xy(1.0, 0.0).cross(Vector2::from_xy(0.0, 1.0)) == 1.0);
  STATIC_CHECK(
    Vector3::from_xyz(1.0, 0.0, 0.0).cross(Vector3::from_xyz(0.0, 1.0, 0.0))
    == Vector3::from_xyz(0.0, 0.0, 1.0)
  );
  STATIC_CHECK(Vector3::from_xyz(1.0, 2.0, 3.0).xy() == Vector2::from_xy(1.0, 2.0));
}

TEST_CASE("Vectors used as positions translate and measure", "[geometry]")
{
  auto const position = Vector2::from_xy(1.0, 2.0);
  auto const moved = position + Vector2::from_xy(3.0, 4.0);
  REQUIRE(moved == Vector2::from_xy(4.0, 6.0));
  REQUIRE(near(position.distance_to(moved), 5.0));
  REQUIRE(near(position.angle_towards(Vector2::from_xy(1.0, 3.0)).degrees(), 90.0, 1e-4));
  // angle_to() compares directions, angle_towards() looks from one position at another
  REQUIRE(near(Vector2::unit_x().angle_to(Vector2::unit_y()).degrees(), 90.0, 1e-4));

  auto const position3 = Vector3::from_xyz(1.0, 2.0, 3.0);
  REQUIRE(near(position3.distance_to(Vector3::from_xyz(1.0, 2.0, -1.0)), 4.0));
}

TEST_CASE("Vectors combine component by component", "[geometry]")
{
  auto constexpr a = Vector3(1.0, -2.0, 3.0);
  auto constexpr b = Vector3(-4.0, 5.0, 0.5);

  STATIC_CHECK(a.component_mul(b) == Vector3(-4.0, -10.0, 1.5));
  STATIC_CHECK(a.component_div(Vector3(2.0, 4.0, -1.0)) == Vector3(0.5, -0.5, -3.0));
  STATIC_CHECK(a.min(b) == Vector3(-4.0, -2.0, 0.5));
  STATIC_CHECK(a.max(b) == Vector3(1.0, 5.0, 3.0));
  STATIC_CHECK(a.abs() == Vector3(1.0, 2.0, 3.0));
  STATIC_CHECK(a.lerp(b, 0.0) == a);
  STATIC_CHECK(a.lerp(b, 1.0) == b);
  STATIC_CHECK(Vector3().lerp(Vector3(2.0, 4.0, 6.0), 0.5) == Vector3(1.0, 2.0, 3.0));

  STATIC_CHECK(Vector2(1.0, -2.0).component_mul({3.0, 4.0}) == Vector2(3.0, -8.0));
  STATIC_CHECK(Vector2(1.0, -2.0).min({0.0, 4.0}) == Vector2(0.0, -2.0));
  STATIC_CHECK(Vector2(1.0, -2.0).max({0.0, 4.0}) == Vector2(1.0, 4.0));
  STATIC_CHECK(Vector2(-1.0, 2.0).abs() == Vector2(1.0, 2.0));
  STATIC_CHECK(Vector2(0.0, 0.0).lerp({2.0, -2.0}, 0.25) == Vector2(0.5, -0.5));
}

TEST_CASE("Vectors normalize, rotate and handle a zero length", "[geometry]")
{
  REQUIRE(near(Vector2::from_xy(3.0, 4.0).normalized().length(), 1.0));
  REQUIRE(near(Vector3::from_xyz(0.0, 3.0, 4.0).normalized().length(), 1.0));
  REQUIRE(Vector2::zero().normalized() == Vector2::zero());
  REQUIRE(Vector3::zero().normalized() == Vector3::zero());
  REQUIRE(Vector2::from_xy(1.0, 0.0).rotated(Angle::frac_pi_2()).y() > 0.999);
  REQUIRE_FALSE(
    std::isfinite(Vector2::from_xy(std::numeric_limits<Real>::infinity(), 0.0).length())
  );
}

TEST_CASE("Geometry values format, stream and hash", "[geometry]")
{
  REQUIRE(Vector2::from_xy(1.0, 2.0).to_string() == "(1, 2)");
  REQUIRE(fmt::format("{}", Vector3::from_xyz(1.0, 2.0, 3.0)) == "(1, 2, 3)");

  auto stream = std::ostringstream();
  stream << Vector3::from_xyz(1.0, 2.0, 3.0);
  REQUIRE(stream.str() == "(1, 2, 3)");

  auto values = std::unordered_set<Vector2> {Vector2::from_xy(1.0, 2.0)};
  REQUIRE(values.contains(Vector2::from_xy(1.0, 2.0)));
}
