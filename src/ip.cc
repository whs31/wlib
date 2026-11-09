#include <w/common/ip.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
#include <string>

#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#endif

#include <w/common/stdint.h>

namespace
{
  [[nodiscard]] int parse_address(int const family, char const* const text, void* const output)
  {
#ifdef _WIN32
    return InetPtonA(family, text, output);
#else
    return inet_pton(family, text, output);
#endif
  }

  [[nodiscard]] char const* format_address(
    int const family,
    void const* const address,
    char* const output,
    w::usize const size
  )
  {
#ifdef _WIN32
    return InetNtopA(family, const_cast<void*>(address), output, size);  // NOLINT(*-const-cast)
#else
    return inet_ntop(family, address, output, static_cast<socklen_t>(size));
#endif
  }
}  // namespace

W_COMMON_NAMESPACE_BEGIN

Ipv4Addr const Ipv4Addr::LOCALHOST = Ipv4Addr::from_octets(127, 0, 0, 1);
Ipv4Addr const Ipv4Addr::UNSPECIFIED = Ipv4Addr::from_octets(0, 0, 0, 0);
Ipv4Addr const Ipv4Addr::BROADCAST = Ipv4Addr::from_octets(255, 255, 255, 255);

Ipv6Addr const Ipv6Addr::LOCALHOST = Ipv6Addr::from_segments(0, 0, 0, 0, 0, 0, 0, 1);
Ipv6Addr const Ipv6Addr::UNSPECIFIED = Ipv6Addr::from_segments(0, 0, 0, 0, 0, 0, 0, 0);

IpError IpError::invalid_v4_string(std::string_view const text)
{
  return IpError("invalid ipv4 string: {}", text);
}

IpError IpError::invalid_v6_string(std::string_view const text)
{
  return IpError("invalid ipv6 string: {}", text);
}

IpError IpError::missing_colon(std::string_view const text)
{
  return IpError("missing colon in a socket address: {}", text);
}

IpError IpError::invalid_port(std::string_view const text)
{
  return IpError("invalid port: {}", text);
}

IpError IpError::not_a_v4() { return IpError("an attempt to cast an ipv6 address to ipv4"); }

IpError IpError::not_a_v6() { return IpError("an attempt to cast an ipv4 address to ipv6"); }

Ipv4Addr Ipv4Addr::parse(std::string_view const text)
{
  if(text.empty() or text.contains('\0'))
    throw IpError::invalid_v4_string(text);

  auto native = in_addr {};
  auto const terminated = std::string(text);
  if(parse_address(AF_INET, terminated.c_str(), &native) != 1)
    throw IpError::invalid_v4_string(text);

  auto octets = std::array<u8, 4> {};
  std::memcpy(octets.data(), &native, octets.size());
  return from_octets(octets.at(0), octets.at(1), octets.at(2), octets.at(3));
}

std::string Ipv4Addr::to_string() const
{
  auto const [a, b, c, d] = this->octets();
  return fmt::format("{}.{}.{}.{}", a, b, c, d);
}

Ipv6Addr Ipv6Addr::parse(std::string_view const text)
{
  if(text.empty() or text.contains('\0'))
    throw IpError::invalid_v6_string(text);

  auto native = in6_addr {};
  auto const terminated = std::string(text);
  if(parse_address(AF_INET6, terminated.c_str(), &native) != 1)
    throw IpError::invalid_v6_string(text);

  auto octets = std::array<u8, 16> {};
  std::memcpy(octets.data(), &native, octets.size());
  return Ipv6Addr(octets);
}

std::string Ipv6Addr::to_string() const
{
  auto buffer = std::array<char, INET6_ADDRSTRLEN> {};
  if(format_address(AF_INET6, this->m_octets.data(), buffer.data(), buffer.size()) == nullptr)
    throw IpError("failed to format ipv6 address");
  return buffer.data();
}

std::ostream& operator<<(std::ostream& stream, Ipv4Addr const& address)
{
  return stream << address.to_string();
}

std::ostream& operator<<(std::ostream& stream, Ipv6Addr const& address)
{
  return stream << address.to_string();
}

W_COMMON_NAMESPACE_END
