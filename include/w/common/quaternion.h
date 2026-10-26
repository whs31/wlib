#pragma once

#include <concepts>
#include <cstddef>
#include <array>
#include <bit>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <numbers>
#include <limits>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/detail/format.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/error.h>
#include <w/common/euler_angles.h>
#include <w/common/clip_space.h>
#include <w/common/math.h>
#include <w/common/matrix3x3.h>
#include <w/common/matrix4x4.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Hamilton quaternion of floating-point values, scalar first, used for rotations and
 * attitudes.
 * @details The default value is the identity rotation.
 *
 * @par Conventions
 * - **Hamilton product**: `i² = j² = k² = ijk = −1`, so `i * j == k`. The product `a * b` is the
 *   rotation `b` followed by the rotation `a`.
 * - **Active rotation**: rotate() turns the vector, not the frame:
 *   `q.rotate(v) == q ⊗ (0, v) ⊗ q*`. Rotations are right-handed about their axis.
 * - **Body-to-reference attitude**: an attitude rotates vectors from the frame it describes into
 *   the reference frame: `v_ned = attitude_body_to_ned.rotate(v_body)` and
 *   `v_body = attitude_body_to_ned.inverse_rotate(v_ned)`. Compositions chain the frame names:
 *   `attitude_a_to_c = attitude_b_to_c * attitude_a_to_b`.
 * - to_rotation_matrix() is the same rotation as a matrix: `m * v == q.rotate(v)`.
 * - `q` and `−q` are the same rotation; no sign of `w` is enforced.
 * - **Euler angles** name their order: from_yaw_pitch_roll() is the aerospace ZYX sequence,
 *   from_euler() takes any EulerOrder, e.g. the editor's EulerOrder::YXZ.
 *
 * Only unit quaternions are rotations. Integrating derivative() drifts off the unit sphere;
 * renormalize after each step with normalized().
 *
 * @par Layout
 * The storage is exactly four contiguous @p T values in `w, x, y, z` order, with no padding. The
 * class is standard layout and trivially copyable, so it can be reflected by offset or bit-cast to
 * `std::array<T, 4>`. The
 * layout is part of the API and is checked by `static_assert`s in this header. Jolt, HLSL and glm
 * store `x, y, z, w`; from_xyzw() and to_xyzw() convert at those edges, member by member.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto attitude_body_to_ned = Quaternion::from_yaw_pitch_roll(EulerAngles::from_degrees(0, 5, 90));
 * auto const nose_ned = attitude_body_to_ned.rotate(Vector3::unit_x());  // about east
 *
 * // one explicit Euler step of the attitude kinematics
 * auto const omega_body = Vector3(0.0, 0.0, 0.1);
 * attitude_body_to_ned += 0.01 * attitude_body_to_ned.derivative(omega_body);
 * attitude_body_to_ned = attitude_body_to_ned.normalized();
 * @endcode
 *
 * @ingroup core_math
 * @see Matrix3x3
 * @see EulerAngles
 * @see Transform
 */
template <std::floating_point T>
class BasicQuaternion
{
 public:
  /**
   * @brief Creates the identity rotation.
   */
  constexpr BasicQuaternion() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicQuaternion(
    BasicQuaternion<U> const& other
  ) noexcept
    : BasicQuaternion(
        static_cast<T>(other.w()),
        static_cast<T>(other.x()),
        static_cast<T>(other.y()),
        static_cast<T>(other.z())
      )
  {}

  constexpr BasicQuaternion(T const w, T const x, T const y, T const z) noexcept
    : m_w(w)
    , m_x(x)
    , m_y(y)
    , m_z(z)
  {}

  /**
   * @brief Creates the quaternion with the scalar part @p w and the vector part @p vector.
   */
  constexpr BasicQuaternion(T const w, BasicVector3<T> const vector) noexcept
    : m_w(w)
    , m_x(vector.x())
    , m_y(vector.y())
    , m_z(vector.z())
  {}

