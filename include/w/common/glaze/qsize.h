#pragma once

#include <w/common/config.h>

#ifndef W_COMMON_WITH_QT
#  error "Qt glaze adapters require wlib to be built with with_qt=True"
#endif

#include <array>

#include <qsize.h>

#include <w/common/glaze/detail.h>

/**
 * @brief Glaze metadata of `QSize`: a two-element array `[width, height]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<QSize>
{
  static auto constexpr value = glz::custom<
    [](QSize& value, std::array<int, 2> const& pair) {
      auto const [first, second] = pair;
      value = QSize(first, second);
    },
    [](QSize const& value) { return std::array<int, 2> {value.width(), value.height()}; }>;
};

/**
 * @brief Glaze metadata of `QSizeF`: a two-element array `[width, height]`.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<QSizeF>
{
  static auto constexpr value = glz::custom<
    [](QSizeF& value, std::array<double, 2> const& pair) {
      auto const [first, second] = pair;
      value = QSizeF(first, second);
    },
    [](QSizeF const& value) { return std::array<double, 2> {value.width(), value.height()}; }>;
};
