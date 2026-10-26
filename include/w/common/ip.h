#pragma once

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <w/common/config.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/error.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Error raised while parsing or converting an IP or socket address.
 * @ingroup core_errors
 */
class W_COMMON_API IpError final : public Error
{
 public:
  template <typename... Args>
  explicit IpError(fmt::format_string<Args...> fmt, Args&&... args)
    : Error(fmt, std::forward<Args>(args)...)
  {}

  [[nodiscard]] static IpError invalid_v4_string(std::string_view text);
  [[nodiscard]] static IpError invalid_v6_string(std::string_view text);
  [[nodiscard]] static IpError missing_colon(std::string_view text);
  [[nodiscard]] static IpError invalid_port(std::string_view text);
  [[nodiscard]] static IpError not_a_v4();
  [[nodiscard]] static IpError not_a_v6();
};

/**
 * @brief IPv4 address stored as a big-endian/network-order integer.
 * @ingroup core_networking
 */
class W_COMMON_API Ipv4Addr
{
 public:
  static Ipv4Addr const LOCALHOST;
  static Ipv4Addr const UNSPECIFIED;
  static Ipv4Addr const BROADCAST;

  constexpr Ipv4Addr() noexcept = default;

  explicit constexpr Ipv4Addr(std::uint32_t const address) noexcept
    : m_address(address)
  {}

  [[nodiscard]] static constexpr Ipv4Addr from_octets(
    std::uint8_t const a,
    std::uint8_t const b,
    std::uint8_t const c,
    std::uint8_t const d
  ) noexcept
  {
    return Ipv4Addr(
      static_cast<std::uint32_t>(a) << 24 | static_cast<std::uint32_t>(b) << 16
      | static_cast<std::uint32_t>(c) << 8 | static_cast<std::uint32_t>(d)
    );
  }

  [[nodiscard]] static Ipv4Addr parse(std::string_view text);

  [[nodiscard]] constexpr std::uint32_t address() const noexcept { return this->m_address; }

  [[nodiscard]] constexpr std::array<std::uint8_t, 4> octets() const noexcept
  {
    return {
      static_cast<std::uint8_t>((this->m_address >> 24) & 0xffU),
      static_cast<std::uint8_t>((this->m_address >> 16) & 0xffU),
      static_cast<std::uint8_t>((this->m_address >> 8) & 0xffU),
      static_cast<std::uint8_t>(this->m_address & 0xffU),
    };
  }

  [[nodiscard]] constexpr bool is_unspecified() const noexcept { return this->m_address == 0; }

  [[nodiscard]] constexpr bool is_loopback() const noexcept
  {
    return (this->m_address & 0xff000000U) == 0x7f000000U;
  }

  [[nodiscard]] constexpr bool is_private() const noexcept
  {
    return (this->m_address & 0xff000000U) == 0x0a000000U
        or (this->m_address & 0xfff00000U) == 0xac100000U
        or (this->m_address & 0xffff0000U) == 0xc0a80000U;
  }

  [[nodiscard]] constexpr bool is_multicast() const noexcept
  {
    return (this->m_address & 0xf0000000U) == 0xe0000000U;
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] constexpr bool operator==(Ipv4Addr const&) const noexcept = default;
  [[nodiscard]] constexpr std::strong_ordering operator<=>(Ipv4Addr const&) const noexcept =
    default;

 private:
  std::uint32_t m_address = 0;
};

/**
 * @brief IPv6 address stored as sixteen network-order bytes.
 * @ingroup core_networking
 */
class W_COMMON_API Ipv6Addr
{
 public:
  static Ipv6Addr const LOCALHOST;
  static Ipv6Addr const UNSPECIFIED;

  constexpr Ipv6Addr() noexcept = default;

  explicit constexpr Ipv6Addr(std::array<std::uint8_t, 16> const& octets) noexcept
    : m_octets(octets)
  {}

