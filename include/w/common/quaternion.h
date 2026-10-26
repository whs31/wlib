#pragma once

#include <cstddef>
#include <array>
#include <bit>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <numbers>
#include <optional>
#include <ostream>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/angle.h>
#include <w/common/config.h>
#include <w/common/detail/export.h>
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

#ifdef W_COMMON_WITH_QT
#  include <qmetaobject.h>
#  include <qmetatype.h>
#  include <qquaternion.h>
#  include <qstring.h>
#endif

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Hamilton quaternion of Real values, scalar first, used for rotations and attitudes.
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
 * The storage is exactly four contiguous Real values in `w, x, y, z` order, with no padding. The
 * class is standard layout and trivially copyable, so it can be reflected by offset or bit-cast to
 * `std::array<Real, 4>`. `Q_GADGET` adds no data members and does not affect the layout. The
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
 * @note Qt properties, conversions, and metatype registration are available only when wlib is
 * built with `with_qt=True`.
 * @ingroup core_math
 * @see Matrix3x3
 * @see EulerAngles
 * @see Transform
 */
class W_COMMON_API Quaternion
{
#ifdef W_COMMON_WITH_QT
  Q_GADGET

  Q_PROPERTY(double w READ w CONSTANT FINAL)
  Q_PROPERTY(double x READ x CONSTANT FINAL)
  Q_PROPERTY(double y READ y CONSTANT FINAL)
  Q_PROPERTY(double z READ z CONSTANT FINAL)
  Q_PROPERTY(double length READ length CONSTANT FINAL)
  Q_PROPERTY(bool isFinite READ is_finite CONSTANT FINAL)
  Q_PROPERTY(QString string READ to_qstring CONSTANT FINAL)
#endif

 public:
  /**
   * @brief Creates the identity rotation.
   */
  constexpr Quaternion() noexcept = default;

  constexpr Quaternion(Real const w, Real const x, Real const y, Real const z) noexcept
    : m_w(w)
    , m_x(x)
    , m_y(y)
    , m_z(z)
  {}

  /**
   * @brief Creates the quaternion with the scalar part @p w and the vector part @p vector.
   */
  constexpr Quaternion(Real const w, Vector3 const vector) noexcept
    : m_w(w)
    , m_x(vector.x())
    , m_y(vector.y())
    , m_z(vector.z())
  {}

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  explicit Quaternion(QQuaternion const& quaternion) noexcept;
#endif

  [[nodiscard]] constexpr Real w() const noexcept { return this->m_w; }

  [[nodiscard]] constexpr Real x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr Real y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr Real z() const noexcept { return this->m_z; }

  /**
   * @brief Returns the vector part `(x, y, z)`.
   */
  [[nodiscard]] constexpr Vector3 vector() const noexcept
  {
    return {this->m_x, this->m_y, this->m_z};
  }

  [[nodiscard]] constexpr Quaternion conjugate() const noexcept
  {
    return {this->m_w, -this->m_x, -this->m_y, -this->m_z};
  }

  [[nodiscard]] constexpr Real dot(Quaternion const& other) const noexcept
  {
    return this->m_w * other.m_w + this->m_x * other.m_x + this->m_y * other.m_y
         + this->m_z * other.m_z;
  }

  [[nodiscard]] constexpr Real length_squared() const noexcept { return this->dot(*this); }

  [[nodiscard]] Real length() const noexcept { return std::sqrt(this->length_squared()); }

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
  [[nodiscard]] std::optional<Quaternion> try_normalized() const noexcept
  {
    auto const len = this->length();
    if(not positive(len))
      return std::nullopt;
    return Quaternion(this->m_w / len, this->m_x / len, this->m_y / len, this->m_z / len);
  }

  /**
   * @brief Returns the unit quaternion of the same rotation.
   * @throws NumericError if the length is zero or not finite.
   * @see try_normalized
   */
  [[nodiscard]] Quaternion normalized() const;

