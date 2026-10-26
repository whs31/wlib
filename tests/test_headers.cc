#include <w/common/all.h>

#include <catch2/catch_test_macros.hpp>

using namespace w;

TEST_CASE("Umbrella header exports the public API", "[headers]")
{
  STATIC_CHECK(SECONDS_HOURS_FACTOR == 3'600.0);
  SUCCEED();
}
