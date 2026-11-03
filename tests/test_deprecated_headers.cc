// note: [whs31 : 2026-11-03]
// the headers of the old spelling stay for the users that still include them; each is the only
// include here, so it must be enough on its own
#include <w/common/colour.h>

#include <catch2/catch_test_macros.hpp>

using namespace w::literals;

TEST_CASE("The deprecated spelling of the color header still provides the color types", "[headers]")
{
  STATIC_CHECK("#e4a672"_rgba8 == w::Rgba8(0xe4, 0xa6, 0x72));
  CHECK(w::Rgba8(0xe4, 0xa6, 0x72).to_hex() == "#e4a672");
}
