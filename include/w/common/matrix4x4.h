#pragma once

#include <cstddef>
#include <algorithm>
#include <array>
#include <bit>
#include <functional>
#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/detail/export.h>
#include <w/common/detail/format.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/error.h>
#include <w/common/math.h>
#include <w/common/matrix3x3.h>
#include <w/common/precision.h>
#include <w/common/vector3.h>
#include <w/common/vector4.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief 4×4 matrix of Real values, stored as four row vectors: affine transforms, views and
 * projections.
 * @details Row-major storage, column vectors: `(m * v).x() == m.row(0).dot(v)`, the translation
 * of an affine transform is the last column, and `a * b` applies `b` first, so
 * `projection * view * model * point` reads right to left. The default value is the zero matrix.
 * See @ref conventions for how the storage reaches HLSL without a transpose.
 *
 * A position and a direction transform differently: transform_point() treats its argument as
 * `(p, 1)` and translates it, transform_direction() as `(d, 0)` and does not.
 *
 * @par Layout
 * The storage is exactly four contiguous Vector4 rows, i.e. sixteen contiguous Real values in
 * row-major order (64 bytes), with no padding: `m.at(r, c)` is element `4 * r + c` of
 * `std::bit_cast<std::array<Real, 16>>(m)`. The class is standard layout and trivially copyable.
 * The layout is part of the API and is checked by `static_assert`s in this header.
 *
 * @code {.cpp}
 * using namespace w;
 *
 * auto const model = Matrix4x4::from_translation({0.0, 1.5, 0.0});
 * auto const view = look_at_rh({0.0, 2.0, 5.0}, {0.0, 1.0, 0.0}, Vector3::unit_y());
 * auto const projection = perspective(60_deg, 16.0F / 9.0F, 0.1F, 100.0F, Handedness::Right);
 * auto const clip = projection * view * model * Vector4::from_point(Vector3::zero());
 * @endcode
 *
 * @ingroup core_math
 * @see Matrix3x3
 * @see Vector4
 * @see Transform
 */
class W_COMMON_API Matrix4x4
{
 public:
  /**
   * @brief Default tolerance of is_invertible(), relative to the product of the row magnitudes.
   */
  static constexpr Real singular_tolerance = 1e-6F;

  /**
   * @brief Creates a zero matrix.
   */
  constexpr Matrix4x4() noexcept = default;

  /**
   * @brief Returns the row at @p index.
   * @throws std::out_of_range if @p index is not in `[0, 4)`.
   */
  [[nodiscard]] constexpr Vector4 row(std::size_t const index) const
  {
    return this->m_rows.at(index);
  }

  /**
   * @brief Returns the column at @p index.
   * @throws std::out_of_range if @p index is not in `[0, 4)`.
   */
  [[nodiscard]] constexpr Vector4 column(std::size_t const index) const
  {
    auto const& [a, b, c, d] = this->m_rows;
    return {component(a, index), component(b, index), component(c, index), component(d, index)};
  }

  /**
   * @brief Returns the element in @p row and @p column.
   * @throws std::out_of_range if either index is not in `[0, 4)`.
   */
  [[nodiscard]] constexpr Real at(std::size_t const row, std::size_t const column) const
  {
    return component(this->m_rows.at(row), column);
  }

