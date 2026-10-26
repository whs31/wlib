#pragma once

#include <cstdint>
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <optional>
#include <string>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/detail/export.h>
#include <w/common/detail/format.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>
#include <w/common/precision.h>
#include <w/common/vector2.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Axis-aligned rectangle of Real coordinates: its min corner `(x, y)` and its size, e.g. a
 * UI element or a viewport.
 * @details The rectangle is agnostic of the direction of y: with y down, as in UI and window
 * coordinates, min() is the top-left corner; with y up it is the bottom-left one. It holds the
 * points from min() inclusive to max() exclusive. A rectangle with a non-positive width or height
 * is empty.
 *
 * @par Layout
 * The storage is exactly four contiguous Real values in `x, y, width, height` order. The class is
 * standard layout and trivially copyable.
 * @ingroup core_math
 * @see Recti
 */
class W_COMMON_API Rect
{
 public:
  constexpr Rect() noexcept = default;

  constexpr Rect(Real const x, Real const y, Real const width, Real const height) noexcept
    : m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
  {}

  [[nodiscard]] constexpr Real x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr Real y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr Real width() const noexcept { return this->m_width; }

  [[nodiscard]] constexpr Real height() const noexcept { return this->m_height; }

  [[nodiscard]] constexpr Vector2 min() const noexcept { return {this->m_x, this->m_y}; }

  [[nodiscard]] constexpr Vector2 max() const noexcept
  {
    return {this->m_x + this->m_width, this->m_y + this->m_height};
  }

  [[nodiscard]] constexpr Vector2 size() const noexcept { return {this->m_width, this->m_height}; }

  [[nodiscard]] constexpr Vector2 center() const noexcept
  {
    return {this->m_x + this->m_width / 2, this->m_y + this->m_height / 2};
  }

  [[nodiscard]] constexpr Real area() const noexcept
  {
    return this->is_empty() ? Real {0} : this->m_width * this->m_height;
  }

  /**
   * @brief Returns width over height.
   */
  [[nodiscard]] constexpr Real aspect_ratio() const noexcept
  {
    return this->m_width / this->m_height;
  }

  [[nodiscard]] constexpr bool is_empty() const noexcept
  {
    return not(this->m_width > 0 and this->m_height > 0);
  }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_x) and common::is_finite(this->m_y)
       and common::is_finite(this->m_width) and common::is_finite(this->m_height);
  }

  /**
   * @brief Checks whether @p point is in `[min(), max())`.
   */
  [[nodiscard]] constexpr bool contains(Vector2 const point) const noexcept
  {
    auto const max = this->max();
    return point.x() >= this->m_x and point.x() < max.x() and point.y() >= this->m_y
       and point.y() < max.y();
  }

  /**
   * @brief Checks whether @p other is a non-empty rectangle entirely inside this one.
   */
  [[nodiscard]] constexpr bool contains(Rect const& other) const noexcept
  {
    return not other.is_empty() and other.m_x >= this->m_x and other.m_y >= this->m_y
       and other.max().x() <= this->max().x() and other.max().y() <= this->max().y();
  }

  /**
   * @brief Checks whether the rectangles share an area; touching edges do not.
   */
  [[nodiscard]] constexpr bool intersects(Rect const& other) const noexcept
  {
    return this->intersection(other).has_value();
  }

  /**
   * @brief Returns the overlap of the rectangles, or `std::nullopt` if they share no area.
   */
  [[nodiscard]] constexpr std::optional<Rect> intersection(Rect const& other) const noexcept
  {
    auto const result = from_min_max(this->min().max(other.min()), this->max().min(other.max()));
    if(result.is_empty())
      return std::nullopt;
    return result;
  }

  /**
   * @brief Returns the smallest rectangle that holds both; an empty one does not count.
   */
  [[nodiscard]] constexpr Rect united(Rect const& other) const noexcept
  {
    if(other.is_empty())
      return *this;
    if(this->is_empty())
      return other;
    return from_min_max(this->min().min(other.min()), this->max().max(other.max()));
  }

  [[nodiscard]] constexpr Rect translated(Vector2 const offset) const noexcept
  {
    return {this->m_x + offset.x(), this->m_y + offset.y(), this->m_width, this->m_height};
  }

  /**
   * @brief Returns the rectangle grown by @p margin on every side; a negative margin shrinks it.
   */
  [[nodiscard]] constexpr Rect expanded(Real const margin) const noexcept
  {
    return {
      this->m_x - margin,
      this->m_y - margin,
      this->m_width + 2 * margin,
      this->m_height + 2 * margin,
    };
  }

  /**
   * @brief Returns the rectangle with every coordinate multiplied by @p factor, e.g. from the
   * low-resolution target to the window.
   */
  [[nodiscard]] constexpr Rect scaled(Real const factor) const noexcept
  {
    return {
      this->m_x * factor,
      this->m_y * factor,
      this->m_width * factor,
      this->m_height * factor,
    };
  }

  [[nodiscard]] constexpr std::array<Real, 4> to_array() const noexcept
  {
    return {this->m_x, this->m_y, this->m_width, this->m_height};
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] static constexpr Rect from_min_max(Vector2 const min, Vector2 const max) noexcept
  {
    return {min.x(), min.y(), max.x() - min.x(), max.y() - min.y()};
  }

  [[nodiscard]] static constexpr Rect from_position_size(
    Vector2 const position,
    Vector2 const size
  ) noexcept
  {
    return {position.x(), position.y(), size.x(), size.y()};
  }

  [[nodiscard]] static constexpr Rect from_center_size(
    Vector2 const center,
    Vector2 const size
  ) noexcept
  {
    return from_position_size(center - size / 2, size);
  }

  [[nodiscard]] constexpr bool operator==(Rect const&) const noexcept = default;

 private:
  Real m_x = 0.0;
  Real m_y = 0.0;
  Real m_width = 0.0;
  Real m_height = 0.0;
};