  [[nodiscard]] constexpr T w() const noexcept { return this->m_w; }

  [[nodiscard]] constexpr T x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr T y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr T z() const noexcept { return this->m_z; }

  /**
   * @brief Returns the vector part `(x, y, z)`.
   */
  [[nodiscard]] constexpr BasicVector3<T> vector() const noexcept
  {
    return {this->m_x, this->m_y, this->m_z};
  }

  [[nodiscard]] constexpr BasicQuaternion<T> conjugate() const noexcept
  {
    return {this->m_w, -this->m_x, -this->m_y, -this->m_z};
  }

  [[nodiscard]] constexpr T dot(BasicQuaternion<T> const& other) const noexcept
  {
    return this->m_w * other.m_w + this->m_x * other.m_x + this->m_y * other.m_y
         + this->m_z * other.m_z;
  }

  [[nodiscard]] constexpr T length_squared() const noexcept { return this->dot(*this); }

  [[nodiscard]] T length() const noexcept { return std::sqrt(this->length_squared()); }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_w) and common::is_finite(this->m_x)
       and common::is_finite(this->m_y) and common::is_finite(this->m_z);
  }

  /**
   * @brief Returns the unit quaternion of the same rotation, or `std::nullopt` when the length
   * is zero or not finite.
   * @see normalized
   */
  [[nodiscard]] std::optional<BasicQuaternion<T>> try_normalized() const noexcept
  {
    auto const len = this->length();
    if(not positive(len))
      return std::nullopt;
    return BasicQuaternion<T>(this->m_w / len, this->m_x / len, this->m_y / len, this->m_z / len);
  }

  /**
   * @brief Returns the unit quaternion of the same rotation.
   * @throws NumericError if the length is zero or not finite.
   * @see try_normalized
   */
  [[nodiscard]] BasicQuaternion<T> normalized() const;

  /**
   * @brief Returns the multiplicative inverse `q* / |q|²`; for a unit quaternion, the conjugate.
   */
  [[nodiscard]] constexpr BasicQuaternion<T> inverse() const noexcept
  {
    auto result = this->conjugate();
    result /= this->length_squared();
    return result;
  }

  /**
   * @brief Returns the rotation angle of this unit quaternion, in `[0°, 180°]`.
   */
  [[nodiscard]] BasicAngle<T> angle() const noexcept
  {
    return BasicAngle<T>::from_radians(2 * std::atan2(this->vector().length(), abs(this->m_w)));
  }

  /**
   * @brief Returns the angle of the smallest rotation that turns this unit quaternion into
   * @p other, in `[0°, 180°]`: the angle of `other ⊗ this*`.
   * @details Computed from the chord between the quaternions rather than an arc cosine, so it
   * stays accurate for nearby rotations.
   */
  [[nodiscard]] BasicAngle<T> angle_to(BasicQuaternion<T> const& other) const noexcept
  {
    auto const target = this->dot(other) < 0 ? -other : other;
    auto const chord = BasicQuaternion<T>(
      this->m_w - target.m_w,
      this->m_x - target.m_x,
      this->m_y - target.m_y,
      this->m_z - target.m_z
    );
    auto const sum = BasicQuaternion<T>(
      this->m_w + target.m_w,
      this->m_x + target.m_x,
      this->m_y + target.m_y,
      this->m_z + target.m_z
    );
    return BasicAngle<T>::from_radians(4 * std::atan2(chord.length(), sum.length()));
  }

  /**
   * @brief Normalized linear interpolation toward @p other along the shorter arc.
   * @details Cheaper than slerp() and exact at `t = 0` and `t = 1`, but its angular speed is not
   * constant. @p t is not clamped.
   * @pre Both quaternions are unit quaternions.
   * @see slerp
   */
  [[nodiscard]] BasicQuaternion<T> nlerp(BasicQuaternion<T> const& other, T t) const noexcept;

  /**
   * @brief Spherical linear interpolation toward @p other along the shorter arc, at constant
   * angular speed.
   * @details Falls back to nlerp() when the rotations are within a milliradian, where both agree
   * and the spherical weights lose precision. @p t is not clamped.
   * @pre Both quaternions are unit quaternions.
   * @see nlerp
   */
  [[nodiscard]] BasicQuaternion<T> slerp(BasicQuaternion<T> const& other, T t) const noexcept;

  /**
   * @brief Rotates @p v by this unit quaternion: `q ⊗ (0, v) ⊗ q*`.
   * @see inverse_rotate
   */
  [[nodiscard]] constexpr BasicVector3<T> rotate(BasicVector3<T> const v) const noexcept
  {
    auto const u = this->vector();
    auto const t = 2.0 * u.cross(v);
    return v + this->m_w * t + u.cross(t);
  }

  /**
   * @brief Rotates @p v by the inverse of this unit quaternion: `q* ⊗ (0, v) ⊗ q`.
   * @see rotate
   */
  [[nodiscard]] constexpr BasicVector3<T> inverse_rotate(BasicVector3<T> const v) const noexcept
  {
    return this->conjugate().rotate(v);
  }

  /**
   * @brief Returns the rotation matrix `m` of this unit quaternion: `m * v == rotate(v)`.
   * @details The inverse rotation is the transpose of the matrix.
   */
  [[nodiscard]] constexpr BasicMatrix3x3<T> to_rotation_matrix() const noexcept
  {
    auto const w = this->m_w;
    auto const x = this->m_x;
    auto const y = this->m_y;
    auto const z = this->m_z;
    return BasicMatrix3x3<T>::from_rows(
      {1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y)},
      {2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x)},
      {2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)}
    );
  }

  /**
   * @brief Returns the rotation matrix of this unit quaternion as an affine 4×4 transform without
   * translation.
   */
  [[nodiscard]] constexpr BasicMatrix4x4<T> to_matrix4() const noexcept
  {
    return BasicMatrix4x4<T>::from_affine(this->to_rotation_matrix());
  }

  /**
   * @brief Returns the angles about the x, y and z axes, in that order, that from_euler() turns
   * back into this unit quaternion.
   * @details The middle rotation of @p Order is in `[-90°, 90°]`, the other two in
   * `[-180°, 180°]`. Where the middle one is ±90°, only the sum or difference of the other two is
   * defined; there the last rotation of @p Order is zero and the first carries the whole turn.
   * @see from_euler
   */
  template <EulerOrder Order>
  [[nodiscard]] std::array<BasicAngle<T>, 3> to_euler() const noexcept;

  /**
   * @brief Returns the aerospace ZYX Euler angles of this unit quaternion.
   * @details Same as `to_euler<EulerOrder::ZYX>()`: roll and yaw are in `[-180°, 180°]`, pitch in
   * `[-90°, 90°]`. At ±90° pitch only the sum or difference of roll and yaw is defined; there the
   * roll is zero and the yaw carries the whole rotation about the vertical.
   * @see from_yaw_pitch_roll
   */
  [[nodiscard]] BasicEulerAngles<T> to_yaw_pitch_roll() const noexcept
  {
    auto const [roll, pitch, yaw] = this->to_euler<EulerOrder::ZYX>();
    return {roll, pitch, yaw};
  }

  /**
   * @brief Returns the aerospace ZYX Euler angles of this unit quaternion.
   * @deprecated The name does not say which order; use to_yaw_pitch_roll(), or to_euler() for
   * another order.
   */
  [[deprecated("use to_yaw_pitch_roll(), or to_euler<Order>() for another order")]]
  [[nodiscard]] BasicEulerAngles<T>
    to_euler_angles() const noexcept
  {
    return this->to_yaw_pitch_roll();
  }

  /**
   * @brief Returns the components in `x, y, z, w` order, as Jolt, HLSL and glm store them.
   * @see from_xyzw
   */
  [[nodiscard]] constexpr std::array<T, 4> to_xyzw() const noexcept
  {
    return {this->m_x, this->m_y, this->m_z, this->m_w};
  }

  /**
   * @brief Returns the time derivative `q̇ = ½ q ⊗ (0, ω)` of an attitude turning at the angular
   * velocity @p omega_body.
   * @details @p omega_body is the angular velocity of the body relative to the reference frame,
   * in body axes, in radians per second. The result is per second.
   */
  [[nodiscard]] constexpr BasicQuaternion<T> derivative(BasicVector3<T> omega_body) const noexcept;

  /**
   * @brief Returns the components as `(w, x, y, z)`.
   */
  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  [[nodiscard]] static constexpr BasicQuaternion<T> identity() noexcept { return {}; }

  [[nodiscard]] static constexpr BasicQuaternion<T> from_wxyz(
    T const w,
    T const x,
    T const y,
    T const z
  ) noexcept
  {
    return {w, x, y, z};
  }

  /**
   * @brief Returns the quaternion of the components in `x, y, z, w` order, as Jolt, HLSL and glm
   * store them.
   * @see to_xyzw
   */
  [[nodiscard]] static constexpr BasicQuaternion<T> from_xyzw(
    T const x,
    T const y,
    T const z,
    T const w
  ) noexcept
  {
    return {w, x, y, z};
  }

  /**
   * @brief Returns the right-handed rotation by @p angle about @p axis.
   * @details @p axis need not be a unit vector.
   * @throws NumericError if @p axis is zero or not finite.
   */
  [[nodiscard]] static BasicQuaternion<T> from_axis_angle(
    BasicVector3<T> const axis,
    BasicAngle<T> const angle
  )
  {
    auto const unit = axis.try_normalized();
    if(not unit.has_value())
      throw NumericError("cannot rotate about the axis {}", axis);

    auto const half = angle / 2;
    return {half.cos(), *unit * half.sin()};
  }

  /**
   * @brief Returns the rotation of the angles @p about_x, @p about_y and @p about_z about the
   * coordinate axes, composed in @p Order.
   * @details `from_euler<EulerOrder::YXZ>(pitch, yaw, roll)` is `q_y(yaw) ⊗ q_x(pitch) ⊗
   * q_z(roll)`. The angles are passed by axis, not in the order they are applied, so changing the
   * order never reshuffles the arguments.
   * @see to_euler
   * @see from_yaw_pitch_roll
   */
  template <EulerOrder Order>
  [[nodiscard]] static BasicQuaternion<T> from_euler(
    BasicAngle<T> about_x,
    BasicAngle<T> about_y,
    BasicAngle<T> about_z
  ) noexcept;

  /**
   * @brief Returns the attitude of the aerospace ZYX Euler angles.
   * @details `q = q_z(yaw) ⊗ q_y(pitch) ⊗ q_x(roll)`: yaw about the reference z axis, then pitch
   * about the new y axis, then roll about the new x axis. With NED reference axes and FRD body
   * axes, positive yaw turns the nose right, positive pitch raises it, and positive roll lowers
   * the right side. Same as `from_euler<EulerOrder::ZYX>(roll, pitch, yaw)`.
   * @see to_yaw_pitch_roll
   */
  [[nodiscard]] static BasicQuaternion<T> from_yaw_pitch_roll(
    BasicEulerAngles<T> const& angles
  ) noexcept
  {
    return from_euler<EulerOrder::ZYX>(angles.roll(), angles.pitch(), angles.yaw());
  }

  /**
   * @brief Returns the attitude of the aerospace ZYX Euler angles.
   * @deprecated The name does not say which order; use from_yaw_pitch_roll(), or from_euler() for
   * another order.
   */
  [[deprecated("use from_yaw_pitch_roll(), or from_euler<Order>() for another order")]]
  [[nodiscard]] static BasicQuaternion<T>
    from_euler_angles(BasicEulerAngles<T> const& angles) noexcept
  {
    return from_yaw_pitch_roll(angles);
  }

  /**
   * @brief Returns the unit quaternion of the rotation matrix @p matrix.
   * @details Uses Shepperd's method, which stays accurate for every rotation. The result is
   * normalized, so a matrix that is orthonormal up to rounding gives a unit quaternion.
   * @pre @p matrix is a rotation matrix: orthonormal with determinant `+1`.
   * @see to_rotation_matrix
   */
  [[nodiscard]] static BasicQuaternion<T> from_rotation_matrix(
    BasicMatrix3x3<T> const& matrix
  ) noexcept;

  /**
   * @brief Returns the shortest rotation that turns the direction @p from into the direction
   * @p to.
   * @details Neither needs to be a unit vector. Opposite directions have no unique shortest
   * rotation; the result is then the half turn about an axis perpendicular to @p from.
   * @throws NumericError if either vector is zero or not finite.
   */
  [[nodiscard]] static BasicQuaternion<T> from_to(BasicVector3<T> from, BasicVector3<T> to);

  /**
   * @brief Returns the orientation whose forward axis points along @p forward and whose up axis
   * leans toward @p up: the camera-to-world rotation of look_at().
   * @details The forward axis is −z for Handedness::Right and +z for Handedness::Left; y is up and
   * x completes the basis, so the result is the inverse of the rotation in `look_at(eye, eye +
   * forward, up, handedness)`. Where @p up is parallel to @p forward, the roll is arbitrary and
   * the result is from_to() the forward axis.
   * @throws NumericError if @p forward is zero or not finite.
   */
  [[nodiscard]] static BasicQuaternion<T> look_rotation(
    BasicVector3<T> const forward,
    BasicVector3<T> const up,
    Handedness const handedness
  )
  {
    auto const forward_axis = handedness == Handedness::Right ? -BasicVector3<T>::unit_z()
                                                              : BasicVector3<T>::unit_z();
    auto const direction = forward.try_normalized();
    if(not direction.has_value())
      throw NumericError("cannot look along {}", forward);

    auto const z = handedness == Handedness::Right ? -*direction : *direction;
    auto const x = up.cross(z).try_normalized();
    if(not x.has_value())
      return from_to(forward_axis, *direction);
    return from_rotation_matrix(BasicMatrix3x3<T>::from_columns(*x, z.cross(*x), z));
  }

  [[nodiscard]] constexpr bool operator==(BasicQuaternion<T> const&) const noexcept = default;

  [[nodiscard]] constexpr BasicQuaternion<T> operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr BasicQuaternion<T> operator-() const noexcept
  {
    return {-this->m_w, -this->m_x, -this->m_y, -this->m_z};
  }

  constexpr BasicQuaternion<T>& operator+=(BasicQuaternion<T> const& rhs) noexcept
  {
    this->m_w += rhs.m_w;
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    this->m_z += rhs.m_z;
    return *this;
  }

  constexpr BasicQuaternion<T>& operator-=(BasicQuaternion<T> const& rhs) noexcept
  {
    this->m_w -= rhs.m_w;
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    this->m_z -= rhs.m_z;
    return *this;
  }

  constexpr BasicQuaternion<T>& operator*=(T const rhs) noexcept
  {
    this->m_w *= rhs;
    this->m_x *= rhs;
    this->m_y *= rhs;
    this->m_z *= rhs;
    return *this;
  }

  constexpr BasicQuaternion<T>& operator/=(T const rhs) noexcept
  {
    this->m_w /= rhs;
    this->m_x /= rhs;
    this->m_y /= rhs;
    this->m_z /= rhs;
    return *this;
  }

  [[nodiscard]] friend constexpr BasicQuaternion<T> operator+(
    BasicQuaternion<T> lhs,
    BasicQuaternion<T> const& rhs
  ) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicQuaternion<T> operator-(
    BasicQuaternion<T> lhs,
    BasicQuaternion<T> const& rhs
  ) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicQuaternion<T> operator*(
    BasicQuaternion<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs *= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicQuaternion<T> operator*(
    T const lhs,
    BasicQuaternion<T> rhs
  ) noexcept
  {
    rhs *= lhs;
    return rhs;
  }

  [[nodiscard]] friend constexpr BasicQuaternion<T> operator/(
    BasicQuaternion<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs /= rhs;
    return lhs;
  }

  /**
   * @brief Returns the Hamilton product `a ⊗ b`: the rotation `b` followed by `a`.
   * @relates Quaternion
   */
  [[nodiscard]] friend constexpr BasicQuaternion<T> operator*(
    BasicQuaternion<T> const& a,
    BasicQuaternion<T> const& b
  ) noexcept
  {
    return {
      a.w() * b.w() - a.x() * b.x() - a.y() * b.y() - a.z() * b.z(),
      a.w() * b.x() + a.x() * b.w() + a.y() * b.z() - a.z() * b.y(),
      a.w() * b.y() - a.x() * b.z() + a.y() * b.w() + a.z() * b.x(),
      a.w() * b.z() + a.x() * b.y() - a.y() * b.x() + a.z() * b.w()
    };
  }

 private:
  T m_w = 1.0;
  T m_x = 0.0;
  T m_y = 0.0;
  T m_z = 0.0;
};

