#pragma once

#include <concepts>
#include <array>
#include <bit>
#include <compare>
#include <functional>
#include <iosfwd>
#include <ostream>
#include <string>
#include <type_traits>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <w/common/stdint.h>
#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>

W_COMMON_NAMESPACE_BEGIN

template <std::floating_point T>
class BasicQuaternion;

/**
 * @brief Order in which Quaternion::from_euler() composes its three rotations about the
 * coordinate axes.
 * @details `EulerOrder::ABC` is the rotation `R_A(a) * R_B(b) * R_C(c)` with column vectors: about
 * the fixed axes, C turns first, then B, then A; about the moving axes, A turns first, then the
 * new B, then the newer C. The names match glm's `eulerAngleABC`.
 *
 * - **ZYX** is the aerospace yaw, pitch, roll sequence of EulerAngles and helisim:
 *   Quaternion::from_yaw_pitch_roll().
 * - **YXZ** is the y-up editor sequence: yaw about y, then pitch about x, then roll about z, as in
 *   glm's `yawPitchRoll` and Godot.
 *
 * Only the six Tait–Bryan orders, each axis once, exist.
 * @ingroup core_math
 * @see Quaternion::from_euler
 * @see Quaternion::to_euler
 */
enum class EulerOrder : u8
{
  XYZ,
  XZY,
  YXZ,
  YZX,
  ZXY,
  ZYX,
};

/**
 * @brief Roll, pitch, and yaw rotation angles of the aerospace ZYX sequence.
 * @details Values are stored as three Angle objects. The `*Radians` and
 * `*Degrees` properties are provided for convenient QML use. The angles always mean
 * EulerOrder::ZYX: yaw about z, then pitch about the new y, then roll about the new x. Other
 * orders, e.g. an editor's y-up display angles, go through Quaternion::from_euler() and
 * Quaternion::to_euler() instead.
 *
 * @par Layout
 * The storage is exactly three contiguous Angle values in `roll, pitch, yaw` order, each a single
 * @p T in radians, with no padding. The class is standard layout and trivially copyable, so it
 * can be reflected by offset or bit-cast to `std::array<T, 3>` of radians. The layout is part of
 * the API and is checked by `static_assert`s in this header.
 *
 * @ingroup core_measurements
 * @see Angle
 * @see Quaternion
 */
template <std::floating_point T>
class BasicEulerAngles
{
 public:
  constexpr BasicEulerAngles() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicEulerAngles(
    BasicEulerAngles<U> const& other
  ) noexcept
    : BasicEulerAngles(
        BasicAngle<T>(other.roll()),
        BasicAngle<T>(other.pitch()),
        BasicAngle<T>(other.yaw())
      )
  {}

  constexpr BasicEulerAngles(
    BasicAngle<T> const roll,
    BasicAngle<T> const pitch,
    BasicAngle<T> const yaw
  ) noexcept
    : m_roll(roll)
    , m_pitch(pitch)
    , m_yaw(yaw)
  {}

  [[nodiscard]] static constexpr BasicEulerAngles<T> from_angles(
    BasicAngle<T> const roll,
    BasicAngle<T> const pitch,
    BasicAngle<T> const yaw
  ) noexcept
  {
    return {roll, pitch, yaw};
  }

  [[nodiscard]] static constexpr BasicEulerAngles<T> from_radians(
    T const roll,
    T const pitch,
    T const yaw
  ) noexcept
  {
    return {
      BasicAngle<T>::from_radians(roll),
      BasicAngle<T>::from_radians(pitch),
      BasicAngle<T>::from_radians(yaw)
    };
  }

  [[nodiscard]] static constexpr BasicEulerAngles<T> from_degrees(
    T const roll,
    T const pitch,
    T const yaw
  ) noexcept
  {
    return {
      BasicAngle<T>::from_degrees(roll),
      BasicAngle<T>::from_degrees(pitch),
      BasicAngle<T>::from_degrees(yaw)
    };
  }

  [[nodiscard]] static constexpr BasicEulerAngles<T> zero() noexcept { return {}; }