  /**
   * @brief Returns the multiplicative inverse `q* / |q|²`; for a unit quaternion, the conjugate.
   */
  [[nodiscard]] constexpr Quaternion inverse() const noexcept
  {
    auto result = this->conjugate();
    result /= this->length_squared();
    return result;
  }

  /**
   * @brief Returns the rotation angle of this unit quaternion, in `[0°, 180°]`.
   */
  [[nodiscard]] Angle angle() const noexcept
  {
    return Angle::from_radians(2 * std::atan2(this->vector().length(), abs(this->m_w)));
  }

  /**
   * @brief Returns the angle of the smallest rotation that turns this unit quaternion into
   * @p other, in `[0°, 180°]`: the angle of `other ⊗ this*`.
   * @details Computed from the chord between the quaternions rather than an arc cosine, so it
   * stays accurate for nearby rotations.
   */
  [[nodiscard]] Angle angle_to(Quaternion const& other) const noexcept
  {
    auto const target = this->dot(other) < 0 ? -other : other;
    auto const chord = Quaternion(
      this->m_w - target.m_w,
      this->m_x - target.m_x,
      this->m_y - target.m_y,
      this->m_z - target.m_z
    );
    auto const sum = Quaternion(
      this->m_w + target.m_w,
      this->m_x + target.m_x,
      this->m_y + target.m_y,
      this->m_z + target.m_z
    );
    return Angle::from_radians(4 * std::atan2(chord.length(), sum.length()));
  }

  /**
   * @brief Normalized linear interpolation toward @p other along the shorter arc.
   * @details Cheaper than slerp() and exact at `t = 0` and `t = 1`, but its angular speed is not
   * constant. @p t is not clamped.
   * @pre Both quaternions are unit quaternions.
   * @see slerp
   */
  [[nodiscard]] Quaternion nlerp(Quaternion const& other, Real t) const noexcept;

  /**
   * @brief Spherical linear interpolation toward @p other along the shorter arc, at constant
   * angular speed.
   * @details Falls back to nlerp() when the rotations are within a milliradian, where both agree
   * and the spherical weights lose precision. @p t is not clamped.
   * @pre Both quaternions are unit quaternions.
   * @see nlerp
   */
  [[nodiscard]] Quaternion slerp(Quaternion const& other, Real t) const noexcept;

  /**
   * @brief Rotates @p v by this unit quaternion: `q ⊗ (0, v) ⊗ q*`.
   * @see inverse_rotate
   */
  [[nodiscard]] constexpr Vector3 rotate(Vector3 const v) const noexcept
  {
    auto const u = this->vector();
    auto const t = 2.0 * u.cross(v);
    return v + this->m_w * t + u.cross(t);
  }

  /**
   * @brief Rotates @p v by the inverse of this unit quaternion: `q* ⊗ (0, v) ⊗ q`.
   * @see rotate
   */
  [[nodiscard]] constexpr Vector3 inverse_rotate(Vector3 const v) const noexcept
  {
    return this->conjugate().rotate(v);
  }

  /**
   * @brief Returns the rotation matrix `m` of this unit quaternion: `m * v == rotate(v)`.
   * @details The inverse rotation is the transpose of the matrix.
   */
  [[nodiscard]] constexpr Matrix3x3 to_rotation_matrix() const noexcept
  {
    auto const w = this->m_w;
    auto const x = this->m_x;
    auto const y = this->m_y;
    auto const z = this->m_z;
    return Matrix3x3::from_rows(
      {1 - 2 * (y * y + z * z), 2 * (x * y - w * z), 2 * (x * z + w * y)},
      {2 * (x * y + w * z), 1 - 2 * (x * x + z * z), 2 * (y * z - w * x)},
      {2 * (x * z - w * y), 2 * (y * z + w * x), 1 - 2 * (x * x + y * y)}
    );
  }

