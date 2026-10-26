#include <w/common/angle.h>
#include <w/common/clip_space.h>
#include <w/common/error.h>
#include <w/common/euler_angles.h>
#include <w/common/matrix3x3.h>
#include <w/common/matrix4x4.h>
#include <w/common/projection.h>
#include <w/common/quaternion.h>
#include <w/common/transform.h>
#include <w/common/vector2.h>
#include <w/common/vector3.h>
#include <w/common/vector4.h>

#include <cmath>
#include <algorithm>
#include <array>
#include <functional>
#include <limits>
#include <stdexcept>

#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>

#include "precision_support.h"

using namespace w;

namespace
{
  [[nodiscard]] double max_difference(Vector3 const a, Vector3 const b)
  {
    return std::max({std::abs(a.x() - b.x()), std::abs(a.y() - b.y()), std::abs(a.z() - b.z())});
  }

  [[nodiscard]] double max_difference(Vector4 const a, Vector4 const b)
  {
    return std::max<double>(max_difference(a.xyz(), b.xyz()), std::abs(a.w() - b.w()));
  }

  [[nodiscard]] double max_difference(Matrix4x4 const& a, Matrix4x4 const& b)
  {
    return std::max({
      max_difference(a.row(0), b.row(0)),
      max_difference(a.row(1), b.row(1)),
      max_difference(a.row(2), b.row(2)),
      max_difference(a.row(3), b.row(3)),
    });
  }

  [[nodiscard]] double rotation_difference(Quaternion const& a, Quaternion const& b)
  {
    auto const
      same = std::max<double>(std::abs(a.w() - b.w()), max_difference(a.vector(), b.vector()));
    auto const
      flipped = std::max<double>(std::abs(a.w() + b.w()), max_difference(a.vector(), -b.vector()));
    return std::min(same, flipped);
  }

  auto constexpr infinity = std::numeric_limits<Real>::infinity();
}  // namespace

TEST_CASE("Vector4 separates points from directions", "[linalg][vector]")
{
  STATIC_CHECK(Vector4::from_point({1.0, 2.0, 3.0}) == Vector4(1.0, 2.0, 3.0, 1.0));
  STATIC_CHECK(Vector4::from_direction({1.0, 2.0, 3.0}) == Vector4(1.0, 2.0, 3.0, 0.0));
  STATIC_CHECK(Vector4(2.0, 4.0, -6.0, 2.0).perspective_divided() == Vector3(1.0, 2.0, -3.0));
  STATIC_CHECK(Vector4(1.0, 2.0, 3.0, 4.0).dot(Vector4::splat(1.0)) == 10.0F);
  STATIC_CHECK(Vector4(1.0, -2.0, 3.0, 0.0).min(Vector4::zero()) == Vector4(0.0, -2.0, 0.0, 0.0));
  STATIC_CHECK(Vector4(1.0, -2.0, 3.0, 0.0).max(Vector4::zero()) == Vector4(1.0, 0.0, 3.0, 0.0));
  STATIC_CHECK(Vector4::zero().lerp(Vector4::splat(2.0), 0.25) == Vector4::splat(0.5));
  STATIC_CHECK(
    Vector4::from_xyzw(1.0, 2.0, 3.0, 4.0).with_w(0.0).to_array()
    == std::array<Real, 4> {1.0, 2.0, 3.0, 0.0}
  );

  CHECK(Vector4(0.0, 3.0, 0.0, 4.0).length() == 5.0F);
  CHECK(Vector4(0.0, 3.0, 0.0, 4.0).normalized() == Vector4(0.0, 0.6, 0.0, 0.8));
  CHECK_FALSE(Vector4::zero().try_normalized().has_value());
  CHECK_FALSE(Vector4(0.0, infinity, 0.0, 1.0).is_finite());
  CHECK(fmt::format("{}", Vector4(1.0, -2.0, 0.5, 1.0)) == "(1, -2, 0.5, 1)");
  CHECK(Vector4(1.0, -2.0, 0.5, 1.0).to_string() == "(1, -2, 0.5, 1)");
  CHECK(std::hash<Vector4> {}(Vector4::unit_x()) != std::hash<Vector4> {}(Vector4::unit_y()));
}

