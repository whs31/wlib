#pragma once

#include <functional>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/ostream.h>

#include <w/common/stdint.h>
#include <w/common/config.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/version.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Semantic version requirement expression.
 * @ingroup core_identifiers
 */
class W_COMMON_API VersionRange
{
 public:
  VersionRange();
  explicit VersionRange(std::string_view expression);

  [[nodiscard]] bool contains(Version const& version) const noexcept;

  [[nodiscard]] bool satisfies(Version const& version) const noexcept
  {
    return this->contains(version);
  }

  [[nodiscard]] bool is_any() const noexcept;
  [[nodiscard]] bool is_any_with_prerelease() const noexcept;

  [[nodiscard]] std::string const& to_string() const noexcept { return this->m_expression; }

  [[nodiscard]] static VersionRange any();
  [[nodiscard]] static VersionRange any_with_prerelease();
  [[nodiscard]] static VersionRange parse(std::string_view expression);

  [[nodiscard]] bool operator==(VersionRange const&) const noexcept = default;

  enum class Comparison : u8
  {
    Equal,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
  };

  struct Comparator
  {
    Comparison comparison = Comparison::Equal;
    Version version;
    [[nodiscard]] bool operator==(Comparator const&) const noexcept = default;
  };

  struct Alternative
  {
    std::vector<Comparator> comparators;
    [[nodiscard]] bool operator==(Alternative const&) const noexcept = default;
  };

 private:
  std::string m_expression = "*";
  std::vector<Alternative> m_alternatives;
  bool m_include_prerelease = false;
};

W_COMMON_API std::ostream& operator<<(std::ostream& stream, VersionRange const& range);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::VersionRange>
{
  [[nodiscard]] std::size_t operator()(w::common::VersionRange const& range) const noexcept
  {
    auto const expression = std::hash<std::string> {}(range.to_string());
    auto const prerelease = std::hash<bool> {}(range.is_any_with_prerelease());
    return expression ^ (prerelease + 0x9e3779b9U + (expression << 6U) + (expression >> 2U));
  }
};

#ifndef __DOXYGEN__
template <>
struct fmt::formatter<w::common::VersionRange> : ostream_formatter
{};
#endif
