#include <w/common/stdint.h>
#include <w/common/aabb.h>
#include <w/common/angle.h>
#include <w/common/clip_space.h>
#include <w/common/color.h>
#include <w/common/euler_angles.h>
#include <w/common/frustum.h>
#include <w/common/matrix3x3.h>
#include <w/common/matrix4x4.h>
#include <w/common/plane.h>
#include <w/common/projection.h>
#include <w/common/quaternion.h>
#include <w/common/random.h>
#include <w/common/ray.h>
#include <w/common/sphere.h>
#include <w/common/transform.h>
#include <w/common/vector2.h>
#include <w/common/vector3.h>
#include <w/common/vector4.h>

#include <cmath>
#include <algorithm>
#include <array>
#include <limits>
#include <utility>

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/color_space.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace w;

// note: [whs31 : 2026-10-26]
// glm vectors keep their components in unions and its matrices index only through operator[]
// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)

// note: [whs31 : 2026-10-26]
// glm stores matrices by column, `g[c][r]`, and quaternions as `x, y, z, w`; every comparison
// converts through the helpers below, so a layout mistake fails the comparison instead of
// hiding in it

namespace
{
  auto constexpr iterations = 200;

  [[nodiscard]] glm::vec3 to_glm(Vector3 const v) { return {v.x(), v.y(), v.z()}; }

  [[nodiscard]] glm::vec4 to_glm(Vector4 const v) { return {v.x(), v.y(), v.z(), v.w()}; }

  [[nodiscard]] glm::quat to_glm(Quaternion const& q)
  {
    return glm::quat::wxyz(q.w(), q.x(), q.y(), q.z());
  }

  [[nodiscard]] glm::mat3 to_glm(Matrix3x3 const& m)
  {
    auto g = glm::mat3();
    for(auto r = 0; r < 3; ++r)
      for(auto c = 0; c < 3; ++c)
        g[c][r] = m.at(static_cast<usize>(r), static_cast<usize>(c));
    return g;
  }

  [[nodiscard]] glm::mat4 to_glm(Matrix4x4 const& m)
  {
    auto g = glm::mat4();
    for(auto r = 0; r < 4; ++r)
      for(auto c = 0; c < 4; ++c)
        g[c][r] = m.at(static_cast<usize>(r), static_cast<usize>(c));
    return g;
  }

  [[nodiscard]] Vector3 from_glm(glm::vec3 const v) { return {v.x, v.y, v.z}; }

  [[nodiscard]] Vector4 from_glm(glm::vec4 const v) { return {v.x, v.y, v.z, v.w}; }

  [[nodiscard]] Quaternion from_glm(glm::quat const q) { return {q.w, q.x, q.y, q.z}; }

  [[nodiscard]] Matrix3x3 from_glm(glm::mat3 const& g)
  {
    return Matrix3x3::from_rows(
      {g[0][0], g[1][0], g[2][0]},
      {g[0][1], g[1][1], g[2][1]},
      {g[0][2], g[1][2], g[2][2]}
    );
  }

  [[nodiscard]] Matrix4x4 from_glm(glm::mat4 const& g)
  {
    return Matrix4x4::from_rows(
      {g[0][0], g[1][0], g[2][0], g[3][0]},
      {g[0][1], g[1][1], g[2][1], g[3][1]},
      {g[0][2], g[1][2], g[2][2], g[3][2]},
      {g[0][3], g[1][3], g[2][3], g[3][3]}
    );
  }

  [[nodiscard]] double max_difference(Vector3 const a, Vector3 const b)
  {
    return std::max({std::abs(a.x() - b.x()), std::abs(a.y() - b.y()), std::abs(a.z() - b.z())});
  }

  [[nodiscard]] double max_difference(Vector4 const a, Vector4 const b)
  {
    return std::max<double>(max_difference(a.xyz(), b.xyz()), std::abs(a.w() - b.w()));
  }

