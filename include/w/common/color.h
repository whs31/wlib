#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <functional>
#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

#include <fmt/format.h>

#include <w/common/stdint.h>
#include <w/common/angle.h>
#include <w/common/detail/export.h>
#include <w/common/detail/hash.h>
#include <w/common/detail/namespace.h>
#include <w/common/math.h>

/**
 * @file
 * @brief Colors: linear Rgba for lighting and blending, sRGB-encoded Rgba8 for textures and
 * palettes, and Hsv for palette tooling.
 * @details Shading and blending happen on linear values; 8-bit storage and every color a person
 * picks or types, e.g. `"#e4a672"`, are sRGB-encoded. The conversions apply the exact sRGB
 * transfer function (IEC 61966-2-1). Alpha is linear in both.
 */

W_COMMON_NAMESPACE_BEGIN

class Rgba8;

/**
 * @brief Decodes the sRGB-encoded component @p encoded, in `[0, 1]`, to linear light.
 * @ingroup core_math
 * @see linear_to_srgb
 */
[[nodiscard]] inline float srgb_to_linear(float const encoded) noexcept
{
  if(encoded <= 0.04045F)
    return encoded / 12.92F;
  return std::pow((encoded + 0.055F) / 1.055F, 2.4F);
}

/**
 * @brief Encodes the linear component @p linear, in `[0, 1]`, to sRGB.
 * @ingroup core_math
 * @see srgb_to_linear
 */
[[nodiscard]] inline float linear_to_srgb(float const linear) noexcept
{
  if(linear <= 0.0031308F)
    return linear * 12.92F;
  return 1.055F * std::pow(linear, 1.0F / 2.4F) - 0.055F;
}

/**
 * @brief Hue, saturation and value of a color, as color pickers show them.
 * @details HSV always describes the sRGB-encoded color, not linear light, so its value steps look
 * even. The hue is in `[0°, 360°)`, zero for grays; saturation, value and alpha are in `[0, 1]`.
 * @ingroup core_math
 * @see Rgba8::to_hsv
 * @see Rgba::to_hsv
 */
struct Hsv
{
  Angle hue;
  float saturation = 0.0F;
  float value = 0.0F;
  float alpha = 1.0F;

  [[nodiscard]] constexpr bool operator==(Hsv const&) const noexcept = default;
};

namespace _color_impl  // NOLINT(*-identifier-naming)
{
  struct Components
  {
    float r = 0.0F;
    float g = 0.0F;
    float b = 0.0F;
  };

  [[nodiscard]] inline Hsv to_hsv(Components const c, float const alpha) noexcept
  {
    auto const max = std::max({c.r, c.g, c.b});
    auto const delta = max - std::min({c.r, c.g, c.b});
    auto degrees = 0.0F;
    if(delta > 0) {
      if(max == c.r)
        degrees = 60.0F * std::fmod((c.g - c.b) / delta, 6.0F);
      else if(max == c.g)
        degrees = 60.0F * ((c.b - c.r) / delta + 2.0F);
      else
        degrees = 60.0F * ((c.r - c.g) / delta + 4.0F);
      if(degrees < 0)
        degrees += 360.0F;
    }
    return {
      .hue = Angle::from_degrees(degrees),
      .saturation = max > 0 ? delta / max : 0.0F,
      .value = max,
      .alpha = alpha,
    };
  }

  [[nodiscard]] inline Components from_hsv(Hsv const& hsv) noexcept
  {
    auto const sector = std::fmod(hsv.hue.positive().degrees(), 360.0F) / 60.0F;
    auto const chroma = hsv.value * hsv.saturation;
    auto const second = chroma * (1.0F - std::abs(std::fmod(sector, 2.0F) - 1.0F));
    auto const m = hsv.value - chroma;
    auto rgb = Components {};
    switch(static_cast<int>(sector)) {
      case 0: rgb = {.r = chroma, .g = second, .b = 0}; break;
      case 1: rgb = {.r = second, .g = chroma, .b = 0}; break;
      case 2: rgb = {.r = 0, .g = chroma, .b = second}; break;
      case 3: rgb = {.r = 0, .g = second, .b = chroma}; break;
      case 4: rgb = {.r = second, .g = 0, .b = chroma}; break;
      default: rgb = {.r = chroma, .g = 0, .b = second}; break;
    }
    return {.r = rgb.r + m, .g = rgb.g + m, .b = rgb.b + m};
  }

  [[nodiscard]] constexpr int hex_digit(char const c) noexcept
  {
    if(c >= '0' and c <= '9')
      return c - '0';
    if(c >= 'a' and c <= 'f')
      return c - 'a' + 10;
    if(c >= 'A' and c <= 'F')
      return c - 'A' + 10;
    return -1;
  }