/**
 * @brief Quaternion in single precision, the default.
 */
using Quaternion = BasicQuaternion<float>;

template <std::floating_point T>
constexpr BasicQuaternion<T> BasicQuaternion<T>::derivative(
  BasicVector3<T> const omega_body
) const noexcept
{
  return 0.5 * (*this * BasicQuaternion<T>(0.0, omega_body));
}

template <std::floating_point T>
inline BasicQuaternion<T> BasicQuaternion<T>::nlerp(
  BasicQuaternion<T> const& other,
  T const t
) const noexcept
{
  auto const target = this->dot(other) < 0 ? -other : other;
  auto const blended = *this * (1 - t) + target * t;
  return blended / blended.length();
}

template <std::floating_point T>
inline BasicQuaternion<T> BasicQuaternion<T>::slerp(
  BasicQuaternion<T> const& other,
  T const t
) const noexcept
{
  auto const target = this->dot(other) < 0 ? -other : other;
  // half of angle_to(): the arc between the quaternions on the unit sphere
  auto const arc = this->angle_to(target).radians() / 2;
  if(arc < T {1e-3})
    return this->nlerp(target, t);
  auto const sin_arc = std::sin(arc);
  return (*this * std::sin((1 - t) * arc) + target * std::sin(t * arc)) / sin_arc;
}