TEST_CASE("Matrix4x4 is row-major and transforms column vectors", "[linalg][matrix]")
{
  auto constexpr m = Matrix4x4::from_rows(
    {1.0, 2.0, 3.0, 4.0},
    {5.0, 6.0, 7.0, 8.0},
    {9.0, 10.0, 11.0, 12.0},
    {13.0, 14.0, 15.0, 16.0}
  );
  STATIC_CHECK(m.at(0, 3) == 4.0F);
  STATIC_CHECK(m.column(1) == Vector4(2.0, 6.0, 10.0, 14.0));
  STATIC_CHECK(m.to_array().at(4) == 5.0F);
  STATIC_CHECK(Matrix4x4::from_array(m.to_array()) == m);
  STATIC_CHECK(Matrix4x4::from_columns(m.row(0), m.row(1), m.row(2), m.row(3)) == m.transposed());
  STATIC_CHECK(m * Vector4::unit_x() == m.column(0));
  STATIC_CHECK(m.trace() == 34.0F);
  STATIC_CHECK(m.determinant() == 0.0F);
  STATIC_CHECK(not m.is_invertible());
  STATIC_CHECK(not m.is_affine());

  CHECK_THROWS_AS(m.at(4, 0), std::out_of_range);
  CHECK_THROWS_AS(m.row(4), std::out_of_range);
  CHECK_THROWS_AS(m.inverse(), NumericError);
  CHECK_FALSE(m.try_inverse().has_value());
}

TEST_CASE("Matrix4x4 transforms points and directions differently", "[linalg][matrix]")
{
  auto constexpr move = Matrix4x4::from_translation({1.0, 2.0, 3.0});
  STATIC_CHECK(move.is_affine());
  STATIC_CHECK(move.translation() == Vector3(1.0, 2.0, 3.0));
  STATIC_CHECK(move.transform_point(Vector3::zero()) == Vector3(1.0, 2.0, 3.0));
  STATIC_CHECK(move.transform_direction(Vector3::unit_x()) == Vector3::unit_x());
  STATIC_CHECK(
    Matrix4x4::from_scale({2.0, 3.0, 4.0}).transform_point(Vector3::splat(1.0))
    == Vector3(2.0, 3.0, 4.0)
  );
  STATIC_CHECK(Matrix4x4::diagonal({1.0, 2.0, 3.0, 4.0}).determinant() == 24.0F);

  // note: [whs31 : 2026-10-26]
  // the inverse is constexpr, so a constant transform can be inverted at compile time
  auto constexpr inverse = Matrix4x4::
                             from_affine(Matrix3x3::diagonal({2.0, 4.0, 8.0}), {1.0, 1.0, 1.0})
                               .inverse();
  STATIC_CHECK(inverse.transform_point({1.0, 1.0, 1.0}) == Vector3::zero());
  STATIC_CHECK(inverse.linear() == Matrix3x3::diagonal({0.5, 0.25, 0.125}));

  auto constexpr projection = Matrix4x4::
    from_rows(Vector4::unit_x(), Vector4::unit_y(), Vector4::unit_z(), {0.0, 0.0, 1.0, 0.0});
  STATIC_CHECK(projection.project_point({2.0, 4.0, 2.0}) == Vector3(1.0, 2.0, 1.0));
  STATIC_CHECK(projection.transform_point({2.0, 4.0, 2.0}) == Vector3(2.0, 4.0, 2.0));

  CHECK(
    fmt::format("{}", Matrix4x4::identity())
    == "[(1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 1, 0), (0, 0, 0, 1)]"
  );
  CHECK(std::hash<Matrix4x4> {}(move) != std::hash<Matrix4x4> {}(Matrix4x4::identity()));
}

TEST_CASE("Matrix4x4 inverts views far from the origin", "[linalg][matrix]")
{
  // note: [whs31 : 2026-10-26]
  // a large translation inflates every row of a view matrix; the singularity test must not take
  // that for a near-singular matrix
  for(auto const distance : {1.0F, 1e3F, 1e5F}) {
    auto const eye = Vector3(distance, 0.5F * distance, -distance);
    auto const view = look_at_rh(eye, eye + Vector3(0.0, 0.0, -1.0), Vector3::unit_y());
    auto const
      projection = perspective(Angle::from_degrees(60), 1.5, 0.1, 1000.0, Handedness::Right);
    REQUIRE(view.is_invertible());
    REQUIRE((projection * view).is_invertible());
    CHECK(
      max_difference(view.inverse().transform_point(Vector3::zero()), eye) < 1e-6 * distance + 1e-6
    );
  }
  CHECK_FALSE(Matrix4x4::from_scale({1.0, 1.0, 0.0}).is_invertible());
}

