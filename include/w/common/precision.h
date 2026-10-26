#pragma once

#include <w/common/config.h>
#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Floating-point type of the physical quantities and the linear algebra types.
 * @details `float` by default, `double` when the library is built with `precision=double` in Conan
 * (or `WLIB_PRECISION=double` in CMake), which defines `W_COMMON_DOUBLE_PRECISION` in
 * `<w/common/config.h>`. The choice is part of the ABI: every translation unit of a program sees
 * the same `Real`.
 *
 * Positions on the Earth need more than the 24 bits of a `float` mantissa, so the geodetic types
 * (Latitude, Longitude, GeodeticPosition, LocalTangentPlane, the Web Mercator helpers) compute in
 * `double` regardless of this option.
 * @ingroup core_measurements
 */
#ifdef W_COMMON_DOUBLE_PRECISION
using Real = double;
#else
using Real = float;
#endif

W_COMMON_NAMESPACE_END
