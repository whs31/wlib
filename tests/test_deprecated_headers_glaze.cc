// note: [whs31 : 2026-11-03]
// the glaze header of the old spelling stays for the users that still include it; it is the only
// w/common include here, so it must be enough on its own
#include <w/common/glaze/colour.h>

#include <catch2/catch_test_macros.hpp>

#include "glaze_support.h"

using namespace w::literals;

TEST_CASE("The deprecated spelling of the glaze color header still adapts the colors", "[glaze]")
{
  glaze_support::check_round_trip("#e4a672"_rgba8, R"("#e4a672")");
}
