#include <w/common/glaze/aabb.h>
#include <w/common/glaze/angle.h>
#include <w/common/glaze/curve.h>
#include <w/common/glaze/frustum.h>
#include <w/common/glaze/matrix3x3.h>
#include <w/common/glaze/matrix4x4.h>
#include <w/common/glaze/plane.h>
#include <w/common/glaze/quaternion.h>
#include <w/common/glaze/random.h>
#include <w/common/glaze/ray.h>
#include <w/common/glaze/read.h>
#include <w/common/glaze/rect.h>
#include <w/common/glaze/sphere.h>
#include <w/common/glaze/transform.h>
#include <w/common/glaze/vector2.h>
#include <w/common/glaze/vector3.h>
#include <w/common/glaze/vector4.h>
#include <w/common/projection.h>

#include <array>
#include <string>

#include <glaze/beve.hpp>
#include <glaze/toml.hpp>

#include <catch2/catch_test_macros.hpp>

#include "glaze_support.h"

using namespace w;
using namespace glaze_support;

// glaze reflects on the names of the members, which clang rejects for a type without linkage, so
// this stays out of the anonymous namespace
namespace linear_algebra_glaze_test
{
  struct Placement  // NOLINT(misc-use-internal-linkage)
  {
    Transform transform;
    AABB bounds;
  };
}  // namespace linear_algebra_glaze_test

using linear_algebra_glaze_test::Placement;

namespace
{

  template <typename T>
  void check_beve_round_trip(T const& value)
  {
    auto const buffer = glz::write_beve(value);
    REQUIRE(buffer.has_value());
    auto back = T {};
    REQUIRE_FALSE(glz::read_beve(back, *buffer));
    REQUIRE(back == value);
  }
}  // namespace

TEST_CASE("Vector and quaternion glaze adapters write arrays", "[glaze][linalg]")
{
  check_round_trip(Vector2::from_xy(1.5, -2.0), "[1.5,-2]");
  check_round_trip(Vector3::from_xyz(1.5, -2.0, 3.25), "[1.5,-2,3.25]");
  check_round_trip(Vector4::from_xyzw(1.5, -2.0, 3.25, 1.0), "[1.5,-2,3.25,1]");
  // scalar first, as Quaternion stores it
  check_round_trip(Quaternion::from_wxyz(1.0, 0.0, 0.5, -0.5), "[1,0,0.5,-0.5]");

  REQUIRE(read_error<Vector3>("[1,2]") != glz::error_code::none);
  REQUIRE(read_error<Vector3>("[1,2,3,4]") != glz::error_code::none);
  REQUIRE(read_error<Vector2>("[1,2,3]") != glz::error_code::none);
  REQUIRE(read_error<Quaternion>("[1,0,0]") != glz::error_code::none);
  // the object form of earlier versions is gone
  REQUIRE(read_error<Vector3>(R"({"x":1,"y":2,"z":3})") != glz::error_code::none);
  REQUIRE(read_error<Quaternion>(R"({"w":1,"x":0,"y":0,"z":0})") != glz::error_code::none);
}

TEST_CASE("Matrix glaze adapters write rows", "[glaze][matrix]")
{
  check_round_trip(Matrix3x3::identity(), "[[1,0,0],[0,1,0],[0,0,1]]");
  check_round_trip(
    Matrix3x3::from_rows({1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.5}),
    "[[1,2,3],[4,5,6],[7,8,9.5]]"
  );
  check_round_trip(
    Matrix4x4::from_translation({2.0, 0.0, -1.5}),
    "[[1,0,0,2],[0,1,0,0],[0,0,1,-1.5],[0,0,0,1]]"
  );

  REQUIRE(read_error<Matrix3x3>("[[1,0,0],[0,1,0],[0,0,1],[0,0,0]]") != glz::error_code::none);
  REQUIRE(read_error<Matrix3x3>("[[1,0,0,0],[0,1,0],[0,0,1]]") != glz::error_code::none);
  REQUIRE(read_error<Matrix3x3>(R"({"rows":[]})") != glz::error_code::none);
  REQUIRE(read_error<Matrix4x4>("[[1,0,0],[0,1,0],[0,0,1]]") != glz::error_code::none);
}

