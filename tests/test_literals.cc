#include <w/common/colour.h>
#include <w/common/ip.h>
#include <w/common/uuid.h>
#include <w/common/version.h>

#include <catch2/catch_test_macros.hpp>

// note: [whs31 : 2026-10-27]
// unlike the other tests, this file does not use namespace w, which would find a literal in any
// namespace of w: the directive below must be enough for a consumer
using namespace w::literals;

TEST_CASE(
  "The string literals of the colours, identifiers and addresses are in w::literals",
  "[literals]"
)
{
  STATIC_CHECK("#e4a672"_rgba8 == w::Rgba8(0xe4, 0xa6, 0x72));
  STATIC_CHECK(
    "00112233-4455-6677-8899-aabbccddeeff"_uuid
    == w::Uuid::from_u64_pair(0x0011'2233'4455'6677, 0x8899'aabb'ccdd'eeff)
  );
  CHECK("1.2.3"_version == w::Version(1, 2, 3));
  CHECK("127.0.0.1"_ipv4 == w::Ipv4Addr::LOCALHOST);
  CHECK("::1"_ipv6 == w::Ipv6Addr::LOCALHOST);
}