  [[nodiscard]] inline u8 to_byte(float const value) noexcept
  {
    return static_cast<u8>(std::lround(std::clamp(value, 0.0F, 1.0F) * 255.0F));
  }
}  // namespace _color_impl

/**
 * @brief Linear-light RGBA color of `float` components, for lighting, blending and GPU
 * constants.
 * @details Components are usually in `[0, 1]` but may exceed 1 for HDR. Alpha is straight, not
 * premultiplied; premultiplied() converts. Arithmetic is component-wise, and `a * b` modulates.
 * The default value is transparent black.
 *
 * @par Layout
 * The storage is exactly four contiguous `float` values in `r, g, b, a` order, 16 bytes, so it
 * bit-casts to an HLSL `float4` whatever the platform. The class is standard layout and trivially
 * copyable.
 * @ingroup core_math
 * @see Rgba8
 */
class W_COMMON_API Rgba
{
 public:
  constexpr Rgba() noexcept = default;

  constexpr Rgba(float const r, float const g, float const b, float const a = 1.0F) noexcept
    : m_r(r)
    , m_g(g)
    , m_b(b)
    , m_a(a)
  {}

  [[nodiscard]] constexpr float r() const noexcept { return this->m_r; }

  [[nodiscard]] constexpr float g() const noexcept { return this->m_g; }

  [[nodiscard]] constexpr float b() const noexcept { return this->m_b; }

  [[nodiscard]] constexpr float a() const noexcept { return this->m_a; }

  [[nodiscard]] constexpr Rgba with_alpha(float const a) const noexcept
  {
    return {this->m_r, this->m_g, this->m_b, a};
  }

  /**
   * @brief Checks whether every component is neither infinite nor NaN.
   */
  [[nodiscard]] constexpr bool is_finite() const noexcept
  {
    return common::is_finite(this->m_r) and common::is_finite(this->m_g)
       and common::is_finite(this->m_b) and common::is_finite(this->m_a);
  }

  /**
   * @brief Returns the color with its red, green and blue multiplied by its alpha.
   */
  [[nodiscard]] constexpr Rgba premultiplied() const noexcept
  {
    return {this->m_r * this->m_a, this->m_g * this->m_a, this->m_b * this->m_a, this->m_a};
  }

  /**
   * @brief Returns the color with every component clamped into `[0, 1]`.
   */
  [[nodiscard]] constexpr Rgba clamped() const noexcept
  {
    return {
      std::clamp(this->m_r, 0.0F, 1.0F),
      std::clamp(this->m_g, 0.0F, 1.0F),
      std::clamp(this->m_b, 0.0F, 1.0F),
      std::clamp(this->m_a, 0.0F, 1.0F),
    };
  }

  /**
   * @brief Returns the relative luminance, the Rec. 709 weighted sum of the linear components.
   */
  [[nodiscard]] constexpr float luminance() const noexcept
  {
    return 0.2126F * this->m_r + 0.7152F * this->m_g + 0.0722F * this->m_b;
  }

  /**
   * @brief Returns `*this + (other - *this) * t`, in linear light; @p t is not clamped.
   */
  [[nodiscard]] constexpr Rgba lerp(Rgba const& other, float const t) const noexcept
  {
    return {
      this->m_r + (other.m_r - this->m_r) * t,
      this->m_g + (other.m_g - this->m_g) * t,
      this->m_b + (other.m_b - this->m_b) * t,
      this->m_a + (other.m_a - this->m_a) * t,
    };
  }

  /**
   * @brief Returns the sRGB encoding, every component clamped into `[0, 1]` and rounded to 8 bits.
   */
  [[nodiscard]] Rgba8 to_srgb8() const noexcept;

  /**
   * @brief Returns the hue, saturation and value of the sRGB encoding of the color, clamped into
   * `[0, 1]`, without rounding to 8 bits.
   */
  [[nodiscard]] Hsv to_hsv() const noexcept
  {
    auto const c = this->clamped();
    return _color_impl::to_hsv(
      {
        .r = linear_to_srgb(c.m_r),
        .g = linear_to_srgb(c.m_g),
        .b = linear_to_srgb(c.m_b),
      },
      c.m_a
    );
  }

  [[nodiscard]] constexpr std::array<float, 4> to_array() const noexcept
  {
    return {this->m_r, this->m_g, this->m_b, this->m_a};
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] static constexpr Rgba transparent() noexcept { return {0.0F, 0.0F, 0.0F, 0.0F}; }

