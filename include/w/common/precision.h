#pragma once

#include <concepts>
#include <limits>

#include <w/common/detail/namespace.h>

W_COMMON_NAMESPACE_BEGIN

/**
 * @brief Default floating-point type of the physical quantities and the linear algebra types:
 * `float`.
 * @details Every quantity and linear algebra type is a class template over its scalar, e.g.
 * `BasicAngle<T>` or `BasicVector3<T>`, and its short name is the `float` instantiation:
 * `Angle` is `BasicAngle<float>` and `Vector3` is `BasicVector3<float>`. Game components, GPU
 * constants and physics engines run in `float`; a simulation that needs more digits names the
 * `double` instantiation, e.g. `BasicVector3<double>`.
 *
 * A value converts implicitly to a precision that keeps every digit, e.g. from `float` to
 * `double`, and explicitly to one that does not. Arithmetic mixing two precisions therefore
 * happens in the wider one.
 *
 * Positions on the Earth need more than the 24 bits of a `float` mantissa, so the geodetic types
 * (Latitude, Longitude, LocalTangentPlane, the Web Mercator helpers) compute in `double`.
 * @ingroup core_measurements
 * @see LosslessConversion
 */
using Real = float;

static_assert(sizeof(Real) == 4, "the default precision of WLib is a 32-bit float");

/**
 * @brief Concept that is true if every value of the floating-point type @p From is exactly a
 * value of @p To, e.g. `float` to `double`.
 * @details The quantities and the linear algebra types convert implicitly between precisions only
 * where this holds.
 * @ingroup core_concepts
 */
template <typename From, typename To>
concept LosslessConversion = std::floating_point<From> and std::floating_point<To>
                         and std::numeric_limits<From>::digits <= std::numeric_limits<To>::digits
                         and std::numeric_limits<From>::max_exponent
                               <= std::numeric_limits<To>::max_exponent
                         and std::numeric_limits<From>::min_exponent
                               >= std::numeric_limits<To>::min_exponent;

static_assert(LosslessConversion<float, double>);
static_assert(not LosslessConversion<double, float>);

W_COMMON_NAMESPACE_END