TEST_CASE("Transform glaze adapter names its parts and defaults missing ones", "[glaze][transform]")
{
  auto const
    transform = Transform({1.0, 2.0, 3.0}, Quaternion(0.0, 0.0, 1.0, 0.0), {2.0, 2.0, 2.0});
  check_round_trip(transform, R"({"position":[1,2,3],"rotation":[0,0,1,0],"scale":[2,2,2]})");
  check_round_trip(Transform(), R"({"position":[0,0,0],"rotation":[1,0,0,0],"scale":[1,1,1]})");

  // even a strict read keeps the identity for what a document leaves out
  REQUIRE(read_glaze<Transform>(R"({"position":[0,1,0]})") == Transform::from_position({0, 1, 0}));
  REQUIRE(read_glaze<Transform>("{}") == Transform::identity());
  REQUIRE(read_error<Transform>(R"({"position":[0,1]})") != glz::error_code::none);
  REQUIRE(read_error<Transform>(R"({"translation":[0,1,0]})") == glz::error_code::unknown_key);

  auto const placement = read_glaze<Placement, strict_toml>(
    "transform = { position = [0, 1.5, 0], scale = [2, 2, 2] }\n"
    "bounds = { min = [-1, 0, -1], max = [1, 2, 1] }\n"
  );
  REQUIRE(placement.transform == Transform({0.0, 1.5, 0.0}, {}, {2.0, 2.0, 2.0}));
  REQUIRE(placement.bounds == AABB({-1.0, 0.0, -1.0}, {1.0, 2.0, 1.0}));
  // a TOML array with a missing component is an error, not a zero
  REQUIRE_THROWS_AS(
    (read_glaze<Placement, strict_toml>("transform = { position = [0, 1.5] }\n"
                                        "bounds = { min = [-1, 0, -1], max = [1, 2, 1] }\n")),
    DocumentError
  );
  check_beve_round_trip(transform);
}

TEST_CASE("Geometry glaze adapters write arrays of components", "[glaze][geometry]")
{
  check_round_trip(
    AABB({-1.0, 0.0, -2.0}, {1.0, 2.5, 2.0}),
    R"({"min":[-1,0,-2],"max":[1,2.5,2]})"
  );
  check_round_trip(Sphere({1.0, 2.0, 3.0}, 0.5), R"({"center":[1,2,3],"radius":0.5})");
  check_round_trip(Plane({0.0, 1.0, 0.0}, -2.0), "[0,1,0,-2]");
  check_round_trip(
    Ray({0.0, 1.0, 0.0}, {0.0, 0.0, -1.0}),
    R"({"origin":[0,1,0],"direction":[0,0,-1]})"
  );
  check_round_trip(Rect(1.5, 2.0, 640.0, 360.0), "[1.5,2,640,360]");
  check_round_trip(Recti(0, 0, 640, 360), "[0,0,640,360]");

  auto const frustum = Frustum::from_view_projection(
    orthographic(-2.0, 2.0, -1.0, 1.0, 0.5, 4.0, Handedness::Right)
  );
  REQUIRE(read<Frustum>(write(frustum)) == frustum);
  REQUIRE(write(frustum).starts_with("[[1,0,0,2],[-1,0,0,2],"));

  REQUIRE(read_error<Recti>("[0,0,1.5,2]") != glz::error_code::none);
  REQUIRE(read_error<Rect>("[0,0,1]") != glz::error_code::none);
  check_beve_round_trip(Recti(-3, 4, 16, 9));
  check_beve_round_trip(AABB({-1.0, 0.0, -2.0}, {1.0, 2.5, 2.0}));
}

TEST_CASE("Curve glaze adapter validates the points", "[glaze][curve]")
{
  using C = Curve<double, double, 4>;
  auto const curve = C(std::array {0.0, 1.0, 2.0}, std::array {10.0, 20.0, 15.0});
  check_round_trip(curve, R"({"arguments":[0,1,2],"values":[10,20,15]})");

  auto const angles = Curve<
    Angle,
    double,
    2>(std::array {Angle::from_degrees(0.0), Angle::from_degrees(90.0)}, std::array {0.0, 1.0});
  REQUIRE(read<Curve<Angle, double, 2>>(write(angles)) == angles);

  REQUIRE(
    read_error<C>(R"({"arguments":[0,2,1],"values":[1,2,3]})")
    == glz::error_code::constraint_violated
  );
  REQUIRE(
    read_error<C>(R"({"arguments":[0,1],"values":[1]})") == glz::error_code::constraint_violated
  );
  REQUIRE(
    read_error<C>(R"({"arguments":[0,1,2,3,4],"values":[0,1,2,3,4]})")
    == glz::error_code::constraint_violated
  );
}

TEST_CASE("Random stream glaze adapter restores the generator state", "[glaze][random]")
{
  check_round_trip(RandomStream {.state = {1, 2, 3, 4}}, R"({"state":[1,2,3,4]})");

  auto stream = RandomStream::derive(1, "vt30-1", "imu.gyro");
  REQUIRE(read<RandomStream>(write(stream)) == stream);

  REQUIRE(
    read_error<RandomStream>(R"({"state":[0,0,0,0]})") == glz::error_code::constraint_violated
  );
}
