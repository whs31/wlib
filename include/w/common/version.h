#pragma once

#include <compare>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <w/common/stdint.h>
#include <w/common/config.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/error.h>

W_COMMON_NAMESPACE_BEGIN

class W_COMMON_API VersionError final : public Error
{
 public:
  template <typename... Args>
  explicit VersionError(fmt::format_string<Args...> fmt, Args&&... args)
    : Error(fmt, std::forward<Args>(args)...)
  {}

  [[nodiscard]] static VersionError invalid_string(std::string_view text, std::string_view reason);
  [[nodiscard]] static VersionError invalid_prerelease_identifier(std::string_view text);
  [[nodiscard]] static VersionError invalid_build_metadata(std::string_view text);
  [[nodiscard]] static VersionError invalid_range_string(
    std::string_view text,
    std::string_view reason
  );
};

class W_COMMON_API Prerelease
{
 public:
  static Prerelease const EMPTY;

  Prerelease() noexcept = default;
  explicit Prerelease(std::string_view text);

  [[nodiscard]] std::string const& as_str() const noexcept { return this->m_value; }

  [[nodiscard]] bool is_empty() const noexcept { return this->m_value.empty(); }

  [[nodiscard]] bool operator==(Prerelease const&) const noexcept = default;
  [[nodiscard]] std::strong_ordering operator<=>(Prerelease const& other) const noexcept;

 private:
  std::string m_value;
};

class W_COMMON_API BuildMetadata
{
 public:
  static BuildMetadata const EMPTY;

  BuildMetadata() noexcept = default;
  explicit BuildMetadata(std::string_view text);

  [[nodiscard]] std::string const& as_str() const noexcept { return this->m_value; }

  [[nodiscard]] bool is_empty() const noexcept { return this->m_value.empty(); }

  [[nodiscard]] bool operator==(BuildMetadata const&) const noexcept = default;
  [[nodiscard]] std::strong_ordering operator<=>(BuildMetadata const& other) const noexcept;

 private:
  std::string m_value;
};

/**
 * @brief Semantic version value type.
 * @ingroup core_identifiers
 */
class W_COMMON_API Version
{
 public:
  Version() noexcept = default;

  constexpr Version(u64 const major, u64 const minor, u64 const patch) noexcept
    : m_major(major)
    , m_minor(minor)
    , m_patch(patch)
  {}

  Version(
    u64 major,
    u64 minor,
    u64 patch,
    Prerelease pre,
    BuildMetadata build = BuildMetadata()
  ) noexcept;

  explicit Version(std::string_view text);

  [[nodiscard]] constexpr u64 major() const noexcept { return this->m_major; }

  [[nodiscard]] constexpr u64 minor() const noexcept { return this->m_minor; }

  [[nodiscard]] constexpr u64 patch() const noexcept { return this->m_patch; }

  [[nodiscard]] Prerelease const& pre() const noexcept { return this->m_pre; }

  [[nodiscard]] BuildMetadata const& build() const noexcept { return this->m_build; }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] static constexpr Version new_version(
    u64 const major,
    u64 const minor,
    u64 const patch
  ) noexcept
  {
    return {major, minor, patch};
  }

  [[nodiscard]] static Version parse(std::string_view text);

  [[nodiscard]] std::strong_ordering cmp_precedence(Version const& other) const noexcept;
  [[nodiscard]] bool operator==(Version const&) const noexcept = default;
  [[nodiscard]] std::strong_ordering operator<=>(Version const& other) const noexcept;

 private:
  u64 m_major = 0;
  u64 m_minor = 0;
  u64 m_patch = 0;
  Prerelease m_pre;
  BuildMetadata m_build;
};

inline namespace literals
{
  /**
   * @brief Creates a version from a string literal, e.g. `"1.2.3-rc.1"_version`.
   * @throws VersionError if the literal is not a semantic version.
   * @ingroup core_identifiers
   * @relates Version
   */
  [[nodiscard]] W_COMMON_API Version operator""_version(char const* text, usize length);
}  // namespace literals

W_COMMON_API std::ostream& operator<<(std::ostream& stream, Prerelease const& value);
W_COMMON_API std::ostream& operator<<(std::ostream& stream, BuildMetadata const& value);
W_COMMON_API std::ostream& operator<<(std::ostream& stream, Version const& value);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Prerelease>
{
  [[nodiscard]] std::size_t operator()(w::common::Prerelease const& value) const noexcept
  {
    return std::hash<std::string> {}(value.as_str());
  }
};

template <>
struct std::hash<w::common::BuildMetadata>
{
  [[nodiscard]] std::size_t operator()(w::common::BuildMetadata const& value) const noexcept
  {
    return std::hash<std::string> {}(value.as_str());
  }
};

template <>
struct std::hash<w::common::Version>
{
  [[nodiscard]] std::size_t operator()(w::common::Version const& version) const noexcept
  {
    auto hash = std::hash<w::u64> {}(version.major());
    auto const combine = [&hash](w::usize const value) {
      hash ^= value + 0x9e3779b97f4a7c15ull + (hash << 6U) + (hash >> 2U);
    };
    combine(std::hash<w::u64> {}(version.minor()));
    combine(std::hash<w::u64> {}(version.patch()));
    combine(std::hash<w::common::Prerelease> {}(version.pre()));
    combine(std::hash<w::common::BuildMetadata> {}(version.build()));
    return hash;
  }
};

#ifndef __DOXYGEN__
template <>
struct fmt::formatter<w::common::Prerelease> : ostream_formatter
{};

template <>
struct fmt::formatter<w::common::BuildMetadata> : ostream_formatter
{};

template <>
struct fmt::formatter<w::common::Version> : ostream_formatter
{};
#endif