/**
 * @brief Axis-aligned rectangle of integer coordinates: its min corner `(x, y)` and its size, e.g.
 * a region of the 640×360 render target or a texture atlas.
 * @details It holds the pixels from `(x, y)` inclusive to `(max_x(), max_y())` exclusive, and is
 * agnostic of the direction of y like Rect. A rectangle with a non-positive width or height is
 * empty.
 *
 * @par Layout
 * The storage is exactly four contiguous `std::int32_t` values in `x, y, width, height` order. The
 * class is standard layout and trivially copyable.
 * @ingroup core_math
 * @see Rect
 */
class W_COMMON_API Recti
{
 public:
  constexpr Recti() noexcept = default;

  constexpr Recti(
    std::int32_t const x,
    std::int32_t const y,
    std::int32_t const width,
    std::int32_t const height
  ) noexcept
    : m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
  {}

  [[nodiscard]] constexpr std::int32_t x() const noexcept { return this->m_x; }

  [[nodiscard]] constexpr std::int32_t y() const noexcept { return this->m_y; }

  [[nodiscard]] constexpr std::int32_t width() const noexcept { return this->m_width; }

  [[nodiscard]] constexpr std::int32_t height() const noexcept { return this->m_height; }

  /**
   * @brief Returns `x() + width()`, the first column past the rectangle.
   */
  [[nodiscard]] constexpr std::int32_t max_x() const noexcept { return this->m_x + this->m_width; }

  /**
   * @brief Returns `y() + height()`, the first row past the rectangle.
   */
  [[nodiscard]] constexpr std::int32_t max_y() const noexcept { return this->m_y + this->m_height; }

  [[nodiscard]] constexpr std::int64_t area() const noexcept
  {
    return this->is_empty() ? 0 : std::int64_t {this->m_width} * this->m_height;
  }

  [[nodiscard]] constexpr bool is_empty() const noexcept
  {
    return this->m_width <= 0 or this->m_height <= 0;
  }

  /**
   * @brief Checks whether the pixel @p px, @p py is inside.
   */
  [[nodiscard]] constexpr bool contains(std::int32_t const px, std::int32_t const py) const noexcept
  {
    return px >= this->m_x and px < this->max_x() and py >= this->m_y and py < this->max_y();
  }

  /**
   * @brief Checks whether @p other is a non-empty rectangle entirely inside this one.
   */
  [[nodiscard]] constexpr bool contains(Recti const& other) const noexcept
  {
    return not other.is_empty() and other.m_x >= this->m_x and other.m_y >= this->m_y
       and other.max_x() <= this->max_x() and other.max_y() <= this->max_y();
  }

  /**
   * @brief Checks whether the rectangles share a pixel.
   */
  [[nodiscard]] constexpr bool intersects(Recti const& other) const noexcept
  {
    return this->intersection(other).has_value();
  }

  /**
   * @brief Returns the overlap of the rectangles, or `std::nullopt` if they share no pixel.
   */
  [[nodiscard]] constexpr std::optional<Recti> intersection(Recti const& other) const noexcept
  {
    auto const x = std::max(this->m_x, other.m_x);
    auto const y = std::max(this->m_y, other.m_y);
    auto const result = Recti(
      x,
      y,
      std::min(this->max_x(), other.max_x()) - x,
      std::min(this->max_y(), other.max_y()) - y
    );
    if(result.is_empty())
      return std::nullopt;
    return result;
  }