  /**
   * @brief Returns the translation of an affine transform: the first three elements of the last
   * column.
   */
  [[nodiscard]] constexpr Vector3 translation() const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    return {a.w(), b.w(), c.w()};
  }

  /**
   * @brief Returns the upper-left 3×3 block: the rotation and scale of an affine transform.
   */
  [[nodiscard]] constexpr Matrix3x3 linear() const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    return Matrix3x3::from_rows(a.xyz(), b.xyz(), c.xyz());
  }

  [[nodiscard]] constexpr Matrix4x4 transposed() const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    return from_columns(a, b, c, d);
  }

  [[nodiscard]] constexpr Real determinant() const noexcept
  {
    return Cofactors(*this).determinant();
  }

  [[nodiscard]] constexpr Real trace() const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    return a.x() + b.y() + c.z() + d.w();
  }

  /**
   * @brief Checks whether every element is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return std::ranges::all_of(this->m_rows, &Vector4::is_finite);
  }

  /**
   * @brief Checks whether the last row is exactly `(0, 0, 0, 1)`, i.e. the matrix is an affine
   * transform and not a projection.
   */
  [[nodiscard]] constexpr bool is_affine() const noexcept
  {
    return this->m_rows.back() == Vector4::unit_w();
  }

  /**
   * @brief Checks whether inverse() succeeds.
   * @details The matrix counts as singular when it is not finite or when the determinant of its
   * equilibrated copy, every row and then every column scaled so its largest absolute element is
   * 1, is not greater than @p tolerance. That determinant is at most 16 and does not depend on the
   * scale of the matrix or of its rows, and the column pass undoes the imbalance of a large
   * translation, so neither a tiny but well-conditioned matrix nor a view matrix far from the
   * origin counts as singular.
   */
  [[nodiscard]] constexpr bool is_invertible(
    Real const tolerance = singular_tolerance
  ) const noexcept
  {
    if(not this->is_finite() or not common::is_finite(this->determinant()))
      return false;
    auto rows = this->m_rows;
    for(auto& row : rows) {
      auto const largest = max_abs_component(row);
      if(largest == 0)
        return false;
      row /= largest;
    }
    auto equilibrated = Matrix4x4(rows[0], rows[1], rows[2], rows[3]).transposed();
    auto columns = equilibrated.m_rows;
    for(auto& column : columns) {
      auto const largest = max_abs_component(column);
      if(largest == 0)
        return false;
      column /= largest;
    }
    equilibrated = Matrix4x4(columns[0], columns[1], columns[2], columns[3]);
    return abs(equilibrated.determinant()) > tolerance;
  }

  /**
   * @brief Returns the inverse matrix, or `std::nullopt` unless is_invertible().
   * @see inverse
   */
  [[nodiscard]] constexpr std::optional<Matrix4x4> try_inverse() const noexcept
  {
    if(not this->is_invertible())
      return std::nullopt;
    return Cofactors(*this).inverse();
  }

  /**
   * @brief Returns the inverse matrix.
   * @throws NumericError unless is_invertible().
   * @see try_inverse
   */
  [[nodiscard]] constexpr Matrix4x4 inverse() const
  {
    if(auto const result = this->try_inverse())
      return *result;
    throw NumericError("cannot invert a singular matrix, determinant {}", this->determinant());
  }

  /**
   * @brief Transforms the position @p point, i.e. `(point, 1)`, and drops `w`.
   * @details Right for an affine transform, whose `w` stays 1; use project_point() for a
   * projection.
   * @see transform_direction
   */
  [[nodiscard]] constexpr Vector3 transform_point(Vector3 const point) const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    auto const p = Vector4::from_point(point);
    return {a.dot(p), b.dot(p), c.dot(p)};
  }

  /**
   * @brief Transforms the position @p point, i.e. `(point, 1)`, and divides by the resulting `w`.
   * @details Maps a view-space point through a projection to normalized device coordinates.
   */
  [[nodiscard]] constexpr Vector3 project_point(Vector3 const point) const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    auto const p = Vector4::from_point(point);
    return Vector4(a.dot(p), b.dot(p), c.dot(p), d.dot(p)).perspective_divided();
  }

  /**
   * @brief Transforms the direction @p direction, i.e. `(direction, 0)`: rotation and scale, no
   * translation.
   * @details A normal transforms with the inverse transpose instead, unless the scale is uniform.
   * @see transform_point
   */
  [[nodiscard]] constexpr Vector3 transform_direction(Vector3 const direction) const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    auto const v = Vector4::from_direction(direction);
    return {a.dot(v), b.dot(v), c.dot(v)};
  }

  /**
   * @brief Returns the sixteen elements in row-major order, as they are stored.
   */
  [[nodiscard]] constexpr std::array<Real, 16> to_array() const noexcept
  {
    return std::bit_cast<std::array<Real, 16>>(*this);
  }

  /**
   * @brief Returns the rows as `[(a, b, c, d), (e, f, g, h), ...]`.
   */
  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] static constexpr Matrix4x4 zero() noexcept { return {}; }

  [[nodiscard]] static constexpr Matrix4x4 identity() noexcept
  {
    return diagonal(Vector4::splat(1.0));
  }

  /**
   * @brief Returns the diagonal matrix with @p diagonal on its main diagonal.
   */
  [[nodiscard]] static constexpr Matrix4x4 diagonal(Vector4 const diagonal) noexcept
  {
    return from_rows(
      {diagonal.x(), 0.0, 0.0, 0.0},
      {0.0, diagonal.y(), 0.0, 0.0},
      {0.0, 0.0, diagonal.z(), 0.0},
      {0.0, 0.0, 0.0, diagonal.w()}
    );
  }

  [[nodiscard]] static constexpr Matrix4x4 from_rows(
    Vector4 const a,
    Vector4 const b,
    Vector4 const c,
    Vector4 const d
  ) noexcept
  {
    return {a, b, c, d};
  }

  [[nodiscard]] static constexpr Matrix4x4 from_columns(
    Vector4 const a,
    Vector4 const b,
    Vector4 const c,
    Vector4 const d
  ) noexcept
  {
    return from_rows(
      {a.x(), b.x(), c.x(), d.x()},
      {a.y(), b.y(), c.y(), d.y()},
      {a.z(), b.z(), c.z(), d.z()},
      {a.w(), b.w(), c.w(), d.w()}
    );
  }

  /**
   * @brief Returns the matrix of the sixteen @p elements in row-major order.
   */
  [[nodiscard]] static constexpr Matrix4x4 from_array(std::array<Real, 16> const& elements) noexcept
  {
    return std::bit_cast<Matrix4x4>(elements);
  }

  /**
   * @brief Returns the affine transform with @p linear as its upper-left block and the
   * translation @p translation.
   */
  [[nodiscard]] static constexpr Matrix4x4 from_affine(
    Matrix3x3 const& linear,
    Vector3 const translation = {}
  ) noexcept
  {
    return from_rows(
      {linear.row(0), translation.x()},
      {linear.row(1), translation.y()},
      {linear.row(2), translation.z()},
      Vector4::unit_w()
    );
  }

  /**
   * @brief Returns the translation by @p offset.
   */
  [[nodiscard]] static constexpr Matrix4x4 from_translation(Vector3 const offset) noexcept
  {
    return from_affine(Matrix3x3::identity(), offset);
  }

  /**
   * @brief Returns the scale by @p factors along each axis.
   */
  [[nodiscard]] static constexpr Matrix4x4 from_scale(Vector3 const factors) noexcept
  {
    return diagonal({factors, 1.0});
  }

  [[nodiscard]] constexpr bool operator==(Matrix4x4 const&) const noexcept = default;

  [[nodiscard]] constexpr Matrix4x4 operator+() const noexcept { return *this; }

  [[nodiscard]] constexpr Matrix4x4 operator-() const noexcept
  {
    auto const& [a, b, c, d] = this->m_rows;
    return from_rows(-a, -b, -c, -d);
  }

  constexpr Matrix4x4& operator+=(Matrix4x4 const& rhs) noexcept
  {
    for(auto index = std::size_t {0}; index < this->m_rows.size(); ++index)
      this->m_rows.at(index) += rhs.m_rows.at(index);
    return *this;
  }

  constexpr Matrix4x4& operator-=(Matrix4x4 const& rhs) noexcept
  {
    for(auto index = std::size_t {0}; index < this->m_rows.size(); ++index)
      this->m_rows.at(index) -= rhs.m_rows.at(index);
    return *this;
  }

  constexpr Matrix4x4& operator*=(Real const rhs) noexcept
  {
    for(auto& row : this->m_rows)
      row *= rhs;
    return *this;
  }

  constexpr Matrix4x4& operator/=(Real const rhs) noexcept
  {
    for(auto& row : this->m_rows)
      row /= rhs;
    return *this;
  }

 private:
  constexpr Matrix4x4(Vector4 const a, Vector4 const b, Vector4 const c, Vector4 const d) noexcept
    : m_rows {a, b, c, d}
  {}

  [[nodiscard]] static constexpr Real component(Vector4 const v, std::size_t const index)
  {
    switch(index) {
      case 0: return v.x();
      case 1: return v.y();
      case 2: return v.z();
      case 3: return v.w();
      default: throw std::out_of_range("Matrix4x4 index out of range");
    }
  }

  [[nodiscard]] static constexpr Real max_abs_component(Vector4 const v) noexcept
  {
    return std::max({abs(v.x()), abs(v.y()), abs(v.z()), abs(v.w())});
  }

  /**
   * @brief The 2×2 minors of the upper and the lower two rows, from which the determinant and the
   * adjugate follow by Laplace expansion.
   */
  class Cofactors
  {
   public:
    explicit constexpr Cofactors(Matrix4x4 const& m) noexcept
      : m_rows(m.m_rows)
    {
      auto const& [a, b, c, d] = this->m_rows;
      this->m_upper = {
        a.x() * b.y() - b.x() * a.y(),
        a.x() * b.z() - b.x() * a.z(),
        a.x() * b.w() - b.x() * a.w(),
        a.y() * b.z() - b.y() * a.z(),
        a.y() * b.w() - b.y() * a.w(),
        a.z() * b.w() - b.z() * a.w(),
      };
      this->m_lower = {
        c.x() * d.y() - d.x() * c.y(),
        c.x() * d.z() - d.x() * c.z(),
        c.x() * d.w() - d.x() * c.w(),
        c.y() * d.z() - d.y() * c.z(),
        c.y() * d.w() - d.y() * c.w(),
        c.z() * d.w() - d.z() * c.w(),
      };
    }

    [[nodiscard]] constexpr Real determinant() const noexcept
    {
      auto const& [s0, s1, s2, s3, s4, s5] = this->m_upper;
      auto const& [c0, c1, c2, c3, c4, c5] = this->m_lower;
      return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    }

    [[nodiscard]] constexpr Matrix4x4 inverse() const noexcept
    {
      auto const& [a, b, c, d] = this->m_rows;
      auto const& [s0, s1, s2, s3, s4, s5] = this->m_upper;
      auto const& [c0, c1, c2, c3, c4, c5] = this->m_lower;
      auto result = from_rows(
        {
          b.y() * c5 - b.z() * c4 + b.w() * c3,
          -a.y() * c5 + a.z() * c4 - a.w() * c3,
          d.y() * s5 - d.z() * s4 + d.w() * s3,
          -c.y() * s5 + c.z() * s4 - c.w() * s3,
        },
        {
          -b.x() * c5 + b.z() * c2 - b.w() * c1,
          a.x() * c5 - a.z() * c2 + a.w() * c1,
          -d.x() * s5 + d.z() * s2 - d.w() * s1,
          c.x() * s5 - c.z() * s2 + c.w() * s1,
        },
        {
          b.x() * c4 - b.y() * c2 + b.w() * c0,
          -a.x() * c4 + a.y() * c2 - a.w() * c0,
          d.x() * s4 - d.y() * s2 + d.w() * s0,
          -c.x() * s4 + c.y() * s2 - c.w() * s0,
        },
        {
          -b.x() * c3 + b.y() * c1 - b.z() * c0,
          a.x() * c3 - a.y() * c1 + a.z() * c0,
          -d.x() * s3 + d.y() * s1 - d.z() * s0,
          c.x() * s3 - c.y() * s1 + c.z() * s0,
        }
      );
      result /= this->determinant();
      return result;
    }

   private:
    std::array<Vector4, 4> m_rows;
    std::array<Real, 6> m_upper {};
    std::array<Real, 6> m_lower {};
  };

  std::array<Vector4, 4> m_rows {};
};

