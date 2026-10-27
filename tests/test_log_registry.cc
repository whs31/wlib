#include <w/common/log.h>

#include <catch2/catch_test_macros.hpp>

#if WLIB_TEST_REGISTRY_MODULE
extern "C" bool wlib_registry_module_has_logger(char const* name);
extern "C" bool wlib_registry_module_default_is(char const* name);
#endif

using namespace w;

TEST_CASE("spdlog's registry is shared with other shared libraries", "[log]")
{
#if WLIB_TEST_REGISTRY_MODULE
  auto const previous = spdlog::default_logger();
  create_logger(
    LoggerConfig {.name = "wlib-registry-test", .is_default = true, .target = LogTarget::Stdout}
  );

  CHECK(wlib_registry_module_has_logger("wlib-registry-test"));
  CHECK(wlib_registry_module_default_is("wlib-registry-test"));

  spdlog::set_default_logger(previous);
  spdlog::drop("wlib-registry-test");
  CHECK_FALSE(wlib_registry_module_has_logger("wlib-registry-test"));
#else
  SKIP("static build: one binary, one registry");
#endif
}