  /**
   * @brief Returns the rotation matrix of this unit quaternion as an affine 4×4 transform without
   * translation.
   */
  [[nodiscard]] constexpr Matrix4x4 to_matrix4() const noexcept
  {
    return Matrix4x4::from_affine(this->to_rotation_matrix());
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
  [[nodiscard]] std::array<Angle, 3> to_euler() const noexcept;

  /**
   * @brief Returns the aerospace ZYX Euler angles of this unit quaternion.
   * @details Same as `to_euler<EulerOrder::ZYX>()`: roll and yaw are in `[-180°, 180°]`, pitch in
   * `[-90°, 90°]`. At ±90° pitch only the sum or difference of roll and yaw is defined; there the
   * roll is zero and the yaw carries the whole rotation about the vertical.
   * @see from_yaw_pitch_roll
   */
  [[nodiscard]] EulerAngles to_yaw_pitch_roll() const noexcept
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
  [[nodiscard]] EulerAngles
    to_euler_angles() const noexcept
  {
    return this->to_yaw_pitch_roll();
  }

  /**
   * @brief Returns the components in `x, y, z, w` order, as Jolt, HLSL and glm store them.
   * @see from_xyzw
   */
  [[nodiscard]] constexpr std::array<Real, 4> to_xyzw() const noexcept
  {
    return {this->m_x, this->m_y, this->m_z, this->m_w};
  }

  /**
   * @brief Returns the time derivative `q̇ = ½ q ⊗ (0, ω)` of an attitude turning at the angular
   * velocity @p omega_body.
   * @details @p omega_body is the angular velocity of the body relative to the reference frame,
   * in body axes, in radians per second. The result is per second.
   */
  [[nodiscard]] constexpr Quaternion derivative(Vector3 omega_body) const noexcept;

#ifdef W_COMMON_WITH_QT
  /**
   * @brief Converts to a Qt quaternion.
   * @details The components are narrowed to `float`.
   * @note Available only when wlib is built with `with_qt=True`.
   */
  [[nodiscard]] QQuaternion to_qquaternion() const noexcept;
#endif

  /**
   * @brief Returns the components as `(w, x, y, z)`.
   */
  [[nodiscard]] std::string to_string() const;

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] QString to_qstring() const;
#endif

  [[nodiscard]] static constexpr Quaternion identity() noexcept { return {}; }

  [[nodiscard]] static constexpr Quaternion from_wxyz(
    Real const w,
    Real const x,
    Real const y,
    Real const z
  ) noexcept
  {
    return {w, x, y, z};
  }

  /**
   * @brief Returns the quaternion of the components in `x, y, z, w` order, as Jolt, HLSL and glm
   * store them.
   * @see to_xyzw
   */
  [[nodiscard]] static constexpr Quaternion from_xyzw(
    Real const x,
    Real const y,
    Real const z,
    Real const w
  ) noexcept
  {
    return {w, x, y, z};
  }

  /**
   * @brief Returns the right-handed rotation by @p angle about @p axis.
   * @details @p axis need not be a unit vector.
   * @throws NumericError if @p axis is zero or not finite.
   */
  [[nodiscard]] static Quaternion from_axis_angle(Vector3 const axis, Angle const angle)
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
  [[nodiscard]] static Quaternion from_euler(Angle about_x, Angle about_y, Angle about_z) noexcept;

  /**
   * @brief Returns the attitude of the aerospace ZYX Euler angles.
   * @details `q = q_z(yaw) ⊗ q_y(pitch) ⊗ q_x(roll)`: yaw about the reference z axis, then pitch
   * about the new y axis, then roll about the new x axis. With NED reference axes and FRD body
   * axes, positive yaw turns the nose right, positive pitch raises it, and positive roll lowers
   * the right side. Same as `from_euler<EulerOrder::ZYX>(roll, pitch, yaw)`.
   * @see to_yaw_pitch_roll
   */
  [[nodiscard]] static Quaternion from_yaw_pitch_roll(EulerAngles const& angles) noexcept
  {
    return from_euler<EulerOrder::ZYX>(angles.roll(), angles.pitch(), angles.yaw());
  }

