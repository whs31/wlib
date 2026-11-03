#include <w/common/aabb.h>
#include <w/common/angle.h>
#include <w/common/clip_space.h>
#include <w/common/frustum.h>
#include <w/common/matrix4x4.h>
#include <w/common/plane.h>
#include <w/common/projection.h>
#include <w/common/quaternion.h>
#include <w/common/ray.h>
#include <w/common/rect.h>
#include <w/common/sphere.h>
#include <w/common/vector2.h>
#include <w/common/vector3.h>

#include <cmath>
#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <vector>

#include <fmt/format.h>

#include <catch2/catch_test_macros.hpp>

using namespace w;

namespace
{
  [[nodiscard]] bool near(Real const actual, Real const expected, Real const tolerance = 1e-5F)
  {
    return std::abs(actual - expected) <= tolerance;
  }

  [[nodiscard]] bool near(
    Vector3 const actual,
    Vector3 const expected,
    Real const tolerance = 1e-5F
  )
  {
    return near(actual.x(), expected.x(), tolerance) and near(actual.y(), expected.y(), tolerance)
       and near(actual.z(), expected.z(), tolerance);
  }
}  // namespace

TEST_CASE("Plane measures signed distances", "[bounds][plane]")
{
  auto constexpr floor = Plane::from_point_normal({0.0, 2.0, 0.0}, Vector3::unit_y());
  STATIC_CHECK(floor.signed_distance({5.0, 3.0, -1.0}) == 1.0F);
  STATIC_CHECK(floor.signed_distance({5.0, 0.0, -1.0}) == -2.0F);
  STATIC_CHECK(floor.closest_point({5.0, 7.0, -1.0}) == Vector3(5.0, 2.0, -1.0));
  STATIC_CHECK(floor.flipped().signed_distance({0.0, 3.0, 0.0}) == -1.0F);
  STATIC_CHECK(Plane::from_vector4(floor.to_vector4()) == floor);

  auto const scaled = Plane({0.0, 0.0, 4.0}, -8.0).normalized();
  CHECK(scaled == Plane(Vector3::unit_z(), -2.0));
  CHECK(Plane().normalized() == Plane());

  // counter-clockwise points, seen from the side the normal points to
  auto const through = Plane::from_points({0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0});
  REQUIRE(through.has_value());
  CHECK(near(through.value_or(Plane()).normal(), Vector3::unit_z()));
  CHECK_FALSE(Plane::from_points({0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}, {2.0, 2.0, 2.0}).has_value());
  CHECK(fmt::format("{}", floor) == "(0, 1, 0, -2)");
  CHECK(std::hash<Plane> {}(floor) != std::hash<Plane> {}(floor.flipped()));
}

TEST_CASE("Sphere contains, intersects and transforms", "[bounds][sphere]")
{
  auto constexpr ball = Sphere({1.0, 0.0, 0.0}, 2.0);
  STATIC_CHECK(ball.contains({3.0, 0.0, 0.0}));
  STATIC_CHECK(not ball.contains({3.1F, 0.0, 0.0}));
  STATIC_CHECK(ball.intersects(Sphere({5.0, 0.0, 0.0}, 2.0)));
  STATIC_CHECK(not ball.intersects(Sphere({5.1F, 0.0, 0.0}, 2.0)));

  auto const moved = ball.transformed(
    Matrix4x4::from_translation({0.0, 1.0, 0.0}) * Matrix4x4::from_scale({1.0, 3.0, 2.0})
  );
  CHECK(moved.center() == Vector3(1.0, 1.0, 0.0));
  // a non-uniform scale grows the radius by the largest factor, so the result still bounds
  CHECK(moved.radius() == 6.0F);
  CHECK(fmt::format("{}", ball) == "{center (1, 0, 0), radius 2}");
}