template <std::floating_point T>
inline BasicQuaternion<T> BasicQuaternion<T>::from_rotation_matrix(
  BasicMatrix3x3<T> const& matrix
) noexcept
{
  auto const [m00, m01, m02, m10, m11, m12, m20, m21, m22] = std::bit_cast<std::array<T, 9>>(
    matrix
  );

  // Shepperd's method: derive the largest of |w|, |x|, |y|, |z| from the diagonal, so the
  // divisor of the other three components is never small
  auto const trace = m00 + m11 + m22;
  auto result = BasicQuaternion<T>();
  if(trace > 0) {
    auto const s = 2 * std::sqrt(1 + trace);
    result = {s / 4, (m21 - m12) / s, (m02 - m20) / s, (m10 - m01) / s};
  }
  else if(m00 > m11 and m00 > m22) {
    auto const s = 2 * std::sqrt(1 + m00 - m11 - m22);
    result = {(m21 - m12) / s, s / 4, (m01 + m10) / s, (m02 + m20) / s};
  }
  else if(m11 > m22) {
    auto const s = 2 * std::sqrt(1 + m11 - m00 - m22);
    result = {(m02 - m20) / s, (m01 + m10) / s, s / 4, (m12 + m21) / s};
  }
  else {
    auto const s = 2 * std::sqrt(1 + m22 - m00 - m11);
    result = {(m10 - m01) / s, (m02 + m20) / s, (m12 + m21) / s, s / 4};
  }
  return result / result.length();
}

