#include <private/sexagesimal.h>

#include <cmath>
#include <cstdint>

#include <algorithm>
#include <array>
#include <charconv>
#include <memory>
#include <span>
#include <system_error>

#include <fmt/format.h>

#include <w/common/detail/geographic.h>

W_COMMON_NAMESPACE_BEGIN

namespace _sexagesimal_impl
{
  namespace
  {
    auto constexpr whitespace = std::string_view(" \t\r\n");

    // longest first, so that a separator never stops at the prefix of a longer one
    auto constexpr degree_separators = std::array<std::string_view, 5> {
      "\xc2\xb0",  // degree sign
      "\xc2\xba",  // masculine ordinal indicator, often typed for the degree sign
      "deg",
      "d",
      ":",
    };
    auto constexpr minute_separators = std::array<std::string_view, 5> {
      "\xe2\x80\xb2",  // prime
      "\xe2\x80\x99",  // right single quotation mark, as word processors replace the apostrophe
      "'",
      "m",
      ":",
    };
    auto constexpr second_separators = std::array<std::string_view, 5> {
      "\xe2\x80\xb3",  // double prime
      "\xe2\x80\x9d",  // right double quotation mark
      "''",
      "\"",
      "s",
    };

    // seconds in units of their last decimal stay exact in an int64 up to 9.2e18
    auto constexpr max_dms_units = 9e18;

    void skip_whitespace(std::string_view& text) noexcept
    {
      auto const begin = text.find_first_not_of(whitespace);
      text.remove_prefix(begin == std::string_view::npos ? text.size() : begin);
    }

    [[nodiscard]] bool is_letter(std::string_view const token) noexcept
    {
      return token.size() == 1
         and ((token.front() >= 'a' and token.front() <= 'z')
              or (token.front() >= 'A' and token.front() <= 'Z'));
    }

    // the first of the tokens text starts with; a single letter only if it touches the number
    [[nodiscard]] bool consume_separator(
      std::string_view& text,
      std::span<std::string_view const> const tokens,
      bool const after_space
    ) noexcept
    {
      auto const token = std::ranges::find_if(tokens, [&text, after_space](auto const candidate) {
        return text.starts_with(candidate) and not(after_space and is_letter(candidate));
      });
      if(token == tokens.end())
        return false;
      text.remove_prefix(token->size());
      return true;
    }

    struct HemisphereName
    {
      std::string_view name;
      char hemisphere;
    };

    // words first, so that "south" is not read as "s" followed by "outh"
    auto constexpr hemisphere_names = std::array<HemisphereName, 8> {{
      {.name = "north", .hemisphere = 'N'},
      {.name = "south", .hemisphere = 'S'},
      {.name = "east", .hemisphere = 'E'},
      {.name = "west", .hemisphere = 'W'},
      {.name = "n", .hemisphere = 'N'},
      {.name = "s", .hemisphere = 'S'},
      {.name = "e", .hemisphere = 'E'},
      {.name = "w", .hemisphere = 'W'},
    }};

    [[nodiscard]] char lowercase(char const character) noexcept
    {
      return character >= 'A' and character <= 'Z' ? static_cast<char>(character - 'A' + 'a')
                                                   : character;
    }

    [[nodiscard]] bool starts_with_ignoring_case(
      std::string_view const text,
      std::string_view const prefix
    ) noexcept
    {
      return text.size() >= prefix.size()
         and std::ranges::equal(text.substr(0, prefix.size()), prefix, {}, lowercase);
    }

    [[nodiscard]] bool starts_with_hemisphere_word(std::string_view const text) noexcept
    {
      return std::ranges::any_of(hemisphere_names, [text](auto const& name) {
        return name.name.size() > 1 and starts_with_ignoring_case(text, name.name);
      });
    }

    // the hemisphere text starts with, a letter or a word in either case, or '\0'
    [[nodiscard]] char consume_hemisphere(std::string_view& text) noexcept
    {
      for(auto const& candidate : hemisphere_names)
        if(starts_with_ignoring_case(text, candidate.name)) {
          text.remove_prefix(candidate.name.size());
          return candidate.hemisphere;
        }
      return '\0';
    }

    // a nonnegative number in fixed notation: no sign, no exponent, which would swallow an E
    [[nodiscard]] std::optional<double> consume_number(std::string_view& text) noexcept
    {
      if(text.empty() or ((text.front() < '0' or text.front() > '9') and text.front() != '.'))
        return std::nullopt;
      auto value = 0.0;
      auto const* const last = std::to_address(text.end());
      auto const [end, error] = std::
        from_chars(std::to_address(text.begin()), last, value, std::chars_format::fixed);
      if(error != std::errc {} or not std::isfinite(value))
        return std::nullopt;
      text = std::string_view(end, last);
      return value;
    }