TEST_CASE("Quaternion builds rotations from directions", "[linalg][quaternion]")
{
  auto const quarter = Quaternion::from_to(Vector3::unit_x(), Vector3::unit_y() * 5);
  CHECK(
    rotation_difference(quarter, Quaternion::from_axis_angle(Vector3::unit_z(), Angle::frac_pi_2()))
    < 1e-6
  );
  CHECK(
    rotation_difference(
      Quaternion::from_to(Vector3::unit_x(), Vector3::unit_x()),
      Quaternion::identity()
    )
    < 1e-7
  );
  for(auto const axis :
      {Vector3::unit_x(), Vector3::unit_y(), Vector3::unit_z(), Vector3(1.0, 1.0, 1.0)}) {
    auto const opposite = Quaternion::from_to(axis, -axis);
    CHECK(max_difference(opposite.rotate(axis), -axis) < 1e-6);
    CHECK(std::abs(opposite.length() - 1) < 1e-6);
  }
  CHECK_THROWS_AS(Quaternion::from_to(Vector3::zero(), Vector3::unit_x()), NumericError);

  auto const forward = Vector3(1.0, 0.0, -1.0).normalized();
  auto const rh = Quaternion::look_rotation(forward, Vector3::unit_y(), Handedness::Right);
  auto const lh = Quaternion::look_rotation(forward, Vector3::unit_y(), Handedness::Left);
  CHECK(max_difference(rh.rotate(-Vector3::unit_z()), forward) < 1e-6);
  CHECK(max_difference(lh.rotate(Vector3::unit_z()), forward) < 1e-6);
  CHECK(max_difference(rh.rotate(Vector3::unit_y()), Vector3::unit_y()) < 1e-6);
  // looking straight up leaves the roll arbitrary but still points the camera up
  auto const
    up = Quaternion::look_rotation(Vector3::unit_y(), Vector3::unit_y(), Handedness::Right);
  CHECK(max_difference(up.rotate(-Vector3::unit_z()), Vector3::unit_y()) < 1e-6);
  CHECK_THROWS_AS(
    Quaternion::look_rotation(Vector3::zero(), Vector3::unit_y(), Handedness::Right),
    NumericError
  );
}

TEST_CASE("Quaternion interpolates and measures rotations", "[linalg][quaternion]")
{
  auto const a = Quaternion::from_axis_angle(Vector3::unit_z(), Angle::from_degrees(10));
  auto const b = Quaternion::from_axis_angle(Vector3::unit_z(), Angle::from_degrees(90));
  CHECK(std::abs(a.angle_to(b).degrees() - 80) < 1e-3);
  CHECK(std::abs(a.angle_to(-b).degrees() - 80) < 1e-3);
  CHECK(std::abs(b.angle().degrees() - 90) < 1e-3);
  CHECK(
    rotation_difference(
      a.slerp(b, 0.5),
      Quaternion::from_axis_angle(Vector3::unit_z(), Angle::from_degrees(50))
    )
    < 1e-6
  );
  // the short arc, even when b is given with the other sign
  CHECK(
    rotation_difference(
      a.slerp(-b, 0.5),
      Quaternion::from_axis_angle(Vector3::unit_z(), Angle::from_degrees(50))
    )
    < 1e-6
  );
  CHECK(std::abs(a.nlerp(b, 0.5).length() - 1) < 1e-6);
  // nearly equal rotations take the nlerp path without dividing by a vanishing sine
  auto const close = Quaternion::from_axis_angle(Vector3::unit_z(), Angle::from_degrees(10.00001));
  CHECK(a.slerp(close, 0.5).is_finite());

  CHECK(rotation_difference(a.inverse() * a, Quaternion::identity()) < 1e-7);
  CHECK(Quaternion::from_xyzw(0.0, 0.0, 1.0, 0.0) == Quaternion::from_wxyz(0.0, 0.0, 0.0, 1.0));
  CHECK(
    Quaternion::from_wxyz(1.0, 2.0, 3.0, 4.0).to_xyzw() == std::array<Real, 4> {2.0, 3.0, 4.0, 1.0}
  );
  CHECK(max_difference(b.to_matrix4(), Matrix4x4::from_affine(b.to_rotation_matrix())) == 0.0);
  CHECK_THROWS_AS(Quaternion(0.0, 0.0, 0.0, 0.0).normalized(), NumericError);
}