  /**
   * @brief Returns the smallest rectangle that holds both; an empty one does not count.
   */
  [[nodiscard]] constexpr Recti united(Recti const& other) const noexcept
  {
    if(other.is_empty())
      return *this;
    if(this->is_empty())
      return other;
    auto const x = std::min(this->m_x, other.m_x);
    auto const y = std::min(this->m_y, other.m_y);
    return {
      x,
      y,
      std::max(this->max_x(), other.max_x()) - x,
      std::max(this->max_y(), other.max_y()) - y,
    };
  }

  [[nodiscard]] constexpr Recti translated(
    std::int32_t const dx,
    std::int32_t const dy
  ) const noexcept
  {
    return {this->m_x + dx, this->m_y + dy, this->m_width, this->m_height};
  }

  /**
   * @brief Returns the same rectangle in Real coordinates.
   */
  [[nodiscard]] constexpr Rect to_rect() const noexcept
  {
    return {
      static_cast<Real>(this->m_x),
      static_cast<Real>(this->m_y),
      static_cast<Real>(this->m_width),
      static_cast<Real>(this->m_height),
    };
  }

  [[nodiscard]] constexpr std::array<std::int32_t, 4> to_array() const noexcept
  {
    return {this->m_x, this->m_y, this->m_width, this->m_height};
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] static constexpr Recti from_size(
    std::int32_t const width,
    std::int32_t const height
  ) noexcept
  {
    return {0, 0, width, height};
  }

  /**
   * @brief Returns the smallest pixel rectangle that covers @p rect: its min corner rounded down
   * and its max corner rounded up.
   */
  [[nodiscard]] static Recti enclosing(Rect const& rect) noexcept
  {
    auto const x = static_cast<std::int32_t>(std::floor(rect.x()));
    auto const y = static_cast<std::int32_t>(std::floor(rect.y()));
    return {
      x,
      y,
      static_cast<std::int32_t>(std::ceil(rect.max().x())) - x,
      static_cast<std::int32_t>(std::ceil(rect.max().y())) - y,
    };
  }

  [[nodiscard]] constexpr bool operator==(Recti const&) const noexcept = default;

 private:
  std::int32_t m_x = 0;
  std::int32_t m_y = 0;
  std::int32_t m_width = 0;
  std::int32_t m_height = 0;
};

W_COMMON_API std::ostream& operator<<(std::ostream& os, Rect const& rect);

W_COMMON_API std::ostream& operator<<(std::ostream& os, Recti const& rect);

static_assert(std::is_standard_layout_v<Rect>);
static_assert(std::is_trivially_copyable_v<Rect>);
static_assert(sizeof(Rect) == 4 * sizeof(Real));
static_assert(std::is_standard_layout_v<Recti>);
static_assert(std::is_trivially_copyable_v<Recti>);
static_assert(sizeof(Recti) == 16);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Rect>
{
  [[nodiscard]] std::size_t operator()(w::common::Rect const& rect) const noexcept
  {
    return w::common::_hash_impl::combine(rect.x(), rect.y(), rect.width(), rect.height());
  }
};

template <>
struct std::hash<w::common::Recti>
{
  [[nodiscard]] std::size_t operator()(w::common::Recti const& rect) const noexcept
  {
    return w::common::_hash_impl::combine(rect.x(), rect.y(), rect.width(), rect.height());
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(x, y, width, height)`; the format spec, e.g. `{:.3f}`, applies to every
 * component.
 */
template <>
struct fmt::formatter<w::common::Rect> : fmt::formatter<w::common::Real>
{
  format_context::iterator format(w::common::Rect const& rect, format_context& ctx) const
  {
    return w::common::_format_impl::format_components<4>(*this, rect.to_array(), ctx);
  }
};

/**
 * @brief Formats as `(x, y, width, height)`; the format spec, e.g. `{:>4}`, applies to every
 * component.
 */
template <>
struct fmt::formatter<w::common::Recti> : fmt::formatter<std::int32_t>
{
  format_context::iterator format(w::common::Recti const& rect, format_context& ctx) const
  {
    auto out = ctx.out();
    *out++ = '(';
    auto first = true;
    for(auto const value : rect.to_array()) {
      if(not first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
      ctx.advance_to(out);
      out = fmt::formatter<std::int32_t>::format(value, ctx);
    }
    *out++ = ')';
    return out;
  }
};
#endif