  /**
   * @brief Returns the attitude of the aerospace ZYX Euler angles.
   * @deprecated The name does not say which order; use from_yaw_pitch_roll(), or from_euler() for
   * another order.
   */
  [[deprecated("use from_yaw_pitch_roll(), or from_euler<Order>() for another order")]]
  [[nodiscard]] static Quaternion
    from_euler_angles(EulerAngles const& angles) noexcept
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
  [[nodiscard]] static Quaternion from_rotation_matrix(Matrix3x3 const& matrix) noexcept;

  /**
   * @brief Returns the shortest rotation that turns the direction @p from into the direction
   * @p to.
   * @details Neither needs to be a unit vector. Opposite directions have no unique shortest
   * rotation; the result is then the half turn about an axis perpendicular to @p from.
   * @throws NumericError if either vector is zero or not finite.
   */
  [[nodiscard]] static Quaternion from_to(Vector3 from, Vector3 to);

  /**
   * @brief Returns the orientation whose forward axis points along @p forward and whose up axis
   * leans toward @p up: the camera-to-world rotation of look_at().
   * @details The forward axis is −z for Handedness::Right and +z for Handedness::Left; y is up and
   * x completes the basis, so the result is the inverse of the rotation in `look_at(eye, eye +
   * forward, up, handedness)`. Where @p up is parallel to @p forward, the roll is arbitrary and
   * the result is from_to() the forward axis.
   * @throws NumericError if @p forward is zero or not finite.
   */
  [[nodiscard]] static Quaternion look_rotation(
    Vector3 const forward,
    Vector3 const up,
    Handedness const handedness
  )
  {
    auto const forward_axis = handedness == Handedness::Right ? -Vector3::unit_z()
                                                              : Vector3::unit_z();
    auto const direction = forward.try_normalized();
    if(not direction.has_value())
      throw NumericError("cannot look along {}", forward);

    auto const z = handedness == Handedness::Right ? -*direction : *direction;
    auto const x = up.cross(z).try_normalized();
    if(not x.has_value())
      return from_to(forward_axis, *direction);
    return from_rotation_matrix(Matrix3x3::from_columns(*x, z.cross(*x), z));
  }

#ifdef W_COMMON_WITH_QT
  /// @note Available only when wlib is built with `with_qt=True`.
  [[nodiscard]] static Quaternion from_qquaternion(QQuaternion const& quaternion) noexcept;
#endif

  [[nodiscard]] constexpr bool operator==(Quaternion const&) const noexcept = default;

  [[nodiscard]] constexpr Quaternion operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr Quaternion operator-() const noexcept
  {
    return {-this->m_w, -this->m_x, -this->m_y, -this->m_z};
  }

  constexpr Quaternion& operator+=(Quaternion const& rhs) noexcept
  {
    this->m_w += rhs.m_w;
    this->m_x += rhs.m_x;
    this->m_y += rhs.m_y;
    this->m_z += rhs.m_z;
    return *this;
  }

  constexpr Quaternion& operator-=(Quaternion const& rhs) noexcept
  {
    this->m_w -= rhs.m_w;
    this->m_x -= rhs.m_x;
    this->m_y -= rhs.m_y;
    this->m_z -= rhs.m_z;
    return *this;
  }

  constexpr Quaternion& operator*=(Real const rhs) noexcept
  {
    this->m_w *= rhs;
    this->m_x *= rhs;
    this->m_y *= rhs;
    this->m_z *= rhs;
    return *this;
  }

  constexpr Quaternion& operator/=(Real const rhs) noexcept
  {
    this->m_w /= rhs;
    this->m_x /= rhs;
    this->m_y /= rhs;
    this->m_z /= rhs;
    return *this;
  }

 private:
  Real m_w = 1.0;
  Real m_x = 0.0;
  Real m_y = 0.0;
  Real m_z = 0.0;
};

[[nodiscard]] constexpr Quaternion operator+(Quaternion lhs, Quaternion const& rhs) noexcept
{
  lhs += rhs;
  return lhs;
}

[[nodiscard]] constexpr Quaternion operator-(Quaternion lhs, Quaternion const& rhs) noexcept
{
  lhs -= rhs;
  return lhs;
}