    // a number and its separator; another component may follow a separator or whitespace
    [[nodiscard]] std::optional<double> consume_component(
      std::string_view& text,
      std::span<std::string_view const> const separators,
      bool& more
    ) noexcept
    {
      auto const value = consume_number(text);
      if(not value.has_value()) {
        more = false;
        return std::nullopt;
      }
      auto const length = text.size();
      // "15.6south" is south rather than seconds followed by "outh"
      auto separated = not starts_with_hemisphere_word(text)
                   and consume_separator(text, separators, false);
      if(not separated) {
        // "15.6 s" is south rather than seconds, but "59.771 deg" and "59 °" are degrees
        auto spaced = text;
        skip_whitespace(spaced);
        separated = spaced.size() != text.size() and consume_separator(spaced, separators, true);
        if(separated)
          text = spaced;
      }
      skip_whitespace(text);
      more = separated or text.size() != length;
      return value;
    }

    [[nodiscard]] bool is_whole(double const value) noexcept { return value == std::floor(value); }
  }  // namespace

  std::optional<ParsedCoordinate> parse(std::string_view text) noexcept
  {
    skip_whitespace(text);
    auto hemisphere = consume_hemisphere(text);
    skip_whitespace(text);
    auto const has_sign = text.starts_with('-') or text.starts_with('+');
    auto const negative = text.starts_with('-');
    if(has_sign) {
      if(hemisphere != '\0')
        return std::nullopt;
      text.remove_prefix(1);
    }

    auto more = true;
    auto const degrees = consume_component(text, degree_separators, more);
    if(not degrees.has_value())
      return std::nullopt;
    auto const minutes = more ? consume_component(text, minute_separators, more) : std::nullopt;
    auto const seconds = minutes.has_value() and more
                         ? consume_component(text, second_separators, more)
                         : std::nullopt;
    if(minutes.has_value() and (not is_whole(*degrees) or *minutes >= 60.0))
      return std::nullopt;
    if(seconds.has_value() and (not is_whole(*minutes) or *seconds >= 60.0))
      return std::nullopt;

    skip_whitespace(text);
    if(hemisphere == '\0' and not has_sign) {
      hemisphere = consume_hemisphere(text);
      skip_whitespace(text);
    }
    if(not text.empty())
      return std::nullopt;

    auto const magnitude = *degrees + minutes.value_or(0.0) / 60.0
                         + seconds.value_or(0.0) / 3'600.0;
    auto const south_or_west = negative or hemisphere == 'S' or hemisphere == 'W';
    return ParsedCoordinate {
      .degrees = south_or_west ? -magnitude : magnitude,
      .hemisphere = hemisphere,
    };
  }

  std::string format_decimal(double const degrees, char const hemisphere)
  {
    if(not std::isfinite(degrees))
      return fmt::format("{}{}", degrees, _geographic_impl::degree_sign);
    if(hemisphere == '\0')
      return fmt::format("{}{}", std::abs(degrees), _geographic_impl::degree_sign);
    return fmt::format("{}{}{}", std::abs(degrees), _geographic_impl::degree_sign, hemisphere);
  }

  std::string format_dms(
    double const degrees,
    int const second_decimals,
    char const positive,
    char const negative,
    double const no_hemisphere_at
  )
  {
    auto const decimals = std::clamp(second_decimals, 0, 9);
    auto scale = std::int64_t {1};
    for(auto decimal = 0; decimal < decimals; ++decimal)
      scale *= 10;
    auto const per_second = static_cast<double>(scale);
    auto const magnitude = std::abs(degrees);
    if(not(magnitude * 3'600.0 * per_second < max_dms_units))
      return format_decimal(degrees, '\0');

    // rounded once, in whole units of the last decimal, so that 59.9999" carries into a minute
    auto const units = std::llround(magnitude * 3'600.0 * per_second);
    auto const per_degree = 3'600 * scale;
    auto const per_minute = 60 * scale;
    auto const second_units = units % per_minute;
    auto const no_hemisphere = units == 0
                            or units == std::llround(no_hemisphere_at * 3'600.0 * per_second);
    auto result = fmt::format(
      "{}{}{:02}'{:02}",
      units / per_degree,
      _geographic_impl::degree_sign,
      units % per_degree / per_minute,
      second_units / scale
    );
    if(decimals > 0)
      result += fmt::format(".{:0{}}", second_units % scale, decimals);
    result += '"';
    if(not no_hemisphere)
      result += degrees < 0.0 ? negative : positive;
    return result;
  }
}  // namespace _sexagesimal_impl

W_COMMON_NAMESPACE_END
