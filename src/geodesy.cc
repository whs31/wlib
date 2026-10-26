#include <w/common/geodesy.h>

#include <cmath>

#include <memory>
#include <ostream>
#include <string_view>
#include <utility>

#include <w/common/bit.h>
#include <w/common/check.h>
#include <w/common/error.h>
#include <w/common/memo.h>

W_COMMON_NAMESPACE_BEGIN

namespace
{
  // bowring's iteration converges cubically; three steps reach rounding level near the surface
  auto constexpr latitude_iterations = 3;

  // note: [whs31 : 2026-10-26]
  // earth-centred coordinates are millions of metres, where a float steps by half a metre, so the
  // frame is computed in double whatever the precision of Vector3
  using _geodesy_impl::Ecef;
  using _geodesy_impl::NedAxes;

  [[nodiscard]] Ecef operator+(Ecef const& lhs, Ecef const& rhs) noexcept
  {
    return {.x = lhs.x + rhs.x, .y = lhs.y + rhs.y, .z = lhs.z + rhs.z};
  }

  [[nodiscard]] Ecef operator-(Ecef const& lhs, Ecef const& rhs) noexcept
  {
    return {.x = lhs.x - rhs.x, .y = lhs.y - rhs.y, .z = lhs.z - rhs.z};
  }

  [[nodiscard]] Ecef operator*(Ecef const& lhs, double const rhs) noexcept
  {
    return {.x = lhs.x * rhs, .y = lhs.y * rhs, .z = lhs.z * rhs};
  }

  [[nodiscard]] double dot(Ecef const& lhs, Ecef const& rhs) noexcept
  {
    return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
  }

  // the components of an earth-centred vector along the axes
  [[nodiscard]] Ecef to_ned(NedAxes const& axes, Ecef const& v) noexcept
  {
    return {.x = dot(axes.north, v), .y = dot(axes.east, v), .z = dot(axes.down, v)};
  }

  // the earth-centred vector of components along the axes
  [[nodiscard]] Ecef from_ned(NedAxes const& axes, Ecef const& v) noexcept
  {
    return axes.north * v.x + axes.east * v.y + axes.down * v.z;
  }

  [[nodiscard]] Ecef ecef_of(GeodeticPosition const& position) noexcept
  {
    auto const sin_latitude = position.latitude.sin();
    auto const prime_vertical_radius = wgs84::semi_major_axis
                                     / std::sqrt(
                                         1.0
                                         - wgs84::eccentricity_squared * sin_latitude * sin_latitude
                                     );
    auto const altitude = static_cast<double>(position.altitude_msl.as_m());
    auto const equatorial = (prime_vertical_radius + altitude) * position.latitude.cos();
    return {
      .x = equatorial * position.longitude.cos(),
      .y = equatorial * position.longitude.sin(),
      .z = (prime_vertical_radius * (1.0 - wgs84::eccentricity_squared) + altitude) * sin_latitude,
    };
  }

  // the axes north, east and down of the local frame at a latitude and longitude
  [[nodiscard]] NedAxes ned_axes(Latitude const latitude, Longitude const longitude) noexcept
  {
    auto const sin_latitude = latitude.sin();
    auto const cos_latitude = latitude.cos();
    auto const sin_longitude = longitude.sin();
    auto const cos_longitude = longitude.cos();
    return {
      .north =
        {.x = -sin_latitude * cos_longitude, .y = -sin_latitude * sin_longitude, .z = cos_latitude},
      .east = {.x = -sin_longitude, .y = cos_longitude, .z = 0.0},
      .down =
        {.x = -cos_latitude * cos_longitude,
         .y = -cos_latitude * sin_longitude,
         .z = -sin_latitude},
    };
  }

  // latitude and longitude of the ellipsoid normal through a point, by bowring's iteration on the
  // parametric latitude, which stays well-conditioned at the poles
  [[nodiscard]] std::pair<Latitude, Longitude> normal_through(Ecef const& ecef) noexcept
  {
    // note: [whs31 : 2026-10-14]
    // a simulation asks for it on every terrain query, so the iteration carries the latitude as
    // the direction (north, equatorial) instead of an angle: tan(parametric) = (1 - f)
    // tan(latitude) then needs one square root where the angles took two atan2, a sin and a cos.
    // only the final latitude is an angle
    auto const equatorial = std::sqrt((ecef.x * ecef.x) + (ecef.y * ecef.y));
    auto north = ecef.z;
    auto across = equatorial * (1.0 - wgs84::eccentricity_squared);
    for(auto iteration = 0; iteration < latitude_iterations; ++iteration) {
      auto const parametric_north = (1.0 - wgs84::flattening) * north;
      auto const length = std::sqrt((parametric_north * parametric_north) + (across * across));
      auto const sin_parametric = parametric_north / length;
      auto const cos_parametric = across / length;
      north = ecef.z
            + wgs84::second_eccentricity_squared * wgs84::semi_minor_axis * sin_parametric
                * sin_parametric * sin_parametric;
      across = equatorial
             - wgs84::eccentricity_squared * wgs84::semi_major_axis * cos_parametric
                 * cos_parametric * cos_parametric;
    }
    return {
      Latitude::from_radians(std::atan2(north, across)),
      Longitude::from_radians(std::atan2(ecef.y, ecef.x)),
    };
  }

