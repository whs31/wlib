#pragma once

#include <concepts>
#include <cstddef>
#include <algorithm>
#include <array>
#include <bit>
#include <functional>
#include <iosfwd>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/config.h>
#include <w/common/detail/format.h>
#include <w/common/detail/namespace.h>
#include <w/common/precision.h>
#include <w/common/error.h>
#include <w/common/math.h>
#include <w/common/vector3.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief 3×3 matrix of @p T values, stored as three row vectors.
 * @details Row-major storage, column vectors: `(m * v).x() == m.row(0).dot(v)`, and `a * b` applies
 * `b` first. The default value is the zero matrix. See @ref conventions.
 *
 * Typical uses are rotation matrices, inertia tensors and small linear systems. It is not a
 * general linear-algebra library; use Eigen when more is needed.
 *
 * @par Layout
 * The storage is exactly three contiguous Vector3 rows, i.e. nine contiguous @p T values in
 * row-major order, with no padding. The class is standard layout and trivially copyable, so it can
 * be reflected as `Vector3[3]` by offset or bit-cast to `std::array<Vector3, 3>`. The layout is
 * part of the API and is checked by `static_assert`s in this header.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * // Euler's rotation equation: I ω̇ = τ − ω × (I ω)
 * auto const inertia_body = Matrix3x3::diagonal({0.5, 3.2, 3.0});
 * auto const omega_body = Vector3(0.1, -0.2, 0.3);
 * auto const torque_body = Vector3(0.0, 0.0, 1.5);
 * auto const omega_dot_body =
 *   inertia_body.inverse() * (torque_body - omega_body.cross(inertia_body * omega_body));
 * @endcode
 *
 * @ingroup core_math
 * @see Vector3
 * @see Quaternion
 * @see Matrix4x4
 */
template <std::floating_point T>
class BasicMatrix3x3
{
 public:
  /**
   * @brief Default tolerance of is_invertible(), relative to the product of the row magnitudes.
   */
  static constexpr T singular_tolerance = T {1e-5};

  /**
   * @brief Default tolerance of is_symmetric(), relative to the largest element.
   */
  static constexpr T symmetric_tolerance = T {1e-5};

  /**
   * @brief Creates a zero matrix.
   */
  constexpr BasicMatrix3x3() noexcept = default;

  /**
   * @brief Converts from another precision: implicitly when no value can lose digits, e.g. from
   * `float` to `double`, and explicitly otherwise.
   */
  template <std::floating_point U>
    requires(not std::same_as<U, T>)
  constexpr explicit(not LosslessConversion<U, T>) BasicMatrix3x3(
    BasicMatrix3x3<U> const& other
  ) noexcept
    : BasicMatrix3x3(
        BasicVector3<T>(std::get<0>(other.m_rows)),
        BasicVector3<T>(std::get<1>(other.m_rows)),
        BasicVector3<T>(std::get<2>(other.m_rows))
      )
  {}

  /**
   * @brief Returns the row at @p index.
   * @throws std::out_of_range if @p index is not in `[0, 3)`.
   */
  [[nodiscard]] constexpr BasicVector3<T> row(std::size_t const index) const
  {
    return this->m_rows.at(index);
  }

  /**
   * @brief Returns the column at @p index.
   * @throws std::out_of_range if @p index is not in `[0, 3)`.
   */
  [[nodiscard]] constexpr BasicVector3<T> column(std::size_t const index) const
  {
    auto const& [a, b, c] = this->m_rows;
    return {component(a, index), component(b, index), component(c, index)};
  }

  /**
   * @brief Returns the element in @p row and @p column.
   * @throws std::out_of_range if either index is not in `[0, 3)`.
   */
  [[nodiscard]] constexpr T at(std::size_t const row, std::size_t const column) const
  {
    return component(this->m_rows.at(row), column);
  }

