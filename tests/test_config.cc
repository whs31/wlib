#include <w/common/config.h>

#include <concepts>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include <w/common/precision.h>

#ifdef W_COMMON_WITH_QT
inline auto constexpr wlib_with_qt = true;
#else
inline auto constexpr wlib_with_qt = false;
#endif

#ifdef W_COMMON_DOUBLE_PRECISION
inline auto constexpr wlib_double_precision = true;
#else
inline auto constexpr wlib_double_precision = false;
#endif

TEST_CASE("Version macros encode and decode components", "[core][config]")
{
  auto constexpr version = W_VERSION_ENCODE(12, 34, 56);

  STATIC_CHECK(version == 12'034'056);
  STATIC_CHECK(W_VERSION_DECODE_MAJOR(version) == 12);
  STATIC_CHECK(W_VERSION_DECODE_MINOR(version) == 34);
  STATIC_CHECK(W_VERSION_DECODE_PATCH(version) == 56);
}

TEST_CASE("Preprocessor helpers expand arguments", "[core][config]")
{
#define WLIB_TEST_NUMBER 42  // NOLINT(cppcoreguidelines-macro-usage): the helpers expand macros
  auto constexpr WLIB_TEST_TOKEN_SUFFIX = 8;

  STATIC_CHECK(std::string_view {W_STRINGIFY(WLIB_TEST_NUMBER)} == "42");
  STATIC_CHECK(W_CONCAT(WLIB_TEST_TOKEN, _SUFFIX) == 8);
  STATIC_CHECK(W_CONCAT3(WLIB_TEST_, TOKEN, _SUFFIX) == 8);
#undef WLIB_TEST_NUMBER
}

TEST_CASE("Generated configuration records Qt support", "[core][config][qt]")
{
#ifdef QT_CORE_LIB
  STATIC_CHECK(wlib_with_qt);
#else
  STATIC_CHECK_FALSE(wlib_with_qt);
#endif
}

TEST_CASE("Generated configuration records the precision of Real", "[core][config]")
{
  STATIC_CHECK(std::floating_point<w::Real>);
  STATIC_CHECK(std::same_as<w::Real, double> == wlib_double_precision);
  STATIC_CHECK(std::same_as<w::Real, float> == not wlib_double_precision);
}
