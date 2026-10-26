#pragma once

#include <cstdlib>
#include <cassert>

#include <w/common/config.h>
#include <w/common/error.h>

// NOLINTBEGIN(*-reserved-identifier, *-identifier-naming, *-macro-usage)

/**
 * @ingroup macros
 * @brief Performs a debug-only runtime check.
 * @details In debug builds, throws `EXCEPTION_TYPE(__VA_ARGS__)` when `x`
 * evaluates to `false`. In release builds, the expression and constructor
 * arguments are not evaluated.
 * @param x Boolean expression to check.
 * @param EXCEPTION_TYPE Exception type to construct on failure.
 * @param ... Arguments forwarded to the exception constructor.
 * @see W_HARD_CHECK
 */
#ifdef W_DEBUG
#  define W_SOFT_CHECK(x, EXCEPTION_TYPE, ...)                                              \
    do {                                                                                        \
      if(not (x))                                                                               \
        throw EXCEPTION_TYPE(__VA_ARGS__);                                                      \
    } while(false)
#else
#  define W_SOFT_CHECK(x, EXCEPTION_TYPE, ...)                                              \
    do {                                                                                        \
    } while(false)
#endif

/**
 * @ingroup macros
 * @brief Performs an unconditional runtime check.
 * @details Throws `EXCEPTION_TYPE(__VA_ARGS__)` when `x` evaluates to `false`
 * in every build configuration.
 * @param x Boolean expression to check.
 * @param EXCEPTION_TYPE Exception type to construct on failure.
 * @param ... Arguments forwarded to the exception constructor.
 * @see W_SOFT_CHECK
 */
#define W_HARD_CHECK(x, EXCEPTION_TYPE, ...)                                                \
  do {                                                                                          \
    if(not (x))                                                                                 \
      throw EXCEPTION_TYPE(__VA_ARGS__);                                                        \
  } while(false)

/**
 * @ingroup macros
 * @brief Throws `w::common::NotImplementedError`.
 * @details Uses the exception's default source-location argument, so the
 * reported location is the expansion site.
 */
#define W_NOT_IMPLEMENTED throw w::common::NotImplementedError();

// NOLINTEND(*-reserved-identifier, *-identifier-naming, *-macro-usage)