TEST_CASE("AABB grows, tests and transforms", "[bounds][aabb]")
{
  STATIC_CHECK(AABB().is_empty());
  STATIC_CHECK(
    AABB::empty().merged(Vector3(1.0, 2.0, 3.0)) == AABB({1.0, 2.0, 3.0}, {1.0, 2.0, 3.0})
  );
  STATIC_CHECK(not AABB().contains(Vector3::zero()));

  auto constexpr box = AABB({-1.0, 0.0, -2.0}, {1.0, 4.0, 2.0});
  STATIC_CHECK(box.center() == Vector3(0.0, 2.0, 0.0));
  STATIC_CHECK(box.extents() == Vector3(1.0, 2.0, 2.0));
  STATIC_CHECK(box.size() == Vector3(2.0, 4.0, 4.0));
  STATIC_CHECK(box.contains(Vector3(1.0, 4.0, 2.0)));
  STATIC_CHECK(not box.contains(Vector3(1.1F, 4.0, 2.0)));
  STATIC_CHECK(box.contains(AABB({0.0, 1.0, 0.0}, {0.5, 2.0, 1.0})));
  STATIC_CHECK(box.intersects(AABB({1.0, 4.0, 2.0}, {3.0, 5.0, 3.0})));
  STATIC_CHECK(not box.intersects(AABB({1.5F, 0.0, 0.0}, {3.0, 5.0, 3.0})));
  STATIC_CHECK(box.intersects(Sphere({2.0, 2.0, 0.0}, 1.0)));
  STATIC_CHECK(not box.intersects(Sphere({2.5F, 2.0, 0.0}, 1.0)));
  STATIC_CHECK(box.closest_point({5.0, -3.0, 0.5F}) == Vector3(1.0, 0.0, 0.5F));
  STATIC_CHECK(box.expanded(1.0) == AABB({-2.0, -1.0, -3.0}, {2.0, 5.0, 3.0}));
  STATIC_CHECK(box.merged(AABB()) == box);
  STATIC_CHECK(AABB::from_center_extents({0.0, 2.0, 0.0}, {1.0, 2.0, 2.0}) == box);
  STATIC_CHECK(box.corners().at(0) == box.min());
  STATIC_CHECK(box.corners().at(7) == box.max());

  auto const points = std::vector<Vector3> {{1.0, 2.0, 3.0}, {-1.0, 5.0, 0.0}, {0.0, 0.0, 4.0}};
  CHECK(AABB::from_points(points) == AABB({-1.0, 0.0, 0.0}, {1.0, 5.0, 4.0}));
  CHECK(AABB::from_points({}).is_empty());

  // a quarter turn about y swaps the x and z extents
  auto const turn = Quaternion::from_axis_angle(Vector3::unit_y(), Angle::frac_pi_2()).to_matrix4();
  auto const turned = box.transformed(Matrix4x4::from_translation({10.0, 0.0, 0.0}) * turn);
  CHECK(near(turned.min(), {8.0, 0.0, -1.0}));
  CHECK(near(turned.max(), {12.0, 4.0, 1.0}));
  CHECK(AABB().transformed(turn).is_empty());

  auto const sphere = box.bounding_sphere();
  CHECK(sphere.center() == box.center());
  CHECK(near(sphere.radius(), 3.0F));
  CHECK(fmt::format("{}", box) == "[(-1, 0, -2), (1, 4, 2)]");
  CHECK(std::hash<AABB> {}(box) != std::hash<AABB> {}(AABB()));
}

