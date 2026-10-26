#define W_LOG_ACTIVE_LEVEL 3  // NOLINT(cppcoreguidelines-macro-usage): configures log.h below
#include <w/common/log.h>

#include <string>

#include <catch2/catch_test_macros.hpp>

using namespace w;

TEST_CASE("Logging macros discard calls below their compile-time level", "[log]")
{
  auto const name = std::string {"wlib_log_test_compile_level"};
  auto config = LoggerConfig {
    .name = name,
    .level = spdlog::level::trace,
    .target = LogTarget::Stdout,
    .pattern = {}
  };
  auto const logger = create_logger(config);
  auto evaluations = 0;

  W_LOG_TO(name, debug, "discarded argument {}", ++evaluations);
  W_LOG_TO(name, warn, "retained argument {}", ++evaluations);
  W_LOG(debug, "discarded default argument {}", ++evaluations);
  W_LOG(warn, "retained default argument {}", ++evaluations);

  REQUIRE(evaluations == 2);
  spdlog::drop(name);
}
