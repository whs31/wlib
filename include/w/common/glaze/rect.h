#pragma once

#include <array>
#include <cstdint>

#include <w/common/glaze/detail.h>
#include <w/common/rect.h>

/**
 * @brief Glaze metadata of Rect: an array `[x, y, width, height]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Rect>
{
  static auto constexpr value = glz::custom<
    [](w::common::Rect& rect, w::common::_glaze_impl::ExactArray<w::common::Real, 4> const& xywh) {
      rect = w::common::Rect(xywh[0], xywh[1], xywh[2], xywh[3]);
    },
    [](w::common::Rect const& rect) { return rect.to_array(); }>;
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
