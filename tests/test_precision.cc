#include <chrono>
#include <cmath>
#include <concepts>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>
#include <fmt/format.h>

#include <w/common/all.h>
#include <w/common/precision.h>
#include <w/common/projection.h>

using namespace w;

// note: [whs31 : 2026-10-26]
// explicit instantiations compile every member of every template in both precisions, which a test
// that calls only some of them would not, and a template cannot emit them
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WLIB_INSTANTIATE(TEMPLATE)                                                                 \
  template class w::common::TEMPLATE<float>;  /* NOLINT(bugprone-macro-parentheses) */            \
  template class w::common::TEMPLATE<double>; /* NOLINT(bugprone-macro-parentheses) */

WLIB_INSTANTIATE(BasicAcceleration)
WLIB_INSTANTIATE(BasicAngle)
WLIB_INSTANTIATE(BasicAngularAcceleration)
WLIB_INSTANTIATE(BasicAngularVelocity)
WLIB_INSTANTIATE(BasicCapacitance)
WLIB_INSTANTIATE(BasicCharge)
WLIB_INSTANTIATE(BasicConductance)
WLIB_INSTANTIATE(BasicCurrent)
WLIB_INSTANTIATE(BasicDensity)
WLIB_INSTANTIATE(BasicDistance)
WLIB_INSTANTIATE(BasicEnergy)
WLIB_INSTANTIATE(BasicForce)
WLIB_INSTANTIATE(BasicFrequency)
WLIB_INSTANTIATE(BasicMagneticFluxDensity)
WLIB_INSTANTIATE(BasicMass)
WLIB_INSTANTIATE(BasicMomentOfInertia)
WLIB_INSTANTIATE(BasicPower)
WLIB_INSTANTIATE(BasicPressure)
WLIB_INSTANTIATE(BasicResistance)
WLIB_INSTANTIATE(BasicTemperature)
WLIB_INSTANTIATE(BasicTorque)
WLIB_INSTANTIATE(BasicVelocity)
WLIB_INSTANTIATE(BasicVoltage)
WLIB_INSTANTIATE(BasicVector2)
WLIB_INSTANTIATE(BasicVector3)
WLIB_INSTANTIATE(BasicVector4)
WLIB_INSTANTIATE(BasicMatrix3x3)
WLIB_INSTANTIATE(BasicMatrix4x4)
WLIB_INSTANTIATE(BasicQuaternion)
WLIB_INSTANTIATE(BasicEulerAngles)
WLIB_INSTANTIATE(BasicTransform)
WLIB_INSTANTIATE(BasicPlane)
WLIB_INSTANTIATE(BasicSphere)
WLIB_INSTANTIATE(BasicAABB)
WLIB_INSTANTIATE(BasicRay)
WLIB_INSTANTIATE(BasicFrustum)
WLIB_INSTANTIATE(BasicRect)

#undef WLIB_INSTANTIATE

namespace
{
  template <template <typename> typename Basic>
  concept WidensImplicitly = std::is_convertible_v<Basic<float>, Basic<double>>
                         and not std::is_convertible_v<Basic<double>, Basic<float>>
                         and std::is_constructible_v<Basic<float>, Basic<double>>;
}  // namespace

TEST_CASE("Every alias is the float instantiation of its template", "[precision]")
{
  STATIC_CHECK(std::same_as<Angle, BasicAngle<float>>);
  STATIC_CHECK(std::same_as<Distance, BasicDistance<float>>);
  STATIC_CHECK(std::same_as<Vector3, BasicVector3<float>>);
  STATIC_CHECK(std::same_as<Quaternion, BasicQuaternion<float>>);
  STATIC_CHECK(std::same_as<Matrix4x4, BasicMatrix4x4<float>>);
  STATIC_CHECK(std::same_as<TexelSnap, BasicTexelSnap<float>>);
  STATIC_CHECK(std::same_as<TriangleHit, BasicTriangleHit<float>>);
  STATIC_CHECK(std::same_as<decltype(1.5_deg), Angle>);
  STATIC_CHECK(sizeof(BasicVector3<double>) == 3 * sizeof(double));
  STATIC_CHECK(sizeof(BasicMatrix4x4<double>) == 16 * sizeof(double));
}