  /**
   * @brief Returns the main diagonal `(m00, m11, m22)`.
   */
  [[nodiscard]] constexpr BasicVector3<T> diagonal() const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    return {a.x(), b.y(), c.z()};
  }

  [[nodiscard]] constexpr BasicMatrix3x3<T> transposed() const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    return from_columns(a, b, c);
  }

  [[nodiscard]] constexpr T determinant() const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    return a.dot(b.cross(c));
  }

  [[nodiscard]] constexpr T trace() const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    return a.x() + b.y() + c.z();
  }

  /**
   * @brief Checks whether every element is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    return a.is_finite() and b.is_finite() and c.is_finite();
  }

  /**
   * @brief Checks whether the matrix equals its transpose within @p tolerance relative to its
   * largest element, e.g. an inertia tensor after a change of frame.
   * @details A non-finite matrix is never symmetric.
   */
  [[nodiscard]] constexpr bool is_symmetric(T const tolerance = symmetric_tolerance) const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    auto const limit = tolerance * this->max_abs_element();
    return this->is_finite() and abs(a.y() - b.x()) <= limit and abs(a.z() - c.x()) <= limit
       and abs(b.z() - c.y()) <= limit;
  }

  /**
   * @brief Checks whether inverse() succeeds.
   * @details The matrix counts as singular when it is not finite or when its determinant is not
   * greater than @p tolerance times the product of the largest absolute elements of its rows.
   * That product bounds the determinant up to a constant factor, so the test does not depend on
   * the scale of the rows: a tiny but well-conditioned matrix is invertible.
   */
  [[nodiscard]] constexpr bool is_invertible(T const tolerance = singular_tolerance) const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    auto const det = this->determinant();
    return this->is_finite() and common::is_finite(det)
       and abs(det)
             > tolerance * max_abs_component(a) * max_abs_component(b) * max_abs_component(c);
  }

  /**
   * @brief Returns the inverse matrix, or `std::nullopt` unless is_invertible().
   * @see inverse
   */
  [[nodiscard]] constexpr std::optional<BasicMatrix3x3<T>> try_inverse() const noexcept
  {
    if(not this->is_invertible())
      return std::nullopt;
    auto result = this->adjugate();
    result /= this->determinant();
    return result;
  }

  /**
   * @brief Returns the inverse matrix.
   * @throws NumericError unless is_invertible().
   * @see try_inverse
   */
  [[nodiscard]] constexpr BasicMatrix3x3<T> inverse() const
  {
    if(auto const result = this->try_inverse())
      return *result;
    throw NumericError("cannot invert a singular matrix, determinant {}", this->determinant());
  }

  /**
   * @brief Returns the rows as `[(a, b, c), (d, e, f), (g, h, i)]`.
   */
  [[nodiscard]] std::string to_string() const { return fmt::format("{}", *this); }

  [[nodiscard]] static constexpr BasicMatrix3x3<T> zero() noexcept { return {}; }

  [[nodiscard]] static constexpr BasicMatrix3x3<T> identity() noexcept
  {
    return diagonal(BasicVector3<T>::splat(1.0));
  }

  /**
   * @brief Returns the diagonal matrix with @p diagonal on its main diagonal.
   */
  [[nodiscard]] static constexpr BasicMatrix3x3<T> diagonal(BasicVector3<T> const diagonal) noexcept
  {
    return from_rows({diagonal.x(), 0.0, 0.0}, {0.0, diagonal.y(), 0.0}, {0.0, 0.0, diagonal.z()});
  }

  [[nodiscard]] static constexpr BasicMatrix3x3<T> from_rows(
    BasicVector3<T> const a,
    BasicVector3<T> const b,
    BasicVector3<T> const c
  ) noexcept
  {
    return {a, b, c};
  }

  [[nodiscard]] static constexpr BasicMatrix3x3<T> from_columns(
    BasicVector3<T> const a,
    BasicVector3<T> const b,
    BasicVector3<T> const c
  ) noexcept
  {
    return from_rows({a.x(), b.x(), c.x()}, {a.y(), b.y(), c.y()}, {a.z(), b.z(), c.z()});
  }

  /**
   * @brief Returns the skew-symmetric cross-product matrix of @p v: `m * u == v.cross(u)`.
   */
  [[nodiscard]] static constexpr BasicMatrix3x3<T> cross_product(BasicVector3<T> const v) noexcept
  {
    return from_rows({0.0, -v.z(), v.y()}, {v.z(), 0.0, -v.x()}, {-v.y(), v.x(), 0.0});
  }

  [[nodiscard]] constexpr bool operator==(BasicMatrix3x3<T> const&) const noexcept = default;

  [[nodiscard]] constexpr BasicMatrix3x3<T> operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr BasicMatrix3x3<T> operator-() const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    return from_rows(-a, -b, -c);
  }

  constexpr BasicMatrix3x3<T>& operator+=(BasicMatrix3x3<T> const& rhs) noexcept
  {
    auto& [a, b, c] = this->m_rows;
    auto const& [rhs_a, rhs_b, rhs_c] = rhs.m_rows;
    a += rhs_a;
    b += rhs_b;
    c += rhs_c;
    return *this;
  }

  constexpr BasicMatrix3x3<T>& operator-=(BasicMatrix3x3<T> const& rhs) noexcept
  {
    auto& [a, b, c] = this->m_rows;
    auto const& [rhs_a, rhs_b, rhs_c] = rhs.m_rows;
    a -= rhs_a;
    b -= rhs_b;
    c -= rhs_c;
    return *this;
  }

  constexpr BasicMatrix3x3<T>& operator*=(T const rhs) noexcept
  {
    for(auto& row : this->m_rows)
      row *= rhs;
    return *this;
  }

  constexpr BasicMatrix3x3<T>& operator/=(T const rhs) noexcept
  {
    for(auto& row : this->m_rows)
      row /= rhs;
    return *this;
  }

  [[nodiscard]] friend constexpr BasicMatrix3x3<T> operator+(
    BasicMatrix3x3<T> lhs,
    BasicMatrix3x3<T> const& rhs
  ) noexcept
  {
    lhs += rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicMatrix3x3<T> operator-(
    BasicMatrix3x3<T> lhs,
    BasicMatrix3x3<T> const& rhs
  ) noexcept
  {
    lhs -= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicMatrix3x3<T> operator*(
    BasicMatrix3x3<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs *= rhs;
    return lhs;
  }

  [[nodiscard]] friend constexpr BasicMatrix3x3<T> operator*(
    T const lhs,
    BasicMatrix3x3<T> rhs
  ) noexcept
  {
    rhs *= lhs;
    return rhs;
  }

  [[nodiscard]] friend constexpr BasicMatrix3x3<T> operator/(
    BasicMatrix3x3<T> lhs,
    T const rhs
  ) noexcept
  {
    lhs /= rhs;
    return lhs;
  }

  /**
   * @brief Returns the matrix-vector product `m v`.
   * @relates Matrix3x3
   */
  [[nodiscard]] friend constexpr BasicVector3<T> operator*(
    BasicMatrix3x3<T> const& m,
    BasicVector3<T> const v
  ) noexcept
  {
    return {m.row(0).dot(v), m.row(1).dot(v), m.row(2).dot(v)};
  }

  /**
   * @brief Returns the matrix product `lhs rhs`: `(lhs * rhs) * v == lhs * (rhs * v)`.
   * @relates Matrix3x3
   */
  [[nodiscard]] friend constexpr BasicMatrix3x3<T> operator*(
    BasicMatrix3x3<T> const& lhs,
    BasicMatrix3x3<T> const& rhs
  ) noexcept
  {
    auto const columns = rhs.transposed();
    return BasicMatrix3x3<
      T>::from_rows(columns * lhs.row(0), columns * lhs.row(1), columns * lhs.row(2));
  }

 private:
  template <std::floating_point>
  friend class BasicMatrix3x3;

  constexpr BasicMatrix3x3(
    BasicVector3<T> const a,
    BasicVector3<T> const b,
    BasicVector3<T> const c
  ) noexcept
    : m_rows {a, b, c}
  {}

  [[nodiscard]] static constexpr T component(BasicVector3<T> const v, std::size_t const index)
  {
    switch(index) {
      case 0: return v.x();
      case 1: return v.y();
      case 2: return v.z();
      default: throw std::out_of_range("BasicMatrix3x3<T> index out of range");
    }
  }

  [[nodiscard]] static constexpr T max_abs_component(BasicVector3<T> const v) noexcept
  {
    auto const x = abs(v.x());
    auto const y = abs(v.y());
    auto const z = abs(v.z());
    return std::max({x, y, z});
  }

  [[nodiscard]] constexpr T max_abs_element() const noexcept
  {
    auto result = T {0};
    for(auto const& row : this->m_rows)
      result = std::max(result, max_abs_component(row));
    return result;
  }

  // the columns of the adjugate are the cross products of the other two rows
  [[nodiscard]] constexpr BasicMatrix3x3<T> adjugate() const noexcept
  {
    auto const& [a, b, c] = this->m_rows;
    return from_columns(b.cross(c), c.cross(a), a.cross(b));
  }

  std::array<BasicVector3<T>, 3> m_rows {};
};

/**
 * @brief Matrix3x3 in single precision, the default.
 */
using Matrix3x3 = BasicMatrix3x3<float>;

template <std::floating_point T>
std::ostream& operator<<(std::ostream& os, BasicMatrix3x3<T> const& matrix)
{
  return os << matrix.to_string();
}

static_assert(std::is_standard_layout_v<Matrix3x3>);
static_assert(std::is_trivially_copyable_v<Matrix3x3>);
static_assert(sizeof(Matrix3x3) == 9 * sizeof(float));
static_assert(
  std::bit_cast<std::array<float, 9>>(
    Matrix3x3::from_rows({1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0})
  ) == std::array<float, 9> {1, 2, 3, 4, 5, 6, 7, 8, 9},
  "Matrix3x3 must store three contiguous rows"
);

W_COMMON_NAMESPACE_END

template <std::floating_point T>
struct std::hash<w::common::BasicMatrix3x3<T>>
{
  [[nodiscard]] std::size_t operator()(w::common::BasicMatrix3x3<T> const& matrix) const noexcept
  {
    auto hash = std::hash<w::common::BasicVector3<T>> {}(matrix.row(0));
    hash ^= std::hash<w::common::BasicVector3<T>> {}(matrix.row(1)) + 0x9e3779b97f4a7c15ull
          + (hash << 6U) + (hash >> 2U);
    hash ^= std::hash<w::common::BasicVector3<T>> {}(matrix.row(2)) + 0x9e3779b97f4a7c15ull
          + (hash << 6U) + (hash >> 2U);
    return hash;
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `[(a, b, c), (d, e, f), (g, h, i)]` row by row; the format spec, e.g.
 * `{:.3f}`, applies to every element.
 */
template <std::floating_point T>
struct fmt::formatter<w::common::BasicMatrix3x3<T>> : fmt::formatter<T>
{
  format_context::iterator format(
    w::common::BasicMatrix3x3<T> const& matrix,
    format_context& ctx
  ) const
  {
    auto out = ctx.out();
    *out++ = '[';
    for(auto index = std::size_t {0}; index < 3; ++index) {
      if(index != 0) {
        *out++ = ',';
        *out++ = ' ';
      }
      ctx.advance_to(out);
      auto const row = matrix.row(index);
      out = w::common::_format_impl::
        format_components<3, T>(*this, {row.x(), row.y(), row.z()}, ctx);
    }
    *out++ = ']';
    return out;
  }
};
#endif