[[nodiscard]] constexpr Quaternion operator*(Quaternion lhs, Real const rhs) noexcept
{
  lhs *= rhs;
  return lhs;
}

[[nodiscard]] constexpr Quaternion operator*(Real const lhs, Quaternion rhs) noexcept
{
  rhs *= lhs;
  return rhs;
}

[[nodiscard]] constexpr Quaternion operator/(Quaternion lhs, Real const rhs) noexcept
{
  lhs /= rhs;
  return lhs;
}

/**
 * @brief Returns the Hamilton product `a ⊗ b`: the rotation `b` followed by `a`.
 * @relates Quaternion
 */
[[nodiscard]] constexpr Quaternion operator*(Quaternion const& a, Quaternion const& b) noexcept
{
  return {
    a.w() * b.w() - a.x() * b.x() - a.y() * b.y() - a.z() * b.z(),
    a.w() * b.x() + a.x() * b.w() + a.y() * b.z() - a.z() * b.y(),
    a.w() * b.y() - a.x() * b.z() + a.y() * b.w() + a.z() * b.x(),
    a.w() * b.z() + a.x() * b.y() - a.y() * b.x() + a.z() * b.w()
  };
}

constexpr Quaternion Quaternion::derivative(Vector3 const omega_body) const noexcept
{
  return 0.5 * (*this * Quaternion(0.0, omega_body));
}

inline Quaternion Quaternion::nlerp(Quaternion const& other, Real const t) const noexcept
{
  auto const target = this->dot(other) < 0 ? -other : other;
  auto const blended = *this * (1 - t) + target * t;
  return blended / blended.length();
}

inline Quaternion Quaternion::slerp(Quaternion const& other, Real const t) const noexcept
{
  auto const target = this->dot(other) < 0 ? -other : other;
  // half of angle_to(): the arc between the quaternions on the unit sphere
  auto const arc = this->angle_to(target).radians() / 2;
  if(arc < 1e-3F)
    return this->nlerp(target, t);
  auto const sin_arc = std::sin(arc);
  return (*this * std::sin((1 - t) * arc) + target * std::sin(t * arc)) / sin_arc;
}