TEST_CASE("Ray meets planes, spheres, boxes and triangles", "[bounds][ray]")
{
  auto const ray = Ray({0.0, 0.0, 5.0}, {0.0, 0.0, -1.0});
  STATIC_CHECK(Ray({0.0, 0.0, 5.0}, -Vector3::unit_z()).point_at(2.0) == Vector3(0.0, 0.0, 3.0));

  CHECK(ray.intersect(Plane(Vector3::unit_z(), 0.0)) == 5.0F);
  // either side of a plane counts
  CHECK(ray.intersect(Plane(-Vector3::unit_z(), 0.0)) == 5.0F);
  CHECK_FALSE(ray.intersect(Plane(Vector3::unit_z(), -6.0)).has_value());
  CHECK_FALSE(ray.intersect(Plane(Vector3::unit_x(), 0.0)).has_value());

  CHECK(ray.intersect(Sphere(Vector3::zero(), 1.0)) == 4.0F);
  CHECK(ray.intersect(Sphere({0.0, 0.0, 5.0}, 1.0)) == 0.0F);
  CHECK_FALSE(ray.intersect(Sphere({0.0, 0.0, 10.0}, 1.0)).has_value());
  CHECK_FALSE(ray.intersect(Sphere({2.0, 0.0, 0.0}, 1.0)).has_value());

  CHECK(ray.intersect(AABB(Vector3::splat(-1.0), Vector3::splat(1.0))) == 4.0F);
  CHECK(ray.intersect(AABB({-1.0, -1.0, 4.0}, {1.0, 1.0, 6.0})) == 0.0F);
  CHECK_FALSE(ray.intersect(AABB({2.0, -1.0, -1.0}, {3.0, 1.0, 1.0})).has_value());
  CHECK_FALSE(ray.intersect(AABB({-1.0, -1.0, 6.0}, {1.0, 1.0, 7.0})).has_value());
  // a ray along a face meets the box
  CHECK(ray.intersect(AABB({0.0, -1.0, -1.0}, {1.0, 1.0, 1.0})) == 4.0F);
  CHECK_FALSE(ray.intersect(AABB()).has_value());

  auto const hit = ray.intersect_triangle({-1.0, -1.0, 0.0}, {3.0, -1.0, 0.0}, {-1.0, 3.0, 0.0});
  REQUIRE(hit.has_value());
  auto const [t, u, v] = hit.value_or(TriangleHit {});
  CHECK(t == 5.0F);
  CHECK(near(u, 0.25F));
  CHECK(near(v, 0.25F));
  // both windings
  CHECK(ray.intersect_triangle({-1.0, -1.0, 0.0}, {-1.0, 3.0, 0.0}, {3.0, -1.0, 0.0}).has_value());
  CHECK_FALSE(
    ray.intersect_triangle({1.0, 1.0, 0.0}, {3.0, 1.0, 0.0}, {1.0, 3.0, 0.0}).has_value()
  );
  CHECK_FALSE(
    ray.intersect_triangle({-1.0, -1.0, 6.0}, {3.0, -1.0, 6.0}, {-1.0, 3.0, 6.0}).has_value()
  );

  auto const through = Ray::through({1.0, 1.0, 1.0}, {1.0, 1.0, -3.0});
  CHECK(through.direction() == -Vector3::unit_z());
  CHECK(fmt::format("{}", ray) == "{origin (0, 0, 5), direction (0, 0, -1)}");
}

TEST_CASE("Picking rays start on the near plane", "[bounds][ray]")
{
  for(auto const depth : {ClipDepth::Standard, ClipDepth::Reversed}) {
    for(auto const far : {100.0F, std::numeric_limits<Real>::infinity()}) {
      auto const eye = Vector3(0.0, 2.0, 10.0);
      auto const view = look_at_rh(eye, Vector3(0.0, 2.0, 0.0), Vector3::unit_y());
      auto const
        projection = perspective(Angle::from_degrees(90), 1.0, 0.5, far, Handedness::Right, depth);
      auto const inverse = (projection * view).inverse();

      auto const center = Ray::from_ndc(Vector2::zero(), inverse, depth);
      CHECK(near(center.origin(), {0.0, 2.0, 9.5}, 1e-4F));
      CHECK(near(center.direction(), -Vector3::unit_z(), 1e-4F));

      // with a 90° field of view the corner of the screen is 45° off the axis in x and y
      auto const corner = Ray::from_ndc({1.0, 1.0}, inverse, depth);
      CHECK(near(corner.direction(), Vector3(1.0, 1.0, -1.0).normalized(), 1e-4F));
    }
  }

  auto const ortho = orthographic(-4.0, 4.0, -2.0, 2.0, 1.0, 20.0, Handedness::Right).inverse();
  auto const parallel = Ray::from_ndc({0.5, -0.5}, ortho);
  CHECK(near(parallel.origin(), {2.0, -1.0, -1.0}, 1e-4F));
  CHECK(near(parallel.direction(), -Vector3::unit_z(), 1e-4F));
}