template <std::floating_point T>
inline BasicQuaternion<T> BasicQuaternion<T>::from_to(
  BasicVector3<T> const from,
  BasicVector3<T> const to
)
{
  auto const u = from.try_normalized();
  auto const v = to.try_normalized();
  if(not u.has_value() or not v.has_value())
    throw NumericError("cannot rotate {} onto {}", from, to);

  auto const cos_angle = u->dot(*v);
  // note: [whs31 : 2026-10-26]
  // near the half turn the cross product vanishes, so the axis comes from a coordinate axis
  // instead; float resolves the remaining angle to about a milliradian
  if(cos_angle < -1 + T {1e-6}) {
    auto axis = BasicVector3<T>::unit_x().cross(*u);
    if(axis.length_squared() < T {1e-6})
      axis = BasicVector3<T>::unit_y().cross(*u);
    return {0.0, axis.normalized()};
  }
  auto const q = BasicQuaternion<T>(1 + cos_angle, u->cross(*v));
  return q / q.length();
}

/**
 * @brief Returns `a.nlerp(b, t)`.
 * @relates Quaternion
 */
template <std::floating_point T>
[[nodiscard]] inline BasicQuaternion<T> nlerp(
  BasicQuaternion<T> const& a,
  BasicQuaternion<T> const& b,
  std::type_identity_t<T> const t
) noexcept
{
  return a.nlerp(b, t);
}

