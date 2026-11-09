#pragma once

#include <algorithm>
#include <array>
#include <compare>
#include <functional>
#include <iosfwd>
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

  explicit constexpr Ipv4Addr(u32 const address) noexcept
    : m_address(address)
  {}

  [[nodiscard]] static constexpr Ipv4Addr from_octets(
    u8 const a,
    u8 const b,
    u8 const c,
    u8 const d
  ) noexcept
  {
    return Ipv4Addr(
      static_cast<u32>(a) << 24 | static_cast<u32>(b) << 16 | static_cast<u32>(c) << 8
      | static_cast<u32>(d)
    );
  }

  [[nodiscard]] static Ipv4Addr parse(std::string_view text);

  [[nodiscard]] constexpr u32 address() const noexcept { return this->m_address; }

  [[nodiscard]] constexpr std::array<u8, 4> octets() const noexcept
  {
    return {
      static_cast<u8>((this->m_address >> 24) & 0xffU),
      static_cast<u8>((this->m_address >> 16) & 0xffU),
      static_cast<u8>((this->m_address >> 8) & 0xffU),
      static_cast<u8>(this->m_address & 0xffU),
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
  u32 m_address = 0;
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

  explicit constexpr Ipv6Addr(std::array<u8, 16> const& octets) noexcept
    : m_octets(octets)
  {}

  [[nodiscard]] static constexpr Ipv6Addr from_segments(
    u16 const a,
    u16 const b,
    u16 const c,
    u16 const d,
    u16 const e,
    u16 const f,
    u16 const g,
    u16 const h
  ) noexcept
  {
    return Ipv6Addr({
      static_cast<u8>(a >> 8),
      static_cast<u8>(a & 0xFFu),
      static_cast<u8>(b >> 8),
      static_cast<u8>(b & 0xFFu),
      static_cast<u8>(c >> 8),
      static_cast<u8>(c & 0xFFu),
      static_cast<u8>(d >> 8),
      static_cast<u8>(d & 0xFFu),
      static_cast<u8>(e >> 8),
      static_cast<u8>(e & 0xFFu),
      static_cast<u8>(f >> 8),
      static_cast<u8>(f & 0xFFu),
      static_cast<u8>(g >> 8),
      static_cast<u8>(g & 0xFFu),
      static_cast<u8>(h >> 8),
      static_cast<u8>(h & 0xFFu),
    });
  }

  [[nodiscard]] static Ipv6Addr parse(std::string_view text);

  [[nodiscard]] constexpr std::array<u8, 16> const& octets() const noexcept
  {
    return this->m_octets;
  }

  [[nodiscard]] constexpr std::array<u16, 8> segments() const noexcept
  {
    auto result = std::array<u16, 8> {};
    for(auto i = 0uz; i < result.size(); ++i) {
      result.at(i) = static_cast<u16>(
        static_cast<u16>(this->m_octets.at(i * 2)) << 8
        | static_cast<u16>(this->m_octets.at(i * 2 + 1))
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
    auto constexpr localhost = std::array<u8, 16> {
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
    return this->m_octets.front() == 0xFF;
  }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] constexpr bool operator==(Ipv6Addr const&) const noexcept = default;
  [[nodiscard]] constexpr std::strong_ordering operator<=>(Ipv6Addr const&) const noexcept =
    default;

 private:
  std::array<u8, 16> m_octets {};
};

W_COMMON_API std::ostream& operator<<(std::ostream& stream, Ipv4Addr const& address);
W_COMMON_API std::ostream& operator<<(std::ostream& stream, Ipv6Addr const& address);

inline namespace literals
{
  /**
   * @brief Creates an IPv4 address from a string literal, e.g. `"192.168.0.1"_ipv4`.
   * @throws IpError if the literal is not an IPv4 address.
   * @ingroup core_networking
   * @relates Ipv4Addr
   */
  [[nodiscard]] inline Ipv4Addr operator""_ipv4(char const* text, usize const length)
  {
    return Ipv4Addr::parse(std::string_view {text, length});
  }

  /**
   * @brief Creates an IPv6 address from a string literal, e.g. `"::1"_ipv6`.
   * @throws IpError if the literal is not an IPv6 address.
   * @ingroup core_networking
   * @relates Ipv6Addr
   */
  [[nodiscard]] inline Ipv6Addr operator""_ipv6(char const* text, usize const length)
  {
    return Ipv6Addr::parse(std::string_view {text, length});
  }
}  // namespace literals

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::Ipv4Addr>
{
  [[nodiscard]] std::size_t operator()(w::common::Ipv4Addr const& address) const noexcept
  {
    return std::hash<w::u32> {}(address.address());
  }
};

template <>
struct std::hash<w::common::Ipv6Addr>
{
  [[nodiscard]] std::size_t operator()(w::common::Ipv6Addr const& address) const noexcept
  {
    auto hash = 0uz;
    for(auto const byte : address.octets()) {
      hash ^= std::hash<w::u8> {}(byte) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    }
    return hash;
  }
};

#ifndef __DOXYGEN__
template <>
struct fmt::formatter<w::common::Ipv4Addr> : ostream_formatter
{};

template <>
struct fmt::formatter<w::common::Ipv6Addr> : ostream_formatter
{};
#endif