[[nodiscard]] constexpr Matrix4x4 operator+(Matrix4x4 lhs, Matrix4x4 const& rhs) noexcept
{
  lhs += rhs;
  return lhs;
}

[[nodiscard]] constexpr Matrix4x4 operator-(Matrix4x4 lhs, Matrix4x4 const& rhs) noexcept
{
  lhs -= rhs;
  return lhs;
}

[[nodiscard]] constexpr Matrix4x4 operator*(Matrix4x4 lhs, Real const rhs) noexcept
{
  lhs *= rhs;
  return lhs;
}

[[nodiscard]] constexpr Matrix4x4 operator*(Real const lhs, Matrix4x4 rhs) noexcept
{
  rhs *= lhs;
  return rhs;
}

[[nodiscard]] constexpr Matrix4x4 operator/(Matrix4x4 lhs, Real const rhs) noexcept
{
  lhs /= rhs;
  return lhs;
}

/**
 * @brief Returns the matrix-vector product `m v`.
 * @relates Matrix4x4
 */
[[nodiscard]] constexpr Vector4 operator*(Matrix4x4 const& m, Vector4 const v) noexcept
{
  return {m.row(0).dot(v), m.row(1).dot(v), m.row(2).dot(v), m.row(3).dot(v)};
}

/**
 * @brief Returns the matrix product `lhs rhs`: `(lhs * rhs) * v == lhs * (rhs * v)`.
 * @relates Matrix4x4
 */
