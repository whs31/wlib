#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <array>
#include <string>
#include <variant>
#include <vector>

#include <w/common/color.h>
#include <w/common/glaze/angle.h>
#include <w/common/glaze/detail.h>

W_COMMON_NAMESPACE_BEGIN

namespace _glaze_impl  // NOLINT(readability-identifier-naming)
{
  /**
   * @brief What a color is read from: in a text format, the form the adapter writes or the other
   * one, by the first character of the value; in a binary format, the written form only.
   */
  template <typename Written, typename Other>
  class ColorInput
  {
   public:
    using Value = std::variant<Written, Other>;

    // note: [whs31 : 2026-10-26]
    // not an aggregate, so that glaze reads it through the specialization below rather than by
    // reflection
    [[nodiscard]] Value& value() noexcept { return this->m_value; }

    [[nodiscard]] Value const& value() const noexcept { return this->m_value; }

   private:
    Value m_value;
  };

  using Rgba8Input = ColorInput<std::string, std::vector<std::int64_t>>;
  using RgbaInput = ColorInput<std::vector<float>, std::string>;

  inline void read_rgba8(Rgba8& color, Rgba8Input const& input, glz::context& ctx) noexcept
  {
    if(auto const* const hex = std::get_if<std::string>(&input.value())) {
      if(auto const parsed = Rgba8::from_hex(*hex))
        color = *parsed;
      else
        reject(ctx, "a color string is #rgb, #rgba, #rrggbb or #rrggbbaa");
      return;
    }
    auto const& bytes = std::get<std::vector<std::int64_t>>(input.value());
    auto const in_range = std::ranges::all_of(bytes, [](std::int64_t const byte) {
      return byte >= 0 and byte <= 255;
    });
    if((bytes.size() != 3 and bytes.size() != 4) or not in_range) {
      reject(ctx, "an 8-bit color array is [r, g, b] or [r, g, b, a], each in [0, 255]");
      return;
    }
    auto const byte = [&bytes](std::size_t const index) {
      return index < bytes.size() ? static_cast<std::uint8_t>(bytes.at(index)) : std::uint8_t {255};
    };
    color = Rgba8(byte(0), byte(1), byte(2), byte(3));
  }

  inline void read_rgba(Rgba& color, RgbaInput const& input, glz::context& ctx) noexcept
  {
    if(auto const* const hex = std::get_if<std::string>(&input.value())) {
      if(auto const parsed = Rgba::from_hex(*hex))
        color = *parsed;
      else
        reject(ctx, "a color string is #rgb, #rgba, #rrggbb or #rrggbbaa");
      return;
    }
    auto const& components = std::get<std::vector<float>>(input.value());
    if(components.size() != 3 and components.size() != 4) {
      reject(ctx, "a linear color array is [r, g, b] or [r, g, b, a]");
      return;
    }
    color = Rgba(
      components.at(0),
      components.at(1),
      components.at(2),
      components.size() == 4 ? components.at(3) : 1.0F
    );
  }
}  // namespace _glaze_impl

W_COMMON_NAMESPACE_END

/**
 * @brief Reads a ColorInput: a text format takes either form, by the first character of the
 * value; any other format takes the written form only, so a binary document reads back what the
 * adapter writes.
 */
template <std::uint32_t Format, typename Written, typename Other>
struct glz::from<Format, w::common::_glaze_impl::ColorInput<Written, Other>>
{
  template <auto Opts>
  static void op(auto&& input, is_context auto&& ctx, auto&& it, auto end)
  {
    if constexpr(Format == glz::JSON or Format == glz::TOML)
      glz::from<Format, std::variant<Written, Other>>::template op<
        Opts>(input.value(), ctx, it, end);
    else
      glz::from<Format, Written>::template op<
        Opts>(input.value().template emplace<Written>(), ctx, it, end);
  }
};

/**
 * @brief Glaze metadata of Rgba8: the hex string of Rgba8::to_hex(), e.g. `"#e4a672"`.
 * @details A text document may also give the bytes, `[228, 166, 114]` or `[228, 166, 114, 255]`.
 * A palette then lives in TOML as `sand = "#e4a672"`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Rgba8>
{
  static auto constexpr value = glz::custom<
    [](
      w::common::Rgba8& color,
      w::common::_glaze_impl::Rgba8Input const& input,
      glz::context& ctx
    ) { w::common::_glaze_impl::read_rgba8(color, input, ctx); },
    [](w::common::Rgba8 const& color) { return color.to_hex(); }>;
};

/**
 * @brief Glaze metadata of Rgba: an array of the linear components `[r, g, b, a]`.
 * @details Reading also takes `[r, g, b]`, opaque, and, in a text format, an sRGB hex string
 * such as `"#e4a672"`, which is decoded to linear light.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Rgba>
{
  static auto constexpr value = glz::custom<
    [](w::common::Rgba& color, w::common::_glaze_impl::RgbaInput const& input, glz::context& ctx) {
      w::common::_glaze_impl::read_rgba(color, input, ctx);
    },
    [](w::common::Rgba const& color) { return color.to_array(); }>;
};

/**
 * @brief Glaze metadata of Hsv: `{"hue": angle, "saturation": s, "value": v, "alpha": a}`, the hue
 * read as any Angle, e.g. `"30deg"`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Hsv>
{
  using T = w::common::Hsv;
  static auto constexpr value = glz::
    object("hue", &T::hue, "saturation", &T::saturation, "value", &T::value, "alpha", &T::alpha);
};
