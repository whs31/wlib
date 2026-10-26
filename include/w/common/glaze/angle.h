#pragma once

#include <concepts>
#include <optional>
#include <string>
#include <variant>

#include <w/common/angle.h>
#include <w/common/glaze/detail.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  template <std::floating_point T>
  struct AngleFields
  {
    std::optional<T> angle;
    std::optional<std::string> units;
    std::optional<T> degrees;
    std::optional<T> radians;
  };

  template <std::floating_point T>
  inline auto constexpr angle_reading = QuantityReading<BasicAngle<T>, AngleFields<T>, 2, 2> {
    .fields =
      {
        field_unit(&AngleFields<T>::degrees, &BasicAngle<T>::from_degrees),
        field_unit(&AngleFields<T>::radians, &BasicAngle<T>::from_radians),
      },
    .literals =
      {
        literal_unit("deg", &BasicAngle<T>::from_degrees),
        literal_unit("rad", &BasicAngle<T>::from_radians),
      },
    .unknown_suffix = "an angle literal needs one of the unit suffixes deg, rad",
  };

  template <std::floating_point T>
  void read_angle(BasicAngle<T>& angle, AngleFields<T> const& fields, glz::context& ctx) noexcept
  {
    if(not fields.angle.has_value()) {
      if(fields.units.has_value())
        reject(ctx, "angle units without an angle");
      else
        read_fields(angle, ctx, fields, angle_reading<T>.fields);
      return;
    }
    if(fields.degrees.has_value() or fields.radians.has_value())
      reject(ctx, "an angle must hold its magnitude in exactly one unit");
    else if(fields.units == "degrees")
      angle = BasicAngle<T>::from_degrees(*fields.angle);
    else if(fields.units == "radians")
      angle = BasicAngle<T>::from_radians(*fields.angle);
    else
      reject(ctx, "invalid angle units: expected either degrees or radians");
  }

  template <std::floating_point T>
  void read_angle(
    BasicAngle<T>& angle,
    QuantityInput<AngleFields<T>> const& input,
    glz::context& ctx
  ) noexcept
  {
    if(auto const* const literal = std::get_if<std::string>(&input.value()))
      read_literal(
        angle,
        ctx,
        *literal,
        angle_reading<T>.literals,
        angle_reading<T>.unknown_suffix
      );
    else if(auto const* const fields = std::get_if<AngleFields<T>>(&input.value()))
      read_angle(angle, *fields, ctx);
  }
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Glaze metadata of Angle: `{"angle": degrees, "units": "degrees"}`.
 * @details Reading also accepts `"units": "radians"`, and the shorter `{"degrees": magnitude}` or
 * `{"radians": magnitude}`. A text format also reads a literal string with the suffix of a C++
 * literal, `"30deg"` or `"0.5rad"`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicAngle<T>>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::BasicAngle<T>& angle,
      w::common::_glaze_impl::QuantityInput<w::common::_glaze_impl::AngleFields<T>> const& input,
      glz::context& ctx
    ) { w::common::_glaze_impl::read_angle(angle, input, ctx); },
    [](w::common::BasicAngle<T> const& angle) {
      return w::common::_glaze_impl::AngleFields<T> {
        .angle = angle.degrees(),
        .units = "degrees",
      };
    }>;
};