TEST_CASE("Frustum classifies bounds", "[bounds][frustum]")
{
  auto const view = look_at_rh(Vector3::zero(), -Vector3::unit_z(), Vector3::unit_y());
  for(auto const depth : {ClipDepth::Standard, ClipDepth::Reversed}) {
    auto const
      projection = perspective(Angle::from_degrees(90), 1.0, 1.0, 100.0, Handedness::Right, depth);
    auto const frustum = Frustum::from_view_projection(projection * view, depth);

    CHECK(near(frustum.near_plane().normal(), -Vector3::unit_z()));
    CHECK(near(frustum.near_plane().distance(), -1.0F));
    CHECK(near(frustum.far_plane().normal(), Vector3::unit_z()));
    CHECK(near(frustum.far_plane().distance(), 100.0F, 1e-3F));
    CHECK(near(frustum.left_plane().normal(), Vector3(1.0, 0.0, -1.0).normalized()));

    CHECK(frustum.contains({0.0, 0.0, -50.0}));
    CHECK_FALSE(frustum.contains({0.0, 0.0, -0.5}));
    CHECK_FALSE(frustum.contains({0.0, 0.0, -101.0}));
    CHECK_FALSE(frustum.contains({20.0, 0.0, -10.0}));

    CHECK(frustum.classify(Sphere({0.0, 0.0, -50.0}, 1.0)) == Containment::Inside);
    CHECK(frustum.classify(Sphere({0.0, 0.0, -100.0}, 1.0)) == Containment::Intersecting);
    CHECK(frustum.classify(Sphere({0.0, 0.0, 10.0}, 1.0)) == Containment::Outside);
    CHECK(frustum.classify(AABB({-1.0, -1.0, -11.0}, {1.0, 1.0, -9.0})) == Containment::Inside);
    CHECK(
      frustum.classify(AABB({5.0, -1.0, -11.0}, {15.0, 1.0, -9.0})) == Containment::Intersecting
    );
    CHECK(frustum.classify(AABB({20.0, -1.0, -11.0}, {25.0, 1.0, -9.0})) == Containment::Outside);
    CHECK(frustum.classify(AABB()) == Containment::Outside);
    CHECK(frustum.intersects(Sphere({0.0, 0.0, -100.0}, 1.0)));
    CHECK_FALSE(frustum.intersects(AABB({20.0, -1.0, -11.0}, {25.0, 1.0, -9.0})));
  }

  auto const ortho = Frustum::from_view_projection(
    orthographic(-2.0, 2.0, -1.0, 1.0, 0.0, 10.0, Handedness::Right)
  );
  CHECK(ortho.contains({1.9F, 0.9F, -9.9F}));
  CHECK_FALSE(ortho.contains({2.1F, 0.0, -5.0}));
  CHECK(std::hash<Frustum> {}(ortho) != std::hash<Frustum> {}(Frustum()));
}

