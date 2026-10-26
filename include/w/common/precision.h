#pragma once

#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Floating-point type of the physical quantities and the linear algebra types: `float`.
 * @details Game components, GPU constants and physics engines all run in `float`, so the library
 * does too. The alias stays as the one name of that choice, and the layouts of the linear algebra
 * types are checked against it.
 *
 * Positions on the Earth need more than the 24 bits of a `float` mantissa, so the geodetic types
 * (Latitude, Longitude, GeodeticPosition, LocalTangentPlane, the Web Mercator helpers) compute in
 * `double`.
 * @ingroup core_measurements
 */
using Real = float;

static_assert(sizeof(Real) == 4, "WLib stores its quantities and vectors as 32-bit floats");

W_COMMON_NAMESPACE_END
