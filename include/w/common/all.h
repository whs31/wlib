#pragma once

/**
 * @defgroup core WLib Core
 * @brief Foundational types and utilities shared by WLib-based libraries.
 *
 * The core module contains small, dependency-light building blocks: error types, checks, value
 * types, measurement values, adapters, memory wrappers, math helpers, filesystem helpers, binary
 * serialization, threading guards, and functional utilities.
 */

/**
 * @defgroup core_measurements Measurements
 * @ingroup core
 * @brief Units, physical constants, and strongly typed measurement values.
 * @details Every scalar quantity is a template `BasicX<T>` over its precision that derives from
 * QuantityBase and stores one `T` in its SI unit; the alias `X` is `BasicX<float>`, and
 * `BasicX<double>` is spelled out. A `float` quantity widens to `double` implicitly; narrowing is
 * explicit. The geographic coordinates Latitude and Longitude are not quantities and always store
 * a `double`.
 * @see QuantityBase
 * @see Distance
 * @see Velocity
 */

/**
 * @defgroup core_identifiers Identifiers and Versions
 * @ingroup core
 * @brief UUID and Semantic Versioning value types.
 * @see Uuid
 * @see Version
 */

/**
 * @defgroup core_networking Networking
 * @ingroup core
 * @brief Networking address types and parsing helpers.
 */

/**
 * @defgroup core_logging Logging
 * @ingroup core
 * @brief spdlog logging setup helpers; the wlib_qt package bridges Qt messages to them.
 */

/**
 * @defgroup core_filesystem Filesystem
 * @ingroup core
 * @brief Platform path helpers and filesystem-based locking.
 */

#include <w/common/aabb.h>
#include <w/common/acceleration.h>
#include <w/common/adapter.h>
#include <w/common/angle.h>
#include <w/common/angular_acceleration.h>
#include <w/common/angular_velocity.h>
#include <w/common/bit.h>
#include <w/common/bytes.h>
#include <w/common/capacitance.h>
#include <w/common/char_utils.h>
#include <w/common/charge.h>
#include <w/common/check.h>
#include <w/common/clip_space.h>
#include <w/common/color.h>
#include <w/common/concepts.h>
#include <w/common/conductance.h>
#include <w/common/constants.h>
#include <w/common/conventions.h>
#include <w/common/current.h>
#include <w/common/curve.h>
#include <w/common/density.h>
#include <w/common/distance.h>
#include <w/common/energy.h>
#include <w/common/error.h>
#include <w/common/euler_angles.h>
#include <w/common/fatal.h>
#include <w/common/force.h>
#include <w/common/frequency.h>
#include <w/common/frustum.h>
#include <w/common/functional.h>
#include <w/common/geodesy.h>
#include <w/common/graph_resolver.h>
#include <w/common/ip.h>
#include <w/common/latitude.h>
#include <w/common/lockfile.h>
#include <w/common/log.h>
#include <w/common/longitude.h>
#include <w/common/magnetic_flux_density.h>
#include <w/common/mass.h>
#include <w/common/math.h>
#include <w/common/matrix3x3.h>
#include <w/common/matrix4x4.h>
#include <w/common/memo.h>
#include <w/common/memory.h>
#include <w/common/moment_of_inertia.h>
#include <w/common/noise.h>
#include <w/common/paths.h>
#include <w/common/pimpl.h>
#include <w/common/plane.h>
#include <w/common/power.h>
#include <w/common/precision.h>
#include <w/common/pressure.h>
#include <w/common/projection.h>
#include <w/common/quantity.h>
#include <w/common/quaternion.h>
#include <w/common/random.h>
#include <w/common/ray.h>
#include <w/common/rect.h>
#include <w/common/resistance.h>
#include <w/common/sockaddr.h>
#include <w/common/sphere.h>
#include <w/common/temperature.h>
#include <w/common/threading.h>
#include <w/common/torque.h>
#include <w/common/transform.h>
#include <w/common/uuid.h>
#include <w/common/vector2.h>
#include <w/common/vector3.h>
#include <w/common/vector4.h>
#include <w/common/velocity.h>
#include <w/common/version.h>
#include <w/common/version_range.h>
#include <w/common/voltage.h>
#include <w/common/web_mercator.h>
#include <w/common/wgs84.h>