TEST_CASE("Euler angles round-trip in every order", "[linalg][euler]")
{
  auto const x = Angle::from_degrees(20);
  auto const y = Angle::from_degrees(-35);
  auto const z = Angle::from_degrees(70);

  // R_A(a) * R_B(b) * R_C(c), by the letters of the order
  auto const rx = Quaternion::from_axis_angle(Vector3::unit_x(), x);
  auto const ry = Quaternion::from_axis_angle(Vector3::unit_y(), y);
  auto const rz = Quaternion::from_axis_angle(Vector3::unit_z(), z);
  CHECK(rotation_difference(Quaternion::from_euler<EulerOrder::XYZ>(x, y, z), rx * ry * rz) < 1e-6);
  CHECK(rotation_difference(Quaternion::from_euler<EulerOrder::YXZ>(x, y, z), ry * rx * rz) < 1e-6);
  CHECK(rotation_difference(Quaternion::from_euler<EulerOrder::ZYX>(x, y, z), rz * ry * rx) < 1e-6);

  auto const check_round_trip = [&]<EulerOrder Order>() {
    auto const angles = Quaternion::from_euler<Order>(x, y, z).template to_euler<Order>();
    CHECK(std::abs((angles.at(0) - x).degrees()) < 1e-3);
    CHECK(std::abs((angles.at(1) - y).degrees()) < 1e-3);
    CHECK(std::abs((angles.at(2) - z).degrees()) < 1e-3);
  };
  check_round_trip.operator()<EulerOrder::XYZ>();
  check_round_trip.operator()<EulerOrder::XZY>();
  check_round_trip.operator()<EulerOrder::YXZ>();
  check_round_trip.operator()<EulerOrder::YZX>();
  check_round_trip.operator()<EulerOrder::ZXY>();
  check_round_trip.operator()<EulerOrder::ZYX>();

  // at the singularity the last angle is zero and the first carries the whole turn
  auto const locked = Quaternion::from_euler<
    EulerOrder::YXZ>(Angle::frac_pi_2(), Angle::from_degrees(30), Angle::from_degrees(15));
  auto const [about_x, about_y, about_z] = locked.to_euler<EulerOrder::YXZ>();
  CHECK(std::abs(about_x.degrees() - 90) < 1e-2);
  CHECK(std::abs(about_z.degrees()) < 1e-6);
  auto const rebuilt = Quaternion::from_euler<EulerOrder::YXZ>(about_x, about_y, about_z);
  CHECK(rotation_difference(rebuilt, locked) < 1e-3);
}

TEST_CASE("Yaw, pitch and roll name the aerospace order", "[linalg][euler]")
{
  auto const attitude = EulerAngles(
    Angle::from_degrees(5),
    Angle::from_degrees(-10),
    Angle::from_degrees(170)
  );
  auto const q = Quaternion::from_yaw_pitch_roll(attitude);
  CHECK(
    rotation_difference(
      q,
      Quaternion::from_euler<EulerOrder::ZYX>(attitude.roll(), attitude.pitch(), attitude.yaw())
    )
    == 0.0
  );
  auto const back = q.to_yaw_pitch_roll();
  CHECK(std::abs((back.yaw() - attitude.yaw()).degrees()) < 1e-3);
  CHECK(std::abs((back.pitch() - attitude.pitch()).degrees()) < 1e-3);
  CHECK(std::abs((back.roll() - attitude.roll()).degrees()) < 1e-3);

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable : 4'996)
#else
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
  // the deprecated names are aliases of the aerospace order
  CHECK(Quaternion::from_euler_angles(attitude) == q);
  CHECK(q.to_euler_angles() == back);
#ifdef _MSC_VER
#  pragma warning(pop)
#else
#  pragma GCC diagnostic pop
#endif
}

