#pragma once

#include <concepts>
#include <array>
#include <cstdint>

#include <w/common/glaze/detail.h>
#include <w/common/rect.h>

/**
 * @brief Glaze metadata of Rect: an array `[x, y, width, height]`.
 * @ingroup core_glaze
 */
template <std::floating_point T>
struct glz::meta<w::common::BasicRect<T>>
{
  static auto constexpr value = glz::custom<
    [](w::common::BasicRect<T>& rect, w::common::_glaze_impl::ExactArray<T, 4> const& xywh) {
      rect = w::common::BasicRect<T>(xywh[0], xywh[1], xywh[2], xywh[3]);
    },
    [](w::common::BasicRect<T> const& rect) { return rect.to_array(); }>;
};

/**
 * @brief Glaze metadata of Recti: an array `[x, y, width, height]` of integers.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Recti>
{
  static auto constexpr value = glz::custom<
    [](w::common::Recti& rect, w::common::_glaze_impl::ExactArray<std::int32_t, 4> const& xywh) {
      rect = w::common::Recti(xywh[0], xywh[1], xywh[2], xywh[3]);
    },
    [](w::common::Recti const& rect) { return rect.to_array(); }>;
};
