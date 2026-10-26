#pragma once

// Qt's moc does not support the C++23 #elifdef directive yet.
// NOLINTBEGIN(readability-use-concise-preprocessor-directives)
#ifdef __DOXYGEN__
#  define W_COMMON_NAMESPACE_BEGIN namespace w { namespace common {
#  define W_COMMON_NAMESPACE_END } }
#elif defined(__cplusplus)
#  define W_COMMON_NAMESPACE_BEGIN namespace w { inline namespace common {
#  define W_COMMON_NAMESPACE_END } }
#else
#  define W_COMMON_NAMESPACE_BEGIN
#  define W_COMMON_NAMESPACE_END
#endif
// NOLINTEND(readability-use-concise-preprocessor-directives)
#define W_COMMON_FORWARD_DECLARE(TYPE) /* NOLINT(cppcoreguidelines-macro-usage) */ \
  W_COMMON_NAMESPACE_BEGIN TYPE; W_COMMON_NAMESPACE_END

/**
 * @defgroup macros Macros
 * @ingroup core
 * @brief Public preprocessor helpers and configuration flags.
 */

#ifdef __DOXYGEN__
/**
 * @ingroup macros
 * @brief Flag defined if debug mode is **enabled**.
 * @details Enables some additional checks, the majority of contract functionality and some
 * assertions.
 * @see W_NO_DEBUG
 */
#  define W_DEBUG

/**
 * @ingroup macros
 * @brief Flag defined if debug mode is **disabled**.
 * @details Disables some additional checks, the majority of contract functionality and some
 * assertions.
 * @see W_DEBUG
 */
#  define W_NO_DEBUG
#endif