  /**
   * @brief Returns the aerospace ZYX Euler angles of the unit quaternion @p attitude.
   * @details Same as Quaternion::to_yaw_pitch_roll(): roll and yaw are in `[-180°, 180°]`, pitch
   * in `[-90°, 90°]`, and the roll is zero at ±90° pitch.
   * @note Include `w/common/quaternion.h` to call it; this header only declares
   * Quaternion.
   * @see to_quaternion
   */
  [[nodiscard]] static BasicEulerAngles<T> from_quaternion(
    BasicQuaternion<T> const& attitude
  ) noexcept;

  [[nodiscard]] constexpr BasicAngle<T> roll() const noexcept { return this->m_roll; }

  [[nodiscard]] constexpr BasicAngle<T> pitch() const noexcept { return this->m_pitch; }

  [[nodiscard]] constexpr BasicAngle<T> yaw() const noexcept { return this->m_yaw; }

  [[nodiscard]] constexpr T roll_radians() const noexcept { return this->m_roll.radians(); }

  [[nodiscard]] constexpr T pitch_radians() const noexcept { return this->m_pitch.radians(); }

  [[nodiscard]] constexpr T yaw_radians() const noexcept { return this->m_yaw.radians(); }

  [[nodiscard]] constexpr T roll_degrees() const noexcept { return this->m_roll.degrees(); }

  [[nodiscard]] constexpr T pitch_degrees() const noexcept { return this->m_pitch.degrees(); }

  [[nodiscard]] constexpr T yaw_degrees() const noexcept { return this->m_yaw.degrees(); }

  [[nodiscard]] bool is_finite() const noexcept
  {
    return this->m_roll.is_finite() and this->m_pitch.is_finite() and this->m_yaw.is_finite();
  }

  /**
   * @brief Returns the attitude of these aerospace ZYX Euler angles.
   * @details Same as Quaternion::from_yaw_pitch_roll():
   * `q = q_z(yaw) ⊗ q_y(pitch) ⊗ q_x(roll)`.
   * @note Include `w/common/quaternion.h` to call it; this header only declares
   * Quaternion.
   * @see from_quaternion
   */
  [[nodiscard]] BasicQuaternion<T> to_quaternion() const noexcept;

  [[nodiscard]] std::string to_string() const
  {
    return fmt::format("roll {}, pitch {}, yaw {}", this->m_roll, this->m_pitch, this->m_yaw);
  }

  [[nodiscard]] constexpr bool operator==(BasicEulerAngles<T> const&) const noexcept = default;

  [[nodiscard]] constexpr std::partial_ordering operator<=>(
    BasicEulerAngles<T> const&
  ) const noexcept = default;

 private:
  BasicAngle<T> m_roll;
  BasicAngle<T> m_pitch;
  BasicAngle<T> m_yaw;
};

/**
 * @brief EulerAngles in single precision, the default.
 */
using EulerAngles = BasicEulerAngles<float>;

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicEulerAngles<T> const& angles)
{
  return os << angles.to_string();
}

static_assert(std::is_standard_layout_v<EulerAngles>);
static_assert(std::is_trivially_copyable_v<EulerAngles>);
static_assert(sizeof(EulerAngles) == 3 * sizeof(float));
static_assert(sizeof(BasicEulerAngles<double>) == 3 * sizeof(double));
static_assert(
  std::bit_cast<std::array<float, 3>>(EulerAngles::from_radians(1.0, 2.0, 3.0))
    == std::array<float, 3> {1, 2, 3},
  "EulerAngles must store roll, pitch, yaw radians contiguously"
);

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicEulerAngles<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicEulerAngles<T> const& angles) const noexcept
  {
    auto hash = std::hash<w::common::BasicAngle<T>> {}(angles.roll());
    hash ^= std::hash<w::common::BasicAngle<T>> {}(angles.pitch()) + 0x9e3779b97f4a7c15ull
          + (hash << 6) + (hash >> 2);  // NOLINT(*-redundant-parentheses)
    hash ^= std::hash<w::common::BasicAngle<T>> {}(angles.yaw()) + 0x9e3779b97f4a7c15ull
          + (hash << 6) + (hash >> 2);  // NOLINT(*-redundant-parentheses)
    return hash;
  }
};

#ifndef __DOXYGEN__
template <std::floating_point T>
struct fmt::formatter<w::common::BasicEulerAngles<T>> : ostream_formatter
{};
#endif