/**
 * @brief Returns `a.slerp(b, t)`.
 * @relates Quaternion
 */
template <std::floating_point T>
[[nodiscard]] inline BasicQuaternion<T> slerp(
  BasicQuaternion<T> const& a,
  BasicQuaternion<T> const& b,
  std::type_identity_t<T> const t
) noexcept
{
  return a.slerp(b, t);
}

namespace _euler_impl  // NOLINT(*-identifier-naming)
{
  /**
   * @brief The axes of @p order, first to last, and whether they are an even permutation of
   * `x, y, z`, which flips the signs of the extraction.
   */
  struct Axes
  {
    std::size_t first = 0;
    std::size_t second = 0;
    std::size_t third = 0;
    int sign = 1;
  };

  [[nodiscard]] constexpr Axes axes(EulerOrder const order) noexcept
  {
    switch(order) {
      case EulerOrder::XYZ: return {.first = 0, .second = 1, .third = 2, .sign = 1};
      case EulerOrder::XZY: return {.first = 0, .second = 2, .third = 1, .sign = -1};
      case EulerOrder::YXZ: return {.first = 1, .second = 0, .third = 2, .sign = -1};
      case EulerOrder::YZX: return {.first = 1, .second = 2, .third = 0, .sign = 1};
      case EulerOrder::ZXY: return {.first = 2, .second = 0, .third = 1, .sign = 1};
      case EulerOrder::ZYX: return {.first = 2, .second = 1, .third = 0, .sign = -1};
    }
    return {};
  }