  [[nodiscard]] static constexpr Ipv6Addr from_segments(
    std::uint16_t const a,
    std::uint16_t const b,
    std::uint16_t const c,
    std::uint16_t const d,
    std::uint16_t const e,
    std::uint16_t const f,
    std::uint16_t const g,
    std::uint16_t const h
  ) noexcept
  {
    return Ipv6Addr({
      static_cast<std::uint8_t>(a >> 8),
      static_cast<std::uint8_t>(a & 0xffU),
      static_cast<std::uint8_t>(b >> 8),
      static_cast<std::uint8_t>(b & 0xffU),
      static_cast<std::uint8_t>(c >> 8),
      static_cast<std::uint8_t>(c & 0xffU),
      static_cast<std::uint8_t>(d >> 8),
      static_cast<std::uint8_t>(d & 0xffU),
      static_cast<std::uint8_t>(e >> 8),
      static_cast<std::uint8_t>(e & 0xffU),
      static_cast<std::uint8_t>(f >> 8),
      static_cast<std::uint8_t>(f & 0xffU),
      static_cast<std::uint8_t>(g >> 8),
      static_cast<std::uint8_t>(g & 0xffU),
      static_cast<std::uint8_t>(h >> 8),
      static_cast<std::uint8_t>(h & 0xffU),
    });
  }

  [[nodiscard]] static Ipv6Addr parse(std::string_view text);

  [[nodiscard]] constexpr std::array<std::uint8_t, 16> const& octets() const noexcept
  {
    return this->m_octets;
  }

  [[nodiscard]] constexpr std::array<std::uint16_t, 8> segments() const noexcept
  {
    auto result = std::array<std::uint16_t, 8> {};
    for(auto i = std::size_t {0}; i < result.size(); ++i) {
      result.at(i) = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(this->m_octets.at(i * 2)) << 8
        | static_cast<std::uint16_t>(this->m_octets.at(i * 2 + 1))
      );
    }
    return result;
  }

  [[nodiscard]] constexpr bool is_unspecified() const noexcept
  {
    return std::ranges::all_of(this->m_octets, [](auto const byte) { return byte == 0; });
  }

  [[nodiscard]] constexpr bool is_loopback() const noexcept
  {
    auto constexpr localhost = std::array<std::uint8_t, 16> {
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      1,
    };
    return this->m_octets == localhost;
  }

  [[nodiscard]] constexpr bool is_multicast() const noexcept
  {
    return this->m_octets.front() == 0xff;
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] constexpr bool operator==(Ipv6Addr const&) const noexcept = default;
  [[nodiscard]] constexpr std::strong_ordering operator<=>(Ipv6Addr const&) const noexcept =
    default;

 private:
  std::array<std::uint8_t, 16> m_octets {};
};

W_COMMON_API std::ostream& operator<<(std::ostream& stream, Ipv4Addr const& address);
W_COMMON_API std::ostream& operator<<(std::ostream& stream, Ipv6Addr const& address);

[[nodiscard]] inline Ipv4Addr operator""_ipv4(char const* text, std::size_t const length)
{
  return Ipv4Addr::parse(std::string_view {text, length});
}

[[nodiscard]] inline Ipv6Addr operator""_ipv6(char const* text, std::size_t const length)
{
  return Ipv6Addr::parse(std::string_view {text, length});
}

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Ipv4Addr>
{
  [[nodiscard]] std::size_t operator()(w::common::Ipv4Addr const& address) const noexcept
  {
    return std::hash<std::uint32_t> {}(address.address());
  }
};

template <>
struct std::hash<w::common::Ipv6Addr>
{
  [[nodiscard]] std::size_t operator()(w::common::Ipv6Addr const& address) const noexcept
  {
    auto hash = std::size_t {};
    for(auto const byte : address.octets()) {
      hash ^= std::hash<std::uint8_t> {}(byte) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    }
    return hash;
  }
};

#ifndef __DOXYGEN__
template <>
struct fmt::formatter<w::common::Ipv4Addr> : fmt::ostream_formatter
{};

template <>
struct fmt::formatter<w::common::Ipv6Addr> : fmt::ostream_formatter
{};
#endif