TEST_CASE("Transform composes position, rotation and scale", "[linalg][transform]")
{
  auto const turn = Quaternion::from_axis_angle(Vector3::unit_y(), Angle::frac_pi_2());
  auto const parent = Transform({10.0, 0.0, 0.0}, turn, Vector3::splat(2.0));
  auto const child = Transform({1.0, 0.0, 0.0}, Quaternion::identity(), {1.0, 3.0, 1.0});

  // scale first, then rotate, then translate
  CHECK(max_difference(parent.transform_point(Vector3::unit_x()), {10.0, 0.0, -2.0}) < 1e-6);
  CHECK(max_difference(parent.transform_direction(Vector3::unit_x()), {0.0, 0.0, -2.0}) < 1e-6);
  CHECK(
    max_difference(
      parent.to_matrix(),
      Matrix4x4::from_translation(parent.position()) * turn.to_matrix4()
        * Matrix4x4::from_scale(parent.scale())
    )
    < 1e-6
  );

  auto const world = parent.compose(child);
  CHECK(max_difference(world.to_matrix(), parent.to_matrix() * child.to_matrix()) < 1e-5);
  auto const point = Vector3(0.5, -1.0, 2.0);
  CHECK(
    max_difference(
      world.transform_point(point),
      parent.transform_point(child.transform_point(point))
    )
    < 1e-5
  );
  CHECK(max_difference(parent.compose(parent.inverse()).to_matrix(), Matrix4x4::identity()) < 1e-6);
  CHECK(
    max_difference(parent.inverse().transform_point(parent.transform_point(point)), point) < 1e-5
  );

  STATIC_CHECK(Transform::identity().to_matrix() == Matrix4x4::identity());
  STATIC_CHECK(
    Transform::from_position({1.0, 2.0, 3.0}).to_matrix()
    == Matrix4x4::from_translation({1.0, 2.0, 3.0})
  );
  STATIC_CHECK(
    Transform::from_scale({1.0, 2.0, 3.0}).to_matrix() == Matrix4x4::from_scale({1.0, 2.0, 3.0})
  );
  STATIC_CHECK(
    Transform().with_position({1.0, 0.0, 0.0}).with_scale(Vector3::splat(2.0)).is_finite()
  );
  CHECK(
    fmt::format("{}", Transform()) == "{position (0, 0, 0), rotation (1, 0, 0, 0), scale (1, 1, 1)}"
  );
  CHECK(std::hash<Transform> {}(parent) != std::hash<Transform> {}(child));
}

TEST_CASE("Transform decomposes affine matrices", "[linalg][transform]")
{
  auto const
    rotation = Quaternion::from_axis_angle(Vector3(1.0, 2.0, -0.5), Angle::from_degrees(75));
  auto const transform = Transform({3.0, -4.0, 5.0}, rotation, {0.5, 2.0, 3.0});
  REQUIRE(Transform::decompose(transform.to_matrix()).has_value());
  auto const decomposed = Transform::decompose(transform.to_matrix()).value_or(Transform());
  CHECK(max_difference(decomposed.position(), transform.position()) < 1e-5);
  CHECK(max_difference(decomposed.scale(), transform.scale()) < 1e-5);
  CHECK(rotation_difference(decomposed.rotation(), rotation) < 1e-5);

  // a mirror keeps a proper rotation and carries the reflection in a negative scale
  auto const mirrored = transform.to_matrix() * Matrix4x4::from_scale({-1.0, 1.0, 1.0});
  REQUIRE(Transform::decompose(mirrored).has_value());
  auto const unmirrored = Transform::decompose(mirrored).value_or(Transform());
  CHECK(unmirrored.scale().x() < 0);
  CHECK(std::abs(unmirrored.rotation().length() - 1) < 1e-5);
  CHECK(max_difference(unmirrored.to_matrix(), mirrored) < 1e-5);

  CHECK_FALSE(
    Transform::decompose(perspective(Angle::from_degrees(60), 1.0, 0.1, 10.0, Handedness::Right))
      .has_value()
  );
  CHECK_FALSE(Transform::decompose(Matrix4x4::from_scale({1.0, 0.0, 1.0})).has_value());
  CHECK_FALSE(Transform::decompose(Matrix4x4::from_translation({infinity, 0.0, 0.0})).has_value());
}

