#pragma once

#include <string>

#include <w/common/glaze/detail.h>
#include <w/common/ip.h>

/**
 * @brief Glaze metadata of Ipv4Addr: its dotted string, e.g. `"192.168.0.1"`.
 * @details Reading fails with `constraint_violated` on an invalid address.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Ipv4Addr>
{
  static auto constexpr value = glz::custom<
    [](w::common::Ipv4Addr& address, std::string const& text, glz::context& ctx) {
      try {
        address = w::common::Ipv4Addr::parse(text);
      } catch(w::common::IpError const&) {
        w::common::_glaze_impl::reject(ctx, "invalid ipv4 address");
      }
    },
    [](w::common::Ipv4Addr const& address) { return address.to_string(); }>;
};

/**
 * @brief Glaze metadata of Ipv6Addr: its string, e.g. `"::1"`.
 * @details Reading fails with `constraint_violated` on an invalid address.
 * @ingroup core_glaze
 */
template <>
struct glz::meta<w::common::Ipv6Addr>
{
  static auto constexpr value = glz::custom<
    [](w::common::Ipv6Addr& address, std::string const& text, glz::context& ctx) {
      try {
        address = w::common::Ipv6Addr::parse(text);
      } catch(w::common::IpError const&) {
        w::common::_glaze_impl::reject(ctx, "invalid ipv6 address");
      }
    },
    [](w::common::Ipv6Addr const& address) { return address.to_string(); }>;
};