  // note: [whs31 : 2026-10-19]
  // callers look the same points up over and over, e.g. the systems of a simulation step: the
  // environment, the rotors and the contacts at the state the step ends at, again at the start of
  // the next one, a vertical ray at every bisection. keyed by the origin of the plane in ecef and
  // the horizontal position, all the latitude and longitude depend on
  using NormalMemo = Memo<5, std::pair<Latitude, Longitude>, 256>;

  [[nodiscard]] NormalMemo& normal_memo()
  {
    thread_local auto const memo = std::make_unique<NormalMemo>();
    return *memo;
  }

  void require_finite(GeodeticPosition const& position, std::string_view const what)
  {
    W_HARD_CHECK(  // NOLINT(*-avoid-do-while)
      position.is_finite(),
      InvalidArgumentError,
      "{} must be finite, got {}",
      what,
      position
    );
  }
}  // namespace

std::string GeodeticPosition::to_string() const
{
  return fmt::
    format("{}, {}, {} m MSL", this->latitude, this->longitude, this->altitude_msl.as_m());
}

std::ostream& operator<<(std::ostream& os, GeodeticPosition const& position)
{
  return os << position.to_string();
}

LocalTangentPlane::LocalTangentPlane()
  : LocalTangentPlane(GeodeticPosition())
{}

LocalTangentPlane::LocalTangentPlane(GeodeticPosition const& origin)
  : m_origin(origin)
{
  require_finite(origin, "tangent plane origin");
  // the macro negates the whole conjunction, which clang-tidy would distribute
  // NOLINTNEXTLINE(*-avoid-do-while,readability-simplify-boolean-expr)
  W_HARD_CHECK(
    origin.latitude > Latitude::south_pole() and origin.latitude < Latitude::north_pole(),
    InvalidArgumentError,
    "tangent plane origin must lie strictly between the poles, got {}",
    origin.latitude
  );
  this->m_origin_ecef = ecef_of(origin);
  this->m_axes = ned_axes(origin.latitude, origin.longitude);
}

GeodeticPosition LocalTangentPlane::lla_from_ned(Vector3 const& position_ned) const
{
  W_HARD_CHECK(  // NOLINT(*-avoid-do-while)
    position_ned.is_finite(),
    InvalidArgumentError,
    "tangent plane position must be finite, got {} m",
    position_ned
  );
  // note: [whs31 : 2026-10-09]
  // the vertical lines of the plane keep their latitude and longitude, so only the horizontal
  // position goes onto it
  auto const key = NormalMemo::Key {
    bits_of(this->m_origin_ecef.x),
    bits_of(this->m_origin_ecef.y),
    bits_of(this->m_origin_ecef.z),
    bits_of(position_ned.x()),
    bits_of(position_ned.y()),
  };
  auto& memo = normal_memo();
  auto const* const kept = memo.find(key);
  auto const horizontal = Ecef {.x = position_ned.x(), .y = position_ned.y(), .z = 0.0};
  auto const [latitude, longitude] = kept != nullptr
                                     ? *kept
                                     : memo.store(
                                         key,
                                         normal_through(
                                           this->m_origin_ecef + from_ned(this->m_axes, horizontal)
                                         )
                                       );
  return {
    .latitude = latitude,
    .longitude = longitude,
    .altitude_msl = this->altitude_msl(position_ned),
  };
}

Vector3 LocalTangentPlane::ned_from_lla(GeodeticPosition const& position) const
{
  require_finite(position, "geodetic position");
  W_HARD_CHECK(  // NOLINT(*-avoid-do-while)
    position.latitude.is_valid(),
    InvalidArgumentError,
    "latitude must be in [-90, 90] deg, got {}",
    position.latitude
  );
  // the normal through the position, followed from the origin's altitude to the plane
  auto const surface = ecef_of({
    .latitude = position.latitude,
    .longitude = position.longitude,
    .altitude_msl = this->m_origin.altitude_msl,
  });
  auto const up = ned_axes(position.latitude, position.longitude).down * -1.0;
  auto const origin_up = this->m_axes.down * -1.0;
  auto const alignment = dot(up, origin_up);
  W_HARD_CHECK(  // NOLINT(*-avoid-do-while)
    alignment > 0.0,
    NumericError,
    "{} {} is too far from the tangent plane origin {} {} to map onto the plane",
    position.latitude,
    position.longitude,
    this->m_origin.latitude,
    this->m_origin.longitude
  );
  auto const rise = -dot(surface - this->m_origin_ecef, origin_up) / alignment;
  auto const horizontal = to_ned(this->m_axes, surface + up * rise - this->m_origin_ecef);
  return {
    static_cast<Real>(horizontal.x),
    static_cast<Real>(horizontal.y),
    (this->m_origin.altitude_msl - position.altitude_msl).as_m(),
  };
}

W_COMMON_NAMESPACE_END
