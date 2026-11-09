#pragma once

#include <compare>
#include <functional>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <w/common/stdint.h>
#include <w/common/config.h>
#include <w/common/detail/export.h>
#include <w/common/detail/namespace.h>
#include <w/common/ip.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief IPv4 socket address.
 * @ingroup core_networking
 */
class W_COMMON_API SockAddrV4
{
 public:
  constexpr SockAddrV4() noexcept = default;

  constexpr SockAddrV4(Ipv4Addr const ip, u16 const port) noexcept
    : m_ip(ip)
    , m_port(port)
  {}

  [[nodiscard]] static constexpr SockAddrV4 from_parts(Ipv4Addr const ip, u16 const port) noexcept
  {
    return {ip, port};
  }

  [[nodiscard]] static SockAddrV4 parse(std::string_view text);

  [[nodiscard]] constexpr Ipv4Addr ip() const noexcept { return this->m_ip; }

  [[nodiscard]] constexpr u32 port() const noexcept { return this->m_port; }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] constexpr bool operator==(SockAddrV4 const&) const noexcept = default;
  [[nodiscard]] constexpr std::strong_ordering operator<=>(SockAddrV4 const&) const noexcept =
    default;

 private:
  Ipv4Addr m_ip;
  u16 m_port = 0;
};

/**
 * @brief IPv6 socket address.
 * @ingroup core_networking
 */
class W_COMMON_API SockAddrV6
{
 public:
  constexpr SockAddrV6() noexcept = default;

  constexpr SockAddrV6(
    Ipv6Addr const ip,
    u16 const port,
    u32 const flow_info = 0,
    u32 const scope_id = 0
  ) noexcept
    : m_ip(ip)
    , m_port(port)
    , m_flow_info(flow_info)
    , m_scope_id(scope_id)
  {}

  [[nodiscard]] static constexpr SockAddrV6 from_parts(
    Ipv6Addr const ip,
    u16 const port,
    u32 const flow_info = 0,
    u32 const scope_id = 0
  ) noexcept
  {
    return {ip, port, flow_info, scope_id};
  }

  [[nodiscard]] static SockAddrV6 parse(std::string_view text);

  [[nodiscard]] constexpr Ipv6Addr ip() const noexcept { return this->m_ip; }

  [[nodiscard]] constexpr u32 port() const noexcept { return this->m_port; }

  [[nodiscard]] constexpr u32 flow_info() const noexcept { return this->m_flow_info; }

  [[nodiscard]] constexpr u32 scope_id() const noexcept { return this->m_scope_id; }

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] constexpr bool operator==(SockAddrV6 const&) const noexcept = default;
  [[nodiscard]] constexpr std::strong_ordering operator<=>(SockAddrV6 const&) const noexcept =
    default;

 private:
  Ipv6Addr m_ip;
  u16 m_port = 0;
  u32 m_flow_info = 0;
  u32 m_scope_id = 0;
};

/**
 * @brief Type-erased IPv4 or IPv6 socket address.
 * @ingroup core_networking
 */
class W_COMMON_API SockAddr
{
 public:
  enum class Family : u8
  {
    Ipv4,
    Ipv6,
  };

  SockAddr() noexcept = default;

  explicit SockAddr(SockAddrV4 const& address) noexcept
    : m_address(address)
  {}

  explicit SockAddr(SockAddrV6 const& address) noexcept
    : m_address(address)
  {}

  [[nodiscard]] static SockAddr parse(std::string_view text);

  [[nodiscard]] bool is_ipv4() const noexcept;

  [[nodiscard]] bool is_ipv6() const noexcept;

  [[nodiscard]] Family family() const noexcept;

  [[nodiscard]] u32 port() const noexcept;

  [[nodiscard]] SockAddrV4 const& as_v4() const;

  [[nodiscard]] SockAddrV6 const& as_v6() const;

  [[nodiscard]] std::string to_string() const;

  [[nodiscard]] bool operator==(SockAddr const& other) const noexcept;
  [[nodiscard]] std::strong_ordering operator<=>(SockAddr const& other) const noexcept;

 private:
  std::variant<SockAddrV4, SockAddrV6> m_address = SockAddrV4();
};

W_COMMON_API std::ostream& operator<<(std::ostream& stream, SockAddrV4 const& address);
W_COMMON_API std::ostream& operator<<(std::ostream& stream, SockAddrV6 const& address);
W_COMMON_API std::ostream& operator<<(std::ostream& stream, SockAddr const& address);

W_COMMON_NAMESPACE_END

template <>
struct std::hash<w::common::SockAddrV4>
{
  [[nodiscard]] std::size_t operator()(w::common::SockAddrV4 const& address) const noexcept
  {
    auto hash = std::hash<w::common::Ipv4Addr> {}(address.ip());
    hash ^= std::hash<w::u32> {}(address.port()) + 0x9e3779b97f4a7c15ULL + (hash << 6)
          + (hash >> 2);
    return hash;
  }
};

template <>
struct std::hash<w::common::SockAddrV6>
{
  [[nodiscard]] std::size_t operator()(w::common::SockAddrV6 const& address) const noexcept
  {
    auto hash = std::hash<w::common::Ipv6Addr> {}(address.ip());
    for(auto const value : {address.port(), address.flow_info(), address.scope_id()}) {
      hash ^= std::hash<w::u32> {}(value) + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
    }
    return hash;
  }
};

template <>
struct std::hash<w::common::SockAddr>
{
  [[nodiscard]] std::size_t operator()(w::common::SockAddr const& address) const
  {
    return address.is_ipv4() ? std::hash<w::common::SockAddrV4> {}(address.as_v4())
                             : std::hash<w::common::SockAddrV6> {}(address.as_v6());
  }
};

#ifndef __DOXYGEN__
template <>
struct fmt::formatter<w::common::SockAddrV4> : ostream_formatter
{};

template <>
struct fmt::formatter<w::common::SockAddrV6> : ostream_formatter
{};

template <>
struct fmt::formatter<w::common::SockAddr> : ostream_formatter
{};
#endif