TEST_CASE("Projections map depth to [0, 1]", "[linalg][projection]")
{
  auto const depth_of = [](Matrix4x4 const& projection, Real const view_z) {
    return projection.project_point({0.0, 0.0, view_z}).z();
  };
  auto const fov = Angle::from_degrees(90);
  auto const rh = perspective(fov, 1.0, 0.5, 50.0, Handedness::Right);
  auto const lh = perspective(fov, 1.0, 0.5, 50.0, Handedness::Left);
  auto const reversed = perspective(fov, 1.0, 0.5, 50.0, Handedness::Right, ClipDepth::Reversed);
  auto const
    infinite = perspective(fov, 1.0, 0.5, infinity, Handedness::Right, ClipDepth::Reversed);
  CHECK(std::abs(depth_of(rh, -0.5)) < 1e-6);
  CHECK(std::abs(depth_of(rh, -50) - 1) < 1e-6);
  CHECK(std::abs(depth_of(lh, 0.5)) < 1e-6);
  CHECK(std::abs(depth_of(lh, 50) - 1) < 1e-6);
  CHECK(std::abs(depth_of(reversed, -0.5) - 1) < 1e-6);
  CHECK(std::abs(depth_of(reversed, -50)) < 1e-6);
  CHECK(std::abs(depth_of(infinite, -0.5) - 1) < 1e-6);
  CHECK(depth_of(infinite, -1e30F) >= 0);
  CHECK(depth_of(infinite, -1e30F) < 1e-6);
  // a 90° field of view puts the edge of the near plane at the edge of the screen
  CHECK(std::abs(rh.project_point({0.5, 0.0, -0.5}).x() - 1) < 1e-6);

  STATIC_CHECK(
    orthographic(-2.0, 2.0, -1.0, 1.0, 0.0, 10.0, Handedness::Right)
      .transform_point({2.0, 1.0, -10.0})
    == Vector3(1.0, 1.0, 1.0)
  );
  STATIC_CHECK(
    orthographic(-2.0, 2.0, -1.0, 1.0, 0.0, 10.0, Handedness::Left, ClipDepth::Reversed)
      .transform_point({-2.0, -1.0, 10.0})
    == Vector3(-1.0, -1.0, 0.0)
  );
}

TEST_CASE("look_at is the inverse of the camera transform", "[linalg][projection]")
{
  auto const eye = Vector3(3.0, 4.0, 5.0);
  auto const target = Vector3(-1.0, 0.0, 2.0);
  for(auto const handedness : {Handedness::Right, Handedness::Left}) {
    auto const view = look_at(eye, target, Vector3::unit_y(), handedness);
    auto const camera = Transform(
      eye,
      Quaternion::look_rotation(target - eye, Vector3::unit_y(), handedness)
    );
    CHECK(max_difference(view, camera.to_matrix().inverse()) < 1e-5);
    auto const forward = handedness == Handedness::Right ? Real {-1} : Real {1};
    auto const seen = view.transform_point(target);
    CHECK(std::abs(seen.x()) < 1e-5);
    CHECK(std::abs(seen.y()) < 1e-5);
    CHECK(std::abs(seen.z() - forward * (target - eye).length()) < 1e-5);
  }
  // looking straight down the up axis is still a valid view
  CHECK(look_at_rh(Vector3::zero(), -Vector3::unit_y(), Vector3::unit_y()).is_finite());
  CHECK_THROWS_AS(look_at_rh(eye, eye, Vector3::unit_y()), NumericError);
}

TEST_CASE("snap_to_texel moves the camera by whole texels", "[linalg][projection]")
{
  auto const orientation = Quaternion::
    look_rotation({1.0, -1.0, -1.0}, Vector3::unit_y(), Handedness::Right);
  auto const texel = Real {0.25};
  auto const position = Vector3(10.13, 3.7, -2.41);
  auto const snap = snap_to_texel(position, orientation, texel);

  auto const camera = orientation.inverse_rotate(snap.position);
  auto const in_texels = camera.xy() / texel;
  CHECK(std::abs(in_texels.x() - std::round(in_texels.x())) < 1e-3);
  CHECK(std::abs(in_texels.y() - std::round(in_texels.y())) < 1e-3);
  CHECK(std::abs(camera.z() - orientation.inverse_rotate(position).z()) < 1e-5);
  CHECK(std::abs(snap.offset_texels.x()) <= 0.5F);
  CHECK(std::abs(snap.offset_texels.y()) <= 0.5F);
  // the offset restores the true camera
  auto const restored = orientation.rotate({(in_texels + snap.offset_texels) * texel, camera.z()});
  CHECK(max_difference(restored, position) < 1e-4);
  // a camera already on the grid stays put
  CHECK(
    max_difference(snap_to_texel(snap.position, orientation, texel).position, snap.position) < 1e-4
  );
}