  [[nodiscard]] static constexpr Rgba black() noexcept { return {0.0F, 0.0F, 0.0F}; }

  [[nodiscard]] static constexpr Rgba white() noexcept { return {1.0F, 1.0F, 1.0F}; }

  /**
   * @brief Returns the linear color of the sRGB-encoded @p color.
   */
  [[nodiscard]] static Rgba from_srgb8(Rgba8 const& color) noexcept;

  /**
   * @brief Returns the linear color of the sRGB-encoded components @p r, @p g, @p b in
   * `[0, 1]`, without rounding to 8 bits.
   */
  [[nodiscard]] static Rgba from_srgb(
    float const r,
    float const g,
    float const b,
    float const a = 1.0F
  ) noexcept
  {
    return {srgb_to_linear(r), srgb_to_linear(g), srgb_to_linear(b), a};
  }

  /**
   * @brief Returns the linear color of @p hsv, which describes the sRGB encoding.
   */
  [[nodiscard]] static Rgba from_hsv(Hsv const& hsv) noexcept
  {
    auto const [r, g, b] = _color_impl::from_hsv(hsv);
    return from_srgb(r, g, b, hsv.alpha);
  }

  /**
   * @brief Returns the linear color of the sRGB hex string @p text, as Rgba8::from_hex() reads
   * it, or `std::nullopt` if it is malformed.
   */
  [[nodiscard]] static std::optional<Rgba> from_hex(std::string_view text) noexcept;

  [[nodiscard]] constexpr bool operator==(Rgba const&) const noexcept = default;

  constexpr Rgba& operator+=(Rgba const& rhs) noexcept
  {
    this->m_r += rhs.m_r;
    this->m_g += rhs.m_g;
    this->m_b += rhs.m_b;
    this->m_a += rhs.m_a;
    return *this;
  }

  constexpr Rgba& operator-=(Rgba const& rhs) noexcept
  {
    this->m_r -= rhs.m_r;
    this->m_g -= rhs.m_g;
    this->m_b -= rhs.m_b;
    this->m_a -= rhs.m_a;
    return *this;
  }

  /**
   * @brief Modulates: multiplies component by component.
   */
  constexpr Rgba& operator*=(Rgba const& rhs) noexcept
  {
    this->m_r *= rhs.m_r;
    this->m_g *= rhs.m_g;
    this->m_b *= rhs.m_b;
    this->m_a *= rhs.m_a;
    return *this;
  }

  constexpr Rgba& operator*=(float const rhs) noexcept
  {
    this->m_r *= rhs;
    this->m_g *= rhs;
    this->m_b *= rhs;
    this->m_a *= rhs;
    return *this;
  }

  constexpr Rgba& operator/=(float const rhs) noexcept
  {
    this->m_r /= rhs;
    this->m_g /= rhs;
    this->m_b /= rhs;
    this->m_a /= rhs;
    return *this;
  }

 private:
  float m_r = 0.0F;
  float m_g = 0.0F;
  float m_b = 0.0F;
  float m_a = 0.0F;
};

[[nodiscard]] constexpr Rgba operator+(Rgba lhs, Rgba const& rhs) noexcept
{
  lhs += rhs;
  return lhs;
}

[[nodiscard]] constexpr Rgba operator-(Rgba lhs, Rgba const& rhs) noexcept
{
  lhs -= rhs;
  return lhs;
}

/**
 * @brief Modulates: multiplies component by component.
 * @relates Rgba
 */
[[nodiscard]] constexpr Rgba operator*(Rgba lhs, Rgba const& rhs) noexcept
{
  lhs *= rhs;
  return lhs;
}

[[nodiscard]] constexpr Rgba operator*(Rgba lhs, float const rhs) noexcept
{
  lhs *= rhs;
  return lhs;
}

[[nodiscard]] constexpr Rgba operator*(float const lhs, Rgba rhs) noexcept
{
  rhs *= lhs;
  return rhs;
}

[[nodiscard]] constexpr Rgba operator/(Rgba lhs, float const rhs) noexcept
{
  lhs /= rhs;
  return lhs;
}

/**
 * @brief sRGB-encoded RGBA color of 8-bit components, e.g. a texel or a palette entry.
 * @details The red, green and blue bytes are sRGB-encoded, as an `*_UNORM_SRGB` texture stores
 * them; alpha is linear. Convert to Rgba before blending or lighting. The default value is
 * transparent black.
 *
 * @par Layout
 * The storage is exactly four bytes in `r, g, b, a` order, the byte order of an `RGBA8` texel. The
 * class is standard layout and trivially copyable.
 *
 * @code {.cpp}
 * using namespace w::literals;
 *
 * auto constexpr sand = "#e4a672"_rgba8;
 * auto const lit = sand.to_linear() * light;
 * @endcode
 * @ingroup core_math
 * @see Rgba
 */
