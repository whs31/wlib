#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

namespace _sexagesimal_impl  // NOLINT(readability-identifier-naming)
{
  /// @brief A coordinate read from text: signed degrees and the hemisphere letter it named.
  struct ParsedCoordinate
  {
    double degrees = 0.0;
    /// @brief `'N'`, `'S'`, `'E'` or `'W'`, or `'\0'` if the text gave a sign or nothing.
    char hemisphere = '\0';
  };

  /**
   * @brief Reads decimal degrees, degrees and decimal minutes, or degrees, minutes and seconds,
   * signed or with a hemisphere letter, as Latitude::parse() documents.
   */
  [[nodiscard]] std::optional<ParsedCoordinate> parse(std::string_view text) noexcept;

  /**
   * @brief Formats `|degrees|` in shortest decimal form with the degree sign, followed by
   * @p hemisphere unless it is `'\0'`.
   */
  [[nodiscard]] std::string format_decimal(double degrees, char hemisphere);

  /**
   * @brief Formats `|degrees|` as degrees, minutes and seconds rounded to @p second_decimals,
   * followed by @p positive or @p negative as the sign of @p degrees chooses, or by no hemisphere
   * if the rounded value is zero or @p no_hemisphere_at.
   */
  [[nodiscard]] std::string format_dms(
    double degrees,
    int second_decimals,
    char positive,
    char negative,
    double no_hemisphere_at
  );
}  // namespace _sexagesimal_impl

W_COMMON_NAMESPACE_END