inline Quaternion Quaternion::from_rotation_matrix(Matrix3x3 const& matrix) noexcept
{
  auto const [m00, m01, m02, m10, m11, m12, m20, m21, m22] = std::bit_cast<std::array<Real, 9>>(
    matrix
  );

  // Shepperd's method: derive the largest of |w|, |x|, |y|, |z| from the diagonal, so the
  // divisor of the other three components is never small
  auto const trace = m00 + m11 + m22;
  auto result = Quaternion();
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

inline Quaternion Quaternion::from_to(Vector3 const from, Vector3 const to)
{
  auto const u = from.try_normalized();
  auto const v = to.try_normalized();
  if(not u.has_value() or not v.has_value())
    throw NumericError("cannot rotate {} onto {}", from, to);

  auto const cos_angle = u->dot(*v);
  // note: [whs31 : 2026-10-26]
  // near the half turn the cross product vanishes, so the axis comes from a coordinate axis
  // instead; float resolves the remaining angle to about a milliradian
  if(cos_angle < -1 + 1e-6F) {
    auto axis = Vector3::unit_x().cross(*u);
    if(axis.length_squared() < 1e-6F)
      axis = Vector3::unit_y().cross(*u);
    return {0.0, axis.normalized()};
  }
  auto const q = Quaternion(1 + cos_angle, u->cross(*v));
  return q / q.length();
}

/**
 * @brief Returns `a.nlerp(b, t)`.
 * @relates Quaternion
 */
[[nodiscard]] inline Quaternion nlerp(
  Quaternion const& a,
  Quaternion const& b,
  Real const t
) noexcept
{
  return a.nlerp(b, t);
}

/**
 * @brief Returns `a.slerp(b, t)`.
 * @relates Quaternion
 */
[[nodiscard]] inline Quaternion slerp(
  Quaternion const& a,
  Quaternion const& b,
  Real const t
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
    Real sign = 1;
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
   * first and last terms, which scale with it; about the square root of the epsilon of a float.
   */
  inline auto constexpr singular_cos = 3e-4F;
}  // namespace _euler_impl

template <EulerOrder Order>
Quaternion Quaternion::from_euler(
  Angle const about_x,
  Angle const about_y,
  Angle const about_z
) noexcept
{
  auto constexpr axes = _euler_impl::axes(Order);
  auto const angles = std::array {about_x, about_y, about_z};
  auto const about = [&angles](std::size_t const axis) {
    auto const half = angles.at(axis) / 2;
    auto vector = std::array<Real, 3> {};
    vector.at(axis) = half.sin();
    return Quaternion(half.cos(), std::bit_cast<Vector3>(vector));
  };
  return about(axes.first) * about(axes.second) * about(axes.third);
}

template <EulerOrder Order>
std::array<Angle, 3> Quaternion::to_euler() const noexcept
{
  auto constexpr axes = _euler_impl::axes(Order);
  auto const [i, j, k, sign] = axes;
  auto const elements = std::bit_cast<std::array<Real, 9>>(this->to_rotation_matrix());
  auto const m = [&elements](std::size_t const row, std::size_t const column) {
    return elements.at(3 * row + column);
  };

  // R = R_i(a) R_j(b) R_k(c): R[i][k] = ±sin b, R[i][i] and R[i][j] carry c scaled by cos b,
  // R[k][k] and R[j][k] carry a scaled by cos b
  auto const cos_middle = std::hypot(m(i, i), m(i, j));
  auto const middle = std::atan2(sign * m(i, k), cos_middle);
  auto first = Real {0};
  auto last = Real {0};
  if(cos_middle < _euler_impl::singular_cos)
    first = std::atan2(sign * m(k, j), m(j, j));
  else {
    first = std::atan2(-sign * m(j, k), m(k, k));
    last = std::atan2(-sign * m(i, j), m(i, i));
  }

  auto result = std::array<Angle, 3> {};
  result.at(i) = Angle::from_radians(first);
  result.at(j) = Angle::from_radians(middle);
  result.at(k) = Angle::from_radians(last);
  return result;
}

W_COMMON_API std::ostream& operator<<(std::ostream& os, Quaternion const& quaternion);

static_assert(std::is_standard_layout_v<Quaternion>);
static_assert(std::is_trivially_copyable_v<Quaternion>);
static_assert(sizeof(Quaternion) == 4 * sizeof(Real));
static_assert(
  std::bit_cast<std::array<Real, 4>>(Quaternion(1.0, 2.0, 3.0, 4.0))
    == std::array<Real, 4> {1, 2, 3, 4},
  "Quaternion must store w, x, y, z contiguously"
);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Quaternion>
{
  [[nodiscard]] std::size_t operator()(w::common::Quaternion const& q) const noexcept
  {
    auto hash = std::hash<w::common::Real> {}(q.w());
    for(auto const value : {q.x(), q.y(), q.z()})
      hash ^= std::hash<w::common::Real> {}(value) + 0x9e3779b97f4a7c15ull + (hash << 6U)
            + (hash >> 2U);
    return hash;
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(w, x, y, z)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <>
struct fmt::formatter<w::common::Quaternion> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Quaternion const& q, format_context& ctx) const
  {
    return w::common::_format_impl::format_components<4>(*this, {q.w(), q.x(), q.y(), q.z()}, ctx);
  }
};
#endif

W_COMMON_NAMESPACE_BEGIN

// note: [whs31 : 2026-10-26]
// defined after the formatter, which the error message needs
inline Quaternion Quaternion::normalized() const
{
  if(auto const result = this->try_normalized())
    return *result;
  throw NumericError("cannot normalize the quaternion {}", *this);
}

W_COMMON_NAMESPACE_END

#ifdef W_COMMON_WITH_QT
Q_DECLARE_METATYPE(w::common::Quaternion)
#endif