class W_COMMON_API Rgba8
{
 public:
  constexpr Rgba8() noexcept = default;

  constexpr Rgba8(u8 const r, u8 const g, u8 const b, u8 const a = 255) noexcept
    : m_r(r)
    , m_g(g)
    , m_b(b)
    , m_a(a)
  {}

  [[nodiscard]] constexpr u8 r() const noexcept { return this->m_r; }

  [[nodiscard]] constexpr u8 g() const noexcept { return this->m_g; }

  [[nodiscard]] constexpr u8 b() const noexcept { return this->m_b; }

  [[nodiscard]] constexpr u8 a() const noexcept { return this->m_a; }

  [[nodiscard]] constexpr Rgba8 with_alpha(u8 const a) const noexcept
  {
    return {this->m_r, this->m_g, this->m_b, a};
  }

  /**
   * @brief Returns the linear color: red, green and blue decoded from sRGB.
   */
  [[nodiscard]] Rgba to_linear() const noexcept
  {
    return Rgba::from_srgb(
      static_cast<float>(this->m_r) / 255.0F,
      static_cast<float>(this->m_g) / 255.0F,
      static_cast<float>(this->m_b) / 255.0F,
      static_cast<float>(this->m_a) / 255.0F
    );
  }

  /**
   * @brief Returns the hue, saturation and value of the color.
   */
  [[nodiscard]] Hsv to_hsv() const noexcept
  {
    return _color_impl::to_hsv(
      {
        .r = static_cast<float>(this->m_r) / 255.0F,
        .g = static_cast<float>(this->m_g) / 255.0F,
        .b = static_cast<float>(this->m_b) / 255.0F,
      },
      static_cast<float>(this->m_a) / 255.0F
    );
  }

  /**
   * @brief Returns `"#rrggbb"`, or `"#rrggbbaa"` unless the color is opaque, in lowercase.
   */
  [[nodiscard]] std::string to_hex() const;

  /**
   * @brief Returns to_hex().
   */
  [[nodiscard]] std::string to_string() const;

  /**
   * @brief Returns the components as `0xRRGGBBAA`.
   */
  [[nodiscard]] constexpr u32 to_rgba32() const noexcept
  {
    return (u32 {this->m_r} << 24U) | (u32 {this->m_g} << 16U) | (u32 {this->m_b} << 8U)
         | u32 {this->m_a};
  }

  /**
   * @brief Returns the color of a hex string: `#rgb`, `#rgba`, `#rrggbb` or `#rrggbbaa`, in
   * either case, the `#` optional, or `std::nullopt` if it is anything else.
   * @details A missing alpha is opaque. The short forms repeat each digit, so `#e4a` is
   * `#ee44aa`.
   */
  [[nodiscard]] static constexpr std::optional<Rgba8> from_hex(std::string_view text) noexcept
  {
    if(text.starts_with('#'))
      text.remove_prefix(1);
    auto const short_form = text.size() == 3 or text.size() == 4;
    if(not short_form and text.size() != 6 and text.size() != 8)
      return std::nullopt;

    auto components = std::array<u8, 4> {0, 0, 0, 255};
    auto const digits = short_form ? usize {1} : usize {2};
    for(auto index = usize {0}; index * digits < text.size(); ++index) {
      auto value = 0;
      for(auto const c : text.substr(index * digits, digits)) {
        auto const digit = _color_impl::hex_digit(c);
        if(digit < 0)
          return std::nullopt;
        value = value * 16 + digit;
      }
      components.at(index) = static_cast<u8>(short_form ? value * 17 : value);
    }
    auto const [r, g, b, a] = components;
    return Rgba8(r, g, b, a);
  }

  /**
   * @brief Returns the color `0xRRGGBBAA`.
   */
  [[nodiscard]] static constexpr Rgba8 from_rgba32(u32 const rgba) noexcept
  {
    return {
      static_cast<u8>(rgba >> 24U),
      static_cast<u8>(rgba >> 16U),
      static_cast<u8>(rgba >> 8U),
      static_cast<u8>(rgba),
    };
  }

  /**
   * @brief Returns the color of @p hsv, rounded to 8 bits.
   */
  [[nodiscard]] static Rgba8 from_hsv(Hsv const& hsv) noexcept
  {
    auto const [r, g, b] = _color_impl::from_hsv(hsv);
    return {
      _color_impl::to_byte(r),
      _color_impl::to_byte(g),
      _color_impl::to_byte(b),
      _color_impl::to_byte(hsv.alpha),
    };
  }