[[nodiscard]] constexpr Matrix4x4 operator*(Matrix4x4 const& lhs, Matrix4x4 const& rhs) noexcept
{
  auto const columns = rhs.transposed();
  return Matrix4x4::from_rows(
    columns * lhs.row(0),
    columns * lhs.row(1),
    columns * lhs.row(2),
    columns * lhs.row(3)
  );
}

W_COMMON_API std::ostream& operator<<(std::ostream& os, Matrix4x4 const& matrix);

static_assert(std::is_standard_layout_v<Matrix4x4>);
static_assert(std::is_trivially_copyable_v<Matrix4x4>);
static_assert(sizeof(Matrix4x4) == 64);
static_assert(
  Matrix4x4::from_rows({0, 1, 2, 3}, {4, 5, 6, 7}, {8, 9, 10, 11}, {12, 13, 14, 15}).to_array()
    == std::array<Real, 16> {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  "Matrix4x4 must store four contiguous rows"
);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Matrix4x4>
{
  [[nodiscard]] std::size_t operator()(w::common::Matrix4x4 const& matrix) const noexcept
  {
    return w::common::_hash_impl::
      combine(matrix.row(0), matrix.row(1), matrix.row(2), matrix.row(3));
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `[(a, b, c, d), (e, f, g, h), ...]` row by row; the format spec, e.g. `{:.3f}`,
 * applies to every element.
 */
template <>
struct fmt::formatter<w::common::Matrix4x4> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Matrix4x4 const& matrix, format_context& ctx) const
  {
    auto out = ctx.out();
    *out++ = '[';
    for(auto index = std::size_t {0}; index < 4; ++index) {
      if(index != 0) {
        *out++ = ',';
        *out++ = ' ';
      }
      ctx.advance_to(out);
      out = w::common::_format_impl::format_components<4>(*this, matrix.row(index).to_array(), ctx);
    }
    *out++ = ']';
    return out;
  }
};
#endif