  /**
   * @brief Below this cosine of the middle angle, rounding noise of the quaternion outweighs the
   * first and last terms, which scale with it: the square root of the epsilon of @p T.
   */
  template <std::floating_point T>
  [[nodiscard]] inline T singular_cos() noexcept
  {
    return std::sqrt(std::numeric_limits<T>::epsilon());
  }
}  // namespace _euler_impl

template <std::floating_point T>
template <EulerOrder Order>
BasicQuaternion<T> BasicQuaternion<T>::from_euler(
  BasicAngle<T> const about_x,
  BasicAngle<T> const about_y,
  BasicAngle<T> const about_z
) noexcept
{
  auto constexpr axes = _euler_impl::axes(Order);
  auto const angles = std::array {about_x, about_y, about_z};
  auto const about = [&angles](std::size_t const axis) {
    auto const half = angles.at(axis) / 2;
    auto vector = std::array<T, 3> {};
    vector.at(axis) = half.sin();
    return BasicQuaternion<T>(half.cos(), std::bit_cast<BasicVector3<T>>(vector));
  };
  return about(axes.first) * about(axes.second) * about(axes.third);
}

template <std::floating_point T>
template <EulerOrder Order>
std::array<BasicAngle<T>, 3> BasicQuaternion<T>::to_euler() const noexcept
{
  auto constexpr axes = _euler_impl::axes(Order);
  auto const [i, j, k, sign] = axes;
  auto const elements = std::bit_cast<std::array<T, 9>>(this->to_rotation_matrix());
  auto const m = [&elements](std::size_t const row, std::size_t const column) {
    return elements.at(3 * row + column);
  };

  // R = R_i(a) R_j(b) R_k(c): R[i][k] = ±sin b, R[i][i] and R[i][j] carry c scaled by cos b,
  // R[k][k] and R[j][k] carry a scaled by cos b
  auto const cos_middle = std::hypot(m(i, i), m(i, j));
  auto const middle = std::atan2(sign * m(i, k), cos_middle);
  auto first = T {0};
  auto last = T {0};
  if(cos_middle < _euler_impl::singular_cos<T>())
    first = std::atan2(sign * m(k, j), m(j, j));
  else {
    first = std::atan2(-sign * m(j, k), m(k, k));
    last = std::atan2(-sign * m(i, j), m(i, i));
  }

  auto result = std::array<BasicAngle<T>, 3> {};
  result.at(i) = BasicAngle<T>::from_radians(first);
  result.at(j) = BasicAngle<T>::from_radians(middle);
  result.at(k) = BasicAngle<T>::from_radians(last);
  return result;
}

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicQuaternion<T> const& quaternion)
{
  return os << quaternion.to_string();
}