TEST_CASE("float widens to double implicitly and narrows only explicitly", "[precision]")
{
  STATIC_CHECK(WidensImplicitly<BasicAcceleration>);
  STATIC_CHECK(WidensImplicitly<BasicAngle>);
  STATIC_CHECK(WidensImplicitly<BasicDistance>);
  STATIC_CHECK(WidensImplicitly<BasicTemperature>);
  STATIC_CHECK(WidensImplicitly<BasicVelocity>);
  STATIC_CHECK(WidensImplicitly<BasicVector2>);
  STATIC_CHECK(WidensImplicitly<BasicVector3>);
  STATIC_CHECK(WidensImplicitly<BasicVector4>);
  STATIC_CHECK(WidensImplicitly<BasicMatrix3x3>);
  STATIC_CHECK(WidensImplicitly<BasicMatrix4x4>);
  STATIC_CHECK(WidensImplicitly<BasicQuaternion>);
  STATIC_CHECK(WidensImplicitly<BasicEulerAngles>);
  STATIC_CHECK(WidensImplicitly<BasicTransform>);
  STATIC_CHECK(WidensImplicitly<BasicPlane>);
  STATIC_CHECK(WidensImplicitly<BasicSphere>);
  STATIC_CHECK(WidensImplicitly<BasicAABB>);
  STATIC_CHECK(WidensImplicitly<BasicRay>);
  STATIC_CHECK(WidensImplicitly<BasicFrustum>);
  STATIC_CHECK(WidensImplicitly<BasicRect>);

  auto constexpr single = Angle::from_radians(0.5);
  BasicAngle<double> constexpr widened = single;
  STATIC_CHECK(widened.radians() == 0.5);
  STATIC_CHECK(Angle(BasicAngle<double>::from_radians(0.25)).radians() == 0.25F);
}

TEST_CASE("Mixing precisions computes in double", "[precision]")
{
  auto constexpr sum = Distance::from_meters(1.0) + BasicDistance<double>::from_meters(0.1);
  STATIC_CHECK(std::same_as<std::remove_cvref_t<decltype(sum)>, BasicDistance<double>>);

  auto const offset = Vector3(1.0, 2.0, 3.0) + BasicVector3<double>(0.5, 0.5, 0.5);
  STATIC_CHECK(std::same_as<std::remove_cvref_t<decltype(offset)>, BasicVector3<double>>);
  REQUIRE(offset == BasicVector3<double>(1.5, 2.5, 3.5));
}

TEST_CASE("double keeps the digits that float loses", "[precision]")
{
  // a position 10 000 km away with a millimetre offset
  auto constexpr far = 1e7;
  auto constexpr offset = 1e-3;

  auto const precise = BasicVector3<double>(far, 0.0, 0.0) + BasicVector3<double>(offset, 0.0, 0.0);
  auto const single = Vector3(1e7F, 0.0F, 0.0F) + Vector3(1e-3F, 0.0F, 0.0F);
  REQUIRE(std::abs(precise.x() - far - offset) < 1e-9);
  REQUIRE(single.x() - 1e7F == 0.0F);

  auto const distance = BasicDistance<double>::from_kilometers(10'000.0)
                      + BasicDistance<double>::from_millimeters(1.0);
  REQUIRE(std::abs(distance.as_m() - far - offset) < 1e-9);

  // a long duration does not lose digits before the division
  auto const velocity = BasicDistance<double>::from_meters(1e9) / std::chrono::hours(24 * 365);
  REQUIRE(velocity.as_mps() == 1e9 / (24.0 * 365.0 * 3'600.0));
}

TEST_CASE("Linear algebra composes in double", "[precision]")
{
  using Q = BasicQuaternion<double>;
  auto const rotation = Q::from_axis_angle(BasicVector3<double>::unit_z(), 90.0_deg);
  auto const rotated = rotation.rotate(BasicVector3<double>::unit_x());
  REQUIRE(std::abs(rotated.y() - 1.0) < 1e-12);

  auto const projection = perspective(
    BasicAngle<double>::from_degrees(60.0),
    1.5,
    0.1,
    1e6,
    Handedness::Right
  );
  REQUIRE(projection.is_finite());
  REQUIRE(fmt::format("{:.2f}", BasicVector2<double>(0.5, 1.25)) == "(0.50, 1.25)");
}