TEST_CASE("Rect measures and combines areas", "[bounds][rect]")
{
  auto constexpr rect = Rect(10.0, 20.0, 40.0, 30.0);
  STATIC_CHECK(rect.min() == Vector2(10.0, 20.0));
  STATIC_CHECK(rect.max() == Vector2(50.0, 50.0));
  STATIC_CHECK(rect.center() == Vector2(30.0, 35.0));
  STATIC_CHECK(rect.area() == 1200.0F);
  STATIC_CHECK(rect.contains(Vector2(10.0, 20.0)));
  // half-open: the max edge is outside
  STATIC_CHECK(not rect.contains(Vector2(50.0, 30.0)));
  STATIC_CHECK(rect.contains(Rect(20.0, 25.0, 10.0, 10.0)));
  STATIC_CHECK(rect.intersects(Rect(45.0, 45.0, 10.0, 10.0)));
  STATIC_CHECK(not rect.intersects(Rect(50.0, 20.0, 10.0, 10.0)));
  STATIC_CHECK(rect.intersection(Rect(45.0, 45.0, 10.0, 10.0)) == Rect(45.0, 45.0, 5.0, 5.0));
  STATIC_CHECK(not rect.intersection(Rect(60.0, 20.0, 10.0, 10.0)).has_value());
  STATIC_CHECK(rect.united(Rect(0.0, 0.0, 1.0, 1.0)) == Rect(0.0, 0.0, 50.0, 50.0));
  STATIC_CHECK(rect.translated({1.0, -1.0}) == Rect(11.0, 19.0, 40.0, 30.0));
  STATIC_CHECK(rect.expanded(5.0) == Rect(5.0, 15.0, 50.0, 40.0));
  STATIC_CHECK(rect.scaled(2.0) == Rect(20.0, 40.0, 80.0, 60.0));
  STATIC_CHECK(Rect::from_min_max({1.0, 2.0}, {4.0, 6.0}) == Rect(1.0, 2.0, 3.0, 4.0));
  STATIC_CHECK(Rect::from_center_size({0.0, 0.0}, {4.0, 2.0}) == Rect(-2.0, -1.0, 4.0, 2.0));
  STATIC_CHECK(Rect(0.0, 0.0, 0.0, 5.0).is_empty());

  CHECK(rect.aspect_ratio() == 4.0F / 3.0F);
  CHECK(fmt::format("{}", rect) == "(10, 20, 40, 30)");
}

TEST_CASE("Recti addresses pixels", "[bounds][rect]")
{
  auto constexpr rect = Recti(0, 0, 640, 360);
  STATIC_CHECK(rect.max_x() == 640);
  STATIC_CHECK(rect.max_y() == 360);
  STATIC_CHECK(rect.area() == 230'400);
  STATIC_CHECK(rect.contains(639, 359));
  STATIC_CHECK(not rect.contains(640, 0));
  STATIC_CHECK(rect.contains(Recti(10, 10, 20, 20)));
  STATIC_CHECK(rect.intersection(Recti(600, 300, 100, 100)) == Recti(600, 300, 40, 60));
  STATIC_CHECK(not rect.intersects(Recti(640, 0, 10, 10)));
  STATIC_CHECK(rect.united(Recti(-10, -10, 5, 5)) == Recti(-10, -10, 650, 370));
  STATIC_CHECK(rect.translated(5, -5) == Recti(5, -5, 640, 360));
  STATIC_CHECK(rect.to_rect() == Rect(0.0, 0.0, 640.0, 360.0));
  STATIC_CHECK(Recti::from_size(16, 9) == Recti(0, 0, 16, 9));
  STATIC_CHECK(Recti(0, 0, -1, 5).is_empty());

  // the enclosing pixels of a fractional rect cover it entirely
  CHECK(Recti::enclosing(Rect(0.5, -0.5, 2.0, 1.0)) == Recti(0, -1, 3, 2));
  CHECK(Recti::enclosing(Rect(1.0, 1.0, 2.0, 2.0)) == Recti(1, 1, 2, 2));
  CHECK(fmt::format("{}", rect) == "(0, 0, 640, 360)");
  CHECK(std::hash<Recti> {}(rect) != std::hash<Recti> {}(Recti()));
}