static_assert(std::is_standard_layout_v<Quaternion>);
static_assert(std::is_trivially_copyable_v<Quaternion>);
static_assert(sizeof(Quaternion) == 4 * sizeof(float));
static_assert(sizeof(BasicQuaternion<double>) == 4 * sizeof(double));
static_assert(
  std::bit_cast<std::array<float, 4>>(Quaternion(1.0, 2.0, 3.0, 4.0))
    == std::array<float, 4> {1, 2, 3, 4},
  "Quaternion must store w, x, y, z contiguously"
);

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicQuaternion<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicQuaternion<T> const& q) const noexcept
  {
    auto hash = std::hash<T> {}(q.w());
    for(auto const value : {q.x(), q.y(), q.z()})
      hash ^= std::hash<T> {}(value) + 0x9e3779b97f4a7c15ull + (hash << 6U) + (hash >> 2U);
    return hash;
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(w, x, y, z)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicQuaternion<T>> : fmt::formatter<T>
{
  format_context::iterator format(w::common::BasicQuaternion<T> const& q, format_context& ctx) const
  {
    return w::common::_format_impl::
      format_components<4, T>(*this, {q.w(), q.x(), q.y(), q.z()}, ctx);
  }
};
#endif

W_COMMON_NAMESPACE_BEGIN

// note: [whs31 : 2026-10-26]
// defined after the formatter, which the error message needs
template <std::floating_point T>
inline BasicQuaternion<T> BasicQuaternion<T>::normalized() const
{
  if(auto const result = this->try_normalized())
    return *result;
  throw NumericError("cannot normalize the quaternion {}", *this);
}

// note: [whs31 : 2026-10-26]
// euler_angles.h only declares the quaternion, so the conversions are defined here
template <std::floating_point T>
inline BasicEulerAngles<T> BasicEulerAngles<T>::from_quaternion(
  BasicQuaternion<T> const& attitude
) noexcept
{
  return attitude.to_yaw_pitch_roll();
}

template <std::floating_point T>
inline BasicQuaternion<T> BasicEulerAngles<T>::to_quaternion() const noexcept
{
  return BasicQuaternion<T>::from_yaw_pitch_roll(*this);
}

W_COMMON_NAMESPACE_END