  [[nodiscard]] constexpr bool operator==(Rgba8 const&) const noexcept = default;

 private:
  u8 m_r = 0;
  u8 m_g = 0;
  u8 m_b = 0;
  u8 m_a = 0;
};

inline Rgba8 Rgba::to_srgb8() const noexcept
{
  return {
    _color_impl::to_byte(linear_to_srgb(this->m_r)),
    _color_impl::to_byte(linear_to_srgb(this->m_g)),
    _color_impl::to_byte(linear_to_srgb(this->m_b)),
    _color_impl::to_byte(this->m_a),
  };
}

inline Rgba Rgba::from_srgb8(Rgba8 const& color) noexcept { return color.to_linear(); }

inline std::optional<Rgba> Rgba::from_hex(std::string_view const text) noexcept
{
  return Rgba8::from_hex(text).transform(&Rgba8::to_linear);
}

inline namespace literals
{
  /**
   * @brief Creates an sRGB color from a hex literal, e.g. `"#e4a672"_rgba8`, checked at compile
   * time.
   * @ingroup core_math
   * @relates Rgba8
   * @see Rgba8::from_hex
   */
  [[nodiscard]] consteval Rgba8 operator""_rgba8(char const* const text, usize const size)
  {
    auto const color = Rgba8::from_hex({text, size});
    if(not color.has_value())
      throw std::invalid_argument("a color literal is #rgb, #rgba, #rrggbb or #rrggbbaa");
    return *color;
  }
}  // namespace literals

W_COMMON_API std::ostream& operator<<(std::ostream& os, Rgba const& color);

W_COMMON_API std::ostream& operator<<(std::ostream& os, Rgba8 const& color);

static_assert(std::is_standard_layout_v<Rgba>);
static_assert(std::is_trivially_copyable_v<Rgba>);
static_assert(sizeof(Rgba) == 16);
static_assert(std::is_standard_layout_v<Rgba8>);
static_assert(std::is_trivially_copyable_v<Rgba8>);
static_assert(sizeof(Rgba8) == 4);
static_assert(
  std::bit_cast<std::array<u8, 4>>(Rgba8(1, 2, 3, 4)) == std::array<u8, 4> {1, 2, 3, 4},
  "Rgba8 must store r, g, b, a bytes in order"
);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Rgba>
{
  [[nodiscard]] std::size_t operator()(w::common::Rgba const& color) const noexcept
  {
    return w::common::_hash_impl::combine(color.r(), color.g(), color.b(), color.a());
  }
};

template <>
struct std::hash<w::common::Rgba8>
{
  [[nodiscard]] std::size_t operator()(w::common::Rgba8 const& color) const noexcept
  {
    return std::hash<std::uint32_t> {}(color.to_rgba32());
  }
};

template <>
struct std::hash<w::common::Hsv>
{
  [[nodiscard]] std::size_t operator()(w::common::Hsv const& hsv) const noexcept
  {
    return w::common::_hash_impl::combine(hsv.hue, hsv.saturation, hsv.value, hsv.alpha);
  }
};

#ifndef __DOXYGEN__
/**
 * @brief Formats as `(r, g, b, a)`; the format spec, e.g. `{:.3f}`, applies to every component.
 */
template <>
struct fmt::formatter<w::common::Rgba> : formatter<float>
{
  format_context::iterator format(w::common::Rgba const& color, format_context& ctx) const
  {
    auto out = ctx.out();
    *out++ = '(';
    auto first = true;
    for(auto const value : color.to_array()) {
      if(not first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
      ctx.advance_to(out);
      out = formatter<float>::format(value, ctx);
    }
    *out++ = ')';
    return out;
  }
};

/**
 * @brief Formats as the hex string of Rgba8::to_hex(); the format spec applies to the string.
 */
template <>
struct fmt::formatter<w::common::Rgba8> : formatter<std::string_view>
{
  format_context::iterator format(w::common::Rgba8 const& color, format_context& ctx) const
  {
    return formatter<std::string_view>::format(color.to_hex(), ctx);
  }
};

/**
 * @brief Formats as `hsv(hue°, saturation, value, alpha)`.
 */
template <>
struct fmt::formatter<w::common::Hsv> : formatter<float>
{
  static format_context::iterator format(w::common::Hsv const& hsv, format_context const& ctx)
  {
    return format_to(
      ctx.out(),
      "hsv({}°, {}, {}, {})",
      hsv.hue.degrees(),
      hsv.saturation,
      hsv.value,
      hsv.alpha
    );
  }
};
#endif