  [[nodiscard]] double max_difference(Matrix3x3 const& a, Matrix3x3 const& b)
  {
    return std::max({
      max_difference(a.row(0), b.row(0)),
      max_difference(a.row(1), b.row(1)),
      max_difference(a.row(2), b.row(2)),
    });
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

  [[nodiscard]] double max_difference(Quaternion const& a, Quaternion const& b)
  {
    return std::max<double>(std::abs(a.w() - b.w()), max_difference(a.vector(), b.vector()));
  }

  // note: [whs31 : 2026-10-26]
  // q and -q are the same rotation, and neither library promises a sign
  [[nodiscard]] double rotation_difference(Quaternion const& a, Quaternion const& b)
  {
    return std::min(max_difference(a, b), max_difference(a, -b));
  }

  [[nodiscard]] double max_abs_element(Matrix4x4 const& m)
  {
    auto result = 0.0;
    for(auto const element : m.to_array())
      result = std::max<double>(result, std::abs(element));
    return result;
  }

  class Sampler
  {
   public:
    explicit Sampler(u64 const seed)
      : m_stream(RandomStream::from_seed(seed))
    {}

    [[nodiscard]] Real real(double const lo, double const hi)
    {
      return static_cast<Real>(this->m_stream.uniform(lo, hi));
    }

    [[nodiscard]] Angle degrees(double const lo, double const hi)
    {
      return Angle::from_degrees(this->real(lo, hi));
    }

    [[nodiscard]] Vector3 vector(double const lo, double const hi)
    {
      return {this->real(lo, hi), this->real(lo, hi), this->real(lo, hi)};
    }

    [[nodiscard]] Vector3 direction()
    {
      while(true) {
        auto const v = this->vector(-1.0, 1.0);
        if(v.length_squared() > Real {0.01} and v.length_squared() <= 1)
          return v.normalized();
      }
    }

    [[nodiscard]] Quaternion rotation()
    {
      return Quaternion::from_axis_angle(this->direction(), this->degrees(-180.0, 180.0));
    }

    [[nodiscard]] Matrix4x4 matrix(double const lo, double const hi)
    {
      return Matrix4x4::from_rows(
        {this->real(lo, hi), this->real(lo, hi), this->real(lo, hi), this->real(lo, hi)},
        {this->real(lo, hi), this->real(lo, hi), this->real(lo, hi), this->real(lo, hi)},
        {this->real(lo, hi), this->real(lo, hi), this->real(lo, hi), this->real(lo, hi)},
        {this->real(lo, hi), this->real(lo, hi), this->real(lo, hi), this->real(lo, hi)}
      );
    }

   private:
    RandomStream m_stream;
  };

  [[nodiscard]] glm::mat4 glm_euler(
    EulerOrder const order,
    Angle const x,
    Angle const y,
    Angle const z
  )
  {
    auto const rx = glm::eulerAngleX(x.radians());
    auto const ry = glm::eulerAngleY(y.radians());
    auto const rz = glm::eulerAngleZ(z.radians());
    switch(order) {
      case EulerOrder::XYZ: return rx * ry * rz;
      case EulerOrder::XZY: return rx * rz * ry;
      case EulerOrder::YXZ: return ry * rx * rz;
      case EulerOrder::YZX: return ry * rz * rx;
      case EulerOrder::ZXY: return rz * rx * ry;
      case EulerOrder::ZYX: return rz * ry * rx;
    }
    std::unreachable();
  }

  // note: [whs31 : 2026-10-26]
  // glm returns the angles in the order of the name; wlib returns them by axis
  [[nodiscard]] std::array<float, 3> glm_extract_by_axis(EulerOrder const order, glm::mat4 const& m)
  {
    auto t1 = 0.0F;
    auto t2 = 0.0F;
    auto t3 = 0.0F;
    switch(order) {
      case EulerOrder::XYZ: glm::extractEulerAngleXYZ(m, t1, t2, t3); return {t1, t2, t3};
      case EulerOrder::XZY: glm::extractEulerAngleXZY(m, t1, t2, t3); return {t1, t3, t2};
      case EulerOrder::YXZ: glm::extractEulerAngleYXZ(m, t1, t2, t3); return {t2, t1, t3};
      case EulerOrder::YZX: glm::extractEulerAngleYZX(m, t1, t2, t3); return {t3, t1, t2};
      case EulerOrder::ZXY: glm::extractEulerAngleZXY(m, t1, t2, t3); return {t2, t3, t1};
      case EulerOrder::ZYX: glm::extractEulerAngleZYX(m, t1, t2, t3); return {t3, t2, t1};
    }
    std::unreachable();
  }

  [[nodiscard]] constexpr usize middle_axis(EulerOrder const order) noexcept
  {
    switch(order) {
      case EulerOrder::YXZ: [[fallthrough]];
      case EulerOrder::ZXY: return 0;
      case EulerOrder::XYZ: [[fallthrough]];
      case EulerOrder::ZYX: return 1;
      case EulerOrder::XZY: [[fallthrough]];
      case EulerOrder::YZX: return 2;
    }
    std::unreachable();
  }

  template <EulerOrder Order>
  void check_euler_order(Sampler& sampler)
  {
    for(auto i = 0; i < iterations; ++i) {
      // note: [whs31 : 2026-10-26]
      // the middle angle stays away from ±90°, where only a sum of the other two is defined
      auto angles = std::array {
        sampler.degrees(-180.0, 180.0),
        sampler.degrees(-180.0, 180.0),
        sampler.degrees(-180.0, 180.0),
      };
      angles.at(middle_axis(Order)) = sampler.degrees(-85.0, 85.0);

      auto const q = Quaternion::from_euler<Order>(angles[0], angles[1], angles[2]);
      auto const expected = glm_euler(Order, angles[0], angles[1], angles[2]);
      CHECK(max_difference(q.to_rotation_matrix(), from_glm(glm::mat3(expected))) < 1e-5);

      auto const ours = q.template to_euler<Order>();
      auto const theirs = glm_extract_by_axis(Order, expected);
      for(auto axis = 0uz; axis < 3; ++axis) {
        CHECK(std::abs((ours.at(axis) - Angle::from_radians(theirs.at(axis))).radians()) < 1e-3);
        CHECK(std::abs((ours.at(axis) - angles.at(axis)).radians()) < 1e-3);
      }
    }
  }

  // note: [whs31 : 2026-10-26]
  // a point is inside when its glm clip coordinates satisfy -w <= x, y <= w and 0 <= z <= w;
  // points within a small margin of a face are skipped, as rounding may put them either side
  void check_containment(
    Frustum const& frustum,
    glm::mat4 const& view_projection,
    Vector3 const eye,
    Sampler& sampler
  )
  {
    for(auto j = 0; j < 100; ++j) {
      auto const point = eye + sampler.vector(-70.0, 70.0);
      auto const clip = view_projection * glm::vec4(to_glm(point), 1.0F);
      auto const margin = 1e-3F * std::abs(clip.w) + 1e-4F;
      auto const slack = std::min({
        clip.w - std::abs(clip.x),
        clip.w - std::abs(clip.y),
        clip.z,
        clip.w - clip.z,
      });
      if(std::abs(slack) < margin)
        continue;
      CHECK(frustum.contains(point) == (slack > 0));
    }
  }

  // note: [whs31 : 2026-10-26]
  // the corners of the clip volume, unprojected by glm, lie on three planes of the frustum each
  void check_corners(
    Frustum const& frustum,
    glm::mat4 const& inverse_view_projection,
    Vector3 const eye,
    ClipDepth const depth
  )
  {
    auto const near_depth = depth == ClipDepth::Standard ? 0.0F : 1.0F;
    for(auto const x : {-1.0F, 1.0F})
      for(auto const y : {-1.0F, 1.0F})
        for(auto const z : {0.0F, 1.0F}) {
          auto const homogeneous = inverse_view_projection * glm::vec4(x, y, z, 1.0F);
          auto const corner = from_glm(glm::vec3(homogeneous) / homogeneous.w);
          auto const tolerance = 1e-4 * (1 + corner.distance_to(eye));
          auto const depth_plane = z == near_depth ? frustum.near_plane() : frustum.far_plane();
          auto const side_plane = x < 0 ? frustum.left_plane() : frustum.right_plane();
          auto const height_plane = y < 0 ? frustum.bottom_plane() : frustum.top_plane();
          CHECK(std::abs(depth_plane.signed_distance(corner)) < tolerance);
          CHECK(std::abs(side_plane.signed_distance(corner)) < tolerance);
          CHECK(std::abs(height_plane.signed_distance(corner)) < tolerance);
        }
  }
}  // namespace

TEST_CASE("Matrix4x4 arithmetic matches glm", "[linalg][glm]")
{
  auto sampler = Sampler(0x4d34'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const a = sampler.matrix(-2.0, 2.0);
    auto const b = sampler.matrix(-2.0, 2.0);
    auto const v = Vector4(sampler.vector(-5.0, 5.0), sampler.real(-2.0, 2.0));

    CHECK(max_difference(a * b, from_glm(to_glm(a) * to_glm(b))) < 1e-5);
    CHECK(max_difference(a * v, from_glm(to_glm(a) * to_glm(v))) < 1e-5);
    CHECK(max_difference(a.transposed(), from_glm(glm::transpose(to_glm(a)))) == 0.0);
    CHECK(std::abs(a.determinant() - glm::determinant(to_glm(a))) < 1e-4);
    CHECK(max_difference(a.linear(), from_glm(glm::mat3(to_glm(a)))) == 0.0);
  }
}

TEST_CASE("Matrix4x4 inverse matches glm", "[linalg][glm]")
{
  auto sampler = Sampler(0x4d34'0002);
  auto checked = 0;
  while(checked < iterations) {
    auto const m = sampler.matrix(-1.0, 1.0);
    // note: [whs31 : 2026-10-26]
    // with entries in [-1, 1], a determinant away from zero bounds the condition number, so the
    // single-precision inverses of both libraries agree to a known tolerance
    if(std::abs(m.determinant()) < Real {0.1})
      continue;
    ++checked;
    auto const expected = from_glm(glm::inverse(to_glm(m)));
    auto const inverse = m.inverse();
    CHECK(max_difference(inverse, expected) / (1 + max_abs_element(expected)) < 1e-4);
    CHECK(max_difference(m * inverse, Matrix4x4::identity()) < 1e-4);
  }

  auto const affine = Matrix4x4::
    from_affine(sampler.rotation().to_rotation_matrix(), sampler.vector(-10.0, 10.0));
  CHECK(max_difference(affine.inverse(), from_glm(glm::affineInverse(to_glm(affine)))) < 1e-5);
}

TEST_CASE("Matrix3x3 arithmetic matches glm", "[linalg][glm]")
{
  auto sampler = Sampler(0x4d33'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const a = Matrix3x3::
      from_rows(sampler.vector(-1.0, 1.0), sampler.vector(-1.0, 1.0), sampler.vector(-1.0, 1.0));
    auto const b = Matrix3x3::
      from_rows(sampler.vector(-1.0, 1.0), sampler.vector(-1.0, 1.0), sampler.vector(-1.0, 1.0));
    auto const v = sampler.vector(-5.0, 5.0);

    CHECK(max_difference(a * b, from_glm(to_glm(a) * to_glm(b))) < 1e-5);
    CHECK(max_difference(a * v, from_glm(to_glm(a) * to_glm(v))) < 1e-5);
    CHECK(std::abs(a.determinant() - glm::determinant(to_glm(a))) < 1e-5);
    if(std::abs(a.determinant()) > Real {0.1})
      CHECK(max_difference(a.inverse(), from_glm(glm::inverse(to_glm(a)))) < 1e-3);
  }
}

TEST_CASE("Perspective projections match glm", "[linalg][glm][projection]")
{
  auto sampler = Sampler(0x5052'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const fov = sampler.degrees(20.0, 120.0);
    auto const aspect = sampler.real(0.5, 2.5);
    auto const z_near = sampler.real(0.01, 1.0);
    auto const z_far = z_near + sampler.real(10.0, 1000.0);
    auto const f = fov.radians();

    auto const rh = perspective(fov, aspect, z_near, z_far, Handedness::Right);
    auto const lh = perspective(fov, aspect, z_near, z_far, Handedness::Left);
    CHECK(max_difference(rh, from_glm(glm::perspectiveRH_ZO(f, aspect, z_near, z_far))) < 1e-5);
    CHECK(max_difference(lh, from_glm(glm::perspectiveLH_ZO(f, aspect, z_near, z_far))) < 1e-5);

    // note: [whs31 : 2026-10-26]
    // reversed depth is the standard projection with near and far swapped
    auto const
      reversed = perspective(fov, aspect, z_near, z_far, Handedness::Right, ClipDepth::Reversed);
    auto const reversed_near = z_far;
    auto const reversed_far = z_near;
    CHECK(
      max_difference(
        reversed,
        from_glm(glm::perspectiveRH_ZO(f, aspect, reversed_near, reversed_far))
      )
      < 1e-4
    );

    auto constexpr infinity = std::numeric_limits<Real>::infinity();
    auto const infinite_rh = perspective(fov, aspect, z_near, infinity, Handedness::Right);
    auto const infinite_lh = perspective(fov, aspect, z_near, infinity, Handedness::Left);
    CHECK(
      max_difference(infinite_rh, from_glm(glm::infinitePerspectiveRH_ZO(f, aspect, z_near))) < 1e-5
    );
    CHECK(
      max_difference(infinite_lh, from_glm(glm::infinitePerspectiveLH_ZO(f, aspect, z_near))) < 1e-5
    );
  }
}

TEST_CASE("Orthographic projections match glm", "[linalg][glm][projection]")
{
  auto sampler = Sampler(0x5052'0002);
  for(auto i = 0; i < iterations; ++i) {
    auto const left = sampler.real(-100.0, -1.0);
    auto const right = sampler.real(1.0, 100.0);
    auto const bottom = sampler.real(-100.0, -1.0);
    auto const top = sampler.real(1.0, 100.0);
    auto const z_near = sampler.real(-10.0, 1.0);
    auto const z_far = z_near + sampler.real(1.0, 500.0);

    auto const rh = orthographic(left, right, bottom, top, z_near, z_far, Handedness::Right);
    auto const lh = orthographic(left, right, bottom, top, z_near, z_far, Handedness::Left);
    auto const reversed = orthographic(
      left,
      right,
      bottom,
      top,
      z_near,
      z_far,
      Handedness::Left,
      ClipDepth::Reversed
    );
    CHECK(
      max_difference(rh, from_glm(glm::orthoRH_ZO(left, right, bottom, top, z_near, z_far))) < 1e-5
    );
    CHECK(
      max_difference(lh, from_glm(glm::orthoLH_ZO(left, right, bottom, top, z_near, z_far))) < 1e-5
    );
    auto const reversed_near = z_far;
    auto const reversed_far = z_near;
    CHECK(
      max_difference(
        reversed,
        from_glm(glm::orthoLH_ZO(left, right, bottom, top, reversed_near, reversed_far))
      )
      < 1e-5
    );
  }
}

TEST_CASE("look_at matches glm", "[linalg][glm][projection]")
{
  auto sampler = Sampler(0x4c4b'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const eye = sampler.vector(-50.0, 50.0);
    auto const target = eye + sampler.direction() * sampler.real(1.0, 20.0);
    auto const up = sampler.direction();
    if((target - eye).normalized().cross(up).length() < Real {0.1})
      continue;

    auto const rh = look_at_rh(eye, target, up);
    auto const lh = look_at_lh(eye, target, up);
    CHECK(
      max_difference(rh, from_glm(glm::lookAtRH(to_glm(eye), to_glm(target), to_glm(up)))) < 1e-4
    );
    CHECK(
      max_difference(lh, from_glm(glm::lookAtLH(to_glm(eye), to_glm(target), to_glm(up)))) < 1e-4
    );
    CHECK(max_difference(look_at(eye, target, up, Handedness::Right), rh) == 0.0);
  }
}

TEST_CASE("Quaternion algebra matches glm", "[linalg][glm][quaternion]")
{
  auto sampler = Sampler(0x5155'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const axis = sampler.direction();
    auto const angle = sampler.degrees(-180.0, 180.0);
    auto const a = Quaternion::from_axis_angle(axis, angle);
    auto const b = sampler.rotation();
    auto const v = sampler.vector(-10.0, 10.0);

    CHECK(max_difference(a, from_glm(glm::angleAxis(angle.radians(), to_glm(axis)))) < 1e-6);
    CHECK(max_difference(a * b, from_glm(to_glm(a) * to_glm(b))) < 1e-6);
    CHECK(max_difference(a.rotate(v), from_glm(to_glm(a) * to_glm(v))) < 1e-5);
    CHECK(max_difference(a.inverse(), from_glm(glm::inverse(to_glm(a)))) < 1e-6);
    CHECK(max_difference(a.to_rotation_matrix(), from_glm(glm::mat3_cast(to_glm(a)))) < 1e-6);
    CHECK(max_difference(a.to_matrix4(), from_glm(glm::mat4_cast(to_glm(a)))) < 1e-6);

    auto const matrix = a.to_rotation_matrix();
    CHECK(
      rotation_difference(
        Quaternion::from_rotation_matrix(matrix),
        from_glm(glm::quat_cast(to_glm(matrix)))
      )
      < 1e-5
    );
    CHECK(rotation_difference(Quaternion::from_rotation_matrix(matrix), a) < 1e-5);

    // note: [whs31 : 2026-10-26]
    // glm::angle is the angle of a single rotation, so the angle between two rotations is the
    // angle of the rotation that turns one into the other
    auto const between = glm::angle(glm::inverse(to_glm(a)) * to_glm(b));
    auto const expected = between > glm::pi<float>() ? glm::two_pi<float>() - between : between;
    CHECK(std::abs(a.angle_to(b).radians() - expected) < 1e-3);
  }
}

TEST_CASE("Quaternion interpolation matches glm", "[linalg][glm][quaternion]")
{
  auto sampler = Sampler(0x5155'0002);
  for(auto i = 0; i < iterations; ++i) {
    auto const a = sampler.rotation();
    auto const b = sampler.rotation();
    auto const t = sampler.real(0.0, 1.0);

    // note: [whs31 : 2026-10-26]
    // glm::slerp takes the short arc, and so does wlib; glm::lerp keeps the sign of b, so the
    // nlerp comparison flips b into a's hemisphere first
    CHECK(rotation_difference(a.slerp(b, t), from_glm(glm::slerp(to_glm(a), to_glm(b), t))) < 1e-4);
    CHECK(rotation_difference(slerp(a, b, t), a.slerp(b, t)) == 0.0);

    auto const near_b = a.dot(b) < 0 ? -b : b;
    auto const expected = glm::normalize(glm::lerp(to_glm(a), to_glm(near_b), t));
    CHECK(rotation_difference(a.nlerp(b, t), from_glm(expected)) < 1e-5);
    CHECK(rotation_difference(a.slerp(b, 0), a) < 1e-6);
    CHECK(rotation_difference(a.slerp(b, 1), b) < 1e-5);
  }
}

TEST_CASE("Quaternion directions match glm", "[linalg][glm][quaternion]")
{
  auto sampler = Sampler(0x5155'0003);
  for(auto i = 0; i < iterations; ++i) {
    auto const from = sampler.direction();
    auto const to = sampler.direction();
    if(from.dot(to) < Real {-0.99})
      continue;
    auto const turn = Quaternion::from_to(from * 3, to * Real {0.5});
    CHECK(rotation_difference(turn, from_glm(glm::rotation(to_glm(from), to_glm(to)))) < 1e-4);
    CHECK(max_difference(turn.rotate(from), to) < 1e-5);

    auto const forward = sampler.direction();
    auto const up = sampler.direction();
    if(forward.cross(up).length() < Real {0.1})
      continue;
    auto const rh = Quaternion::look_rotation(forward, up, Handedness::Right);
    auto const lh = Quaternion::look_rotation(forward, up, Handedness::Left);
    CHECK(rotation_difference(rh, from_glm(glm::quatLookAtRH(to_glm(forward), to_glm(up)))) < 1e-4);
    CHECK(rotation_difference(lh, from_glm(glm::quatLookAtLH(to_glm(forward), to_glm(up)))) < 1e-4);
    CHECK(max_difference(rh.rotate(-Vector3::unit_z()), forward) < 1e-5);
    CHECK(max_difference(lh.rotate(Vector3::unit_z()), forward) < 1e-5);
  }

  auto const opposite = Quaternion::from_to(Vector3::unit_x(), -Vector3::unit_x());
  CHECK(max_difference(opposite.rotate(Vector3::unit_x()), -Vector3::unit_x()) < 1e-6);
}

TEST_CASE("Euler angles match glm in every order", "[linalg][glm][euler]")
{
  auto sampler = Sampler(0x4555'0001);
  check_euler_order<EulerOrder::XYZ>(sampler);
  check_euler_order<EulerOrder::XZY>(sampler);
  check_euler_order<EulerOrder::YXZ>(sampler);
  check_euler_order<EulerOrder::YZX>(sampler);
  check_euler_order<EulerOrder::ZXY>(sampler);
  check_euler_order<EulerOrder::ZYX>(sampler);

  // note: [whs31 : 2026-10-26]
  // glm's yawPitchRoll is the y-up editor order: yaw about y, pitch about x, roll about z
  for(auto i = 0; i < iterations; ++i) {
    auto const yaw = sampler.degrees(-180.0, 180.0);
    auto const pitch = sampler.degrees(-85.0, 85.0);
    auto const roll = sampler.degrees(-180.0, 180.0);
    auto const q = Quaternion::from_euler<EulerOrder::YXZ>(pitch, yaw, roll);
    auto const expected = glm::yawPitchRoll(yaw.radians(), pitch.radians(), roll.radians());
    CHECK(max_difference(q.to_rotation_matrix(), from_glm(glm::mat3(expected))) < 1e-5);
    CHECK(
      max_difference(
        q.to_matrix4(),
        from_glm(glm::eulerAngleYXZ(yaw.radians(), pitch.radians(), roll.radians()))
      )
      < 1e-5
    );

    auto const aerospace = Quaternion::from_yaw_pitch_roll(EulerAngles(roll, pitch, yaw));
    CHECK(
      max_difference(
        aerospace.to_matrix4(),
        from_glm(glm::eulerAngleZYX(yaw.radians(), pitch.radians(), roll.radians()))
      )
      < 1e-5
    );
  }
}

TEST_CASE("Transform matrices and decomposition match glm", "[linalg][glm][transform]")
{
  auto sampler = Sampler(0x5452'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const position = sampler.vector(-100.0, 100.0);
    auto const rotation = sampler.rotation();
    auto const scale = sampler.vector(0.1, 5.0);
    auto const transform = Transform(position, rotation, scale);

    auto const expected = glm::translate(glm::mat4(1.0F), to_glm(position))
                        * glm::mat4_cast(to_glm(rotation))
                        * glm::scale(glm::mat4(1.0F), to_glm(scale));
    auto const matrix = transform.to_matrix();
    CHECK(max_difference(matrix, from_glm(expected)) < 1e-4);

    auto glm_scale = glm::vec3();
    auto glm_rotation = glm::quat();
    auto glm_translation = glm::vec3();
    auto glm_skew = glm::vec3();
    auto glm_perspective = glm::vec4();
    REQUIRE(
      glm::decompose(expected, glm_scale, glm_rotation, glm_translation, glm_skew, glm_perspective)
    );

    REQUIRE(Transform::decompose(matrix).has_value());
    auto const decomposed = Transform::decompose(matrix).value_or(Transform());
    CHECK(max_difference(decomposed.position(), from_glm(glm_translation)) < 1e-4);
    CHECK(max_difference(decomposed.scale(), from_glm(glm_scale)) < 1e-4);
    CHECK(rotation_difference(decomposed.rotation(), from_glm(glm_rotation)) < 1e-4);
    CHECK(max_difference(decomposed.to_matrix(), matrix) < 1e-3);

    auto const point = sampler.vector(-10.0, 10.0);
    CHECK(
      max_difference(
        transform.transform_point(point),
        from_glm(glm::vec3(expected * glm::vec4(to_glm(point), 1.0F)))
      )
      < 1e-3
    );
    // note: [whs31 : 2026-10-26]
    // the inverse of a non-uniformly scaled transform is not a TRS transform, so the inverse is
    // compared with a uniform scale
    auto const uniform = transform.with_scale(Vector3::splat(scale.x()));
    auto const uniform_matrix = glm::inverse(to_glm(uniform.to_matrix()));
    CHECK(max_difference(uniform.inverse().to_matrix(), from_glm(uniform_matrix)) < 1e-3);
  }
}

TEST_CASE("Frustum planes agree with glm clip space", "[linalg][glm][bounds]")
{
  auto sampler = Sampler(0x4652'0001);
  for(auto i = 0; i < 50; ++i) {
    auto const handedness = i % 2 == 0 ? Handedness::Right : Handedness::Left;
    auto const depth = i % 4 < 2 ? ClipDepth::Standard : ClipDepth::Reversed;
    auto const eye = sampler.vector(-20.0, 20.0);
    auto const view = look_at(eye, eye + sampler.direction(), Vector3::unit_y(), handedness);
    auto const projection = i % 3 == 0 ? orthographic(-10, 10, -6, 6, 0.5, 60, handedness, depth)
                                       : perspective(
                                           sampler.degrees(40.0, 90.0),
                                           sampler.real(1.0, 2.0),
                                           0.5,
                                           60,
                                           handedness,
                                           depth
                                         );
    auto const view_projection = projection * view;
    auto const frustum = Frustum::from_view_projection(view_projection, depth);
    auto const glm_view_projection = to_glm(view_projection);

    check_containment(frustum, glm_view_projection, eye, sampler);
    check_corners(frustum, glm::inverse(glm_view_projection), eye, depth);
  }
}

TEST_CASE("Picking rays match glm unprojection", "[linalg][glm][bounds]")
{
  auto sampler = Sampler(0x5241'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const depth = i % 2 == 0 ? ClipDepth::Standard : ClipDepth::Reversed;
    auto const eye = sampler.vector(-20.0, 20.0);
    auto const view = look_at_rh(eye, eye + sampler.direction(), Vector3::unit_y());
    auto const
      projection = perspective(Angle::from_degrees(60), 1.5, 0.1, 100, Handedness::Right, depth);
    auto const inverse = (projection * view).inverse();
    auto const ndc = Vector2(sampler.real(-1.0, 1.0), sampler.real(-1.0, 1.0));
    auto const ray = Ray::from_ndc(ndc, inverse, depth);

    auto const unproject = [&](float const z) {
      auto const p = glm::inverse(to_glm(projection * view)) * glm::vec4(ndc.x(), ndc.y(), z, 1.0F);
      return from_glm(glm::vec3(p) / p.w);
    };
    auto const near_point = unproject(depth == ClipDepth::Standard ? 0.0F : 1.0F);
    auto const far_point = unproject(depth == ClipDepth::Standard ? 1.0F : 0.0F);
    CHECK(max_difference(ray.origin(), near_point) < 1e-3);
    CHECK(max_difference(ray.direction(), (far_point - near_point).normalized()) < 1e-3);
  }
}

TEST_CASE("Ray intersections match glm", "[linalg][glm][bounds]")
{
  auto sampler = Sampler(0x5249'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const ray = Ray(sampler.vector(-10.0, 10.0), sampler.direction());
    auto const glm_origin = to_glm(ray.origin());
    auto const glm_direction = to_glm(ray.direction());

    auto const plane_point = sampler.vector(-10.0, 10.0);
    auto const plane_normal = sampler.direction();
    auto glm_distance = 0.0F;
    auto const plane = Plane::from_point_normal(plane_point, plane_normal);
    if(glm::intersectRayPlane(
         glm_origin,
         glm_direction,
         to_glm(plane_point),
         to_glm(plane_normal),
         glm_distance
       )) {
      REQUIRE(ray.intersect(plane).has_value());
      auto const t = ray.intersect(plane).value_or(0);
      CHECK(std::abs(t - glm_distance) / (1 + glm_distance) < 1e-4);
    }

    auto const center = sampler.vector(-10.0, 10.0);
    auto const radius = sampler.real(0.5, 5.0);
    // note: [whs31 : 2026-10-26]
    // from inside a sphere glm reports where the ray leaves and wlib reports 0, so the origin
    // stays outside
    if(ray.origin().distance_to(center) > radius * Real {1.01}) {
      auto const sphere_hit = glm::intersectRaySphere(
        glm_origin,
        glm_direction,
        to_glm(center),
        radius * radius,
        glm_distance
      );
      auto const t = ray.intersect(Sphere(center, radius));
      CHECK(t.has_value() == sphere_hit);
      if(sphere_hit and t.has_value())
        CHECK(std::abs(*t - glm_distance) / (1 + glm_distance) < 1e-3);
    }

    auto const a = sampler.vector(-10.0, 10.0);
    auto const b = sampler.vector(-10.0, 10.0);
    auto const c = sampler.vector(-10.0, 10.0);
    auto barycentric = glm::vec2();
    auto const glm_hit = glm::intersectRayTriangle(
      glm_origin,
      glm_direction,
      to_glm(a),
      to_glm(b),
      to_glm(c),
      barycentric,
      glm_distance
    );
    auto const hit = ray.intersect_triangle(a, b, c);
    // note: [whs31 : 2026-10-26]
    // glm also reports hits behind the origin, and a ray grazing an edge may fall either side
    auto const grazing = glm_hit
                     and std::min({barycentric.x, barycentric.y, 1 - barycentric.x - barycentric.y})
                           < 1e-4F;
    if(grazing)
      continue;
    CHECK(hit.has_value() == (glm_hit and glm_distance >= 0));
    if(hit.has_value() and glm_hit) {
      CHECK(std::abs(hit->t - glm_distance) / (1 + glm_distance) < 1e-4);
      CHECK(std::abs(hit->u - barycentric.x) < 1e-4);
      CHECK(std::abs(hit->v - barycentric.y) < 1e-4);
    }
  }
}

TEST_CASE("Color conversions match glm", "[color][glm]")
{
  auto sampler = Sampler(0x434c'0001);
  for(auto i = 0; i < iterations; ++i) {
    auto const value = sampler.real(0.0, 1.0);
    CHECK(std::abs(srgb_to_linear(value) - glm::convertSRGBToLinear(glm::vec3(value)).x) < 1e-5);
    CHECK(std::abs(linear_to_srgb(value) - glm::convertLinearToSRGB(glm::vec3(value)).x) < 1e-5);

    auto const color = Rgba8(
      static_cast<u8>(sampler.real(0.0, 255.0)),
      static_cast<u8>(sampler.real(0.0, 255.0)),
      static_cast<u8>(sampler.real(0.0, 255.0))
    );
    auto const hsv = color.to_hsv();
    auto const expected = glm::hsvColor(glm::vec3(color.r(), color.g(), color.b()) / 255.0F);
    CHECK(std::abs(hsv.value - expected.z) < 1e-5);
    CHECK(std::abs(hsv.saturation - expected.y) < 1e-5);
    if(hsv.saturation > 1e-3F) {
      auto const hue = std::remainder(hsv.hue.degrees() - expected.x, 360.0F);
      CHECK(std::abs(hue) < 1e-2);
    }

    auto const pick = Hsv {
      .hue = sampler.degrees(0.0, 359.9),
      .saturation = sampler.real(0.01, 1.0),
      .value = sampler.real(0.0, 1.0),
    };
    auto const linear = Rgba::from_hsv(pick);
    auto const encoded = glm::rgbColor(glm::vec3(pick.hue.degrees(), pick.saturation, pick.value));
    CHECK(std::abs(linear_to_srgb(linear.r()) - encoded.r) < 1e-4);
    CHECK(std::abs(linear_to_srgb(linear.g()) - encoded.g) < 1e-4);
    CHECK(std::abs(linear_to_srgb(linear.b()) - encoded.b) < 1e-4);
  }
}

// NOLINTEND(cppcoreguidelines-pro-type-union-access,cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
