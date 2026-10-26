---
name: rcommon
description: Use RCommon library instead of reimplementing shared C++ infrastructure. Use when adding identifiers, physical quantities, geometry, errors, Pimpl, non-owning pointers, adapters, checks, logging, filesystem paths, semantic versions, JSON adapters, dependency graphs, or similar common utilities in C++ repositories.
---

# RCommon usage
RCommon is the shared C++ utility and domain-primitives library for organization projects.

Before creating a local utility, wrapper, value type, or infrastructure abstraction, check whether RCommon already provides the concept.

Prefer RCommon when its semantics match the requirement. Do not force an unrelated application-specific concept into RCommon abstractions merely to reuse a type.

Use focused RCommon headers rather than `<radar/mms/common/all.h>`.

The namespace is:
```cpp
radar::mms::common
```

`common` inline namespace should be omitted everywhere **except** for inside `Q_PROPERTY` declarations or `Q_DECLARE_METATYPE`/`Q_INVOKABLE`/`Q_SIGNALS`.

## Dependency setup
The Conan package is:
```python
self.req(
    "rcommon@radar/dev",
    transitive_headers=True,
    transitive_libs=True,
)
```

Enable Qt integration only when the consumer needs RCommon's Qt conversions, Qt metatypes, or other Qt-specific functionality:
```python
self.req(
    "rcommon@radar/dev",
    options={"with_qt": True},
    transitive_headers=True,
    transitive_libs=True,
)
```

In CMake, prefer the exported core target:
```cmake
radar::mms::common
```

Use the JSON target only when RCommon's nlohmann/json adapters are required:
```cmake
radar::mms::common::json
```

Do not add the JSON component merely because the project otherwise uses nlohmann/json.

`rcommon_schema` is a separate dependency. Add it when consuming the shared protobuf schema, not merely because the C++ RCommon library is used.

Follow the current repository's Conan helper and package-component conventions instead of rewriting its dependency plumbing.

## Strong domain types
Prefer RCommon value types over primitive values when the concept matches.

### Identifiers
Use:
```cpp
#include <radar/mms/common/uuid.h>
```

Prefer `Uuid` for stable object identifiers rather than raw strings, ad hoc UUID wrappers, or `QUuid` leaking through domain APIs.

Use the existing protobuf/JSON/Qt adapters when crossing those boundaries rather than adding another conversion layer.

### Physical quantities
Use:
```cpp
#include <radar/mms/common/angle.h>
#include <radar/mms/common/distance.h>
#include <radar/mms/common/velocity.h>
#include <radar/mms/common/euler_angles.h>
```

Prefer:

```cpp
Angle
Distance
Velocity
EulerAngles
```
over untyped `float` or `double` parameters when the value represents those physical quantities.
Always use literals such as `_deg` or `_meters` whenever possible.

Do not encode units only in variable names such as `angle_deg`, `distance_m`, or `velocity_mps` when a public or domain API can carry the unit in its type. 
This pattern is already used by many projects in the organization and should be preferred for new compatible APIs.

### Geometry
Check these before adding project-local point or vector types:
```cpp
#include <radar/mms/common/point2.h>
#include <radar/mms/common/point3.h>
#include <radar/mms/common/vector2.h>
#include <radar/mms/common/vector3.h>
```

Use `Point2`/`Point3` for positions and `Vector2`/`Vector3` for displacement or direction when their existing semantics fit.

Use the provided operations and Qt conversions instead of duplicating vector math.

## Error hierarchy
Use:
```cpp
#include <radar/mms/common/error.h>
```

Project-specific exception hierarchies should normally derive from:
```cpp
common::Error
```

Example pattern:
```cpp
class Error : public common::Error
{
 protected:
  template <typename... Args>
  explicit Error(fmt::format_string<Args...> format, Args&&... args)
    : common::Error(format, std::forward<Args>(args)...)
  {}
};
```

Prefer meaningful project-specific error factories or subclasses over repeatedly throwing generic string-only exceptions.

Use `SystemError` when adapting an appropriate `std::system_error`.

Use `NotImplementedError` / `RADAR_NOT_IMPLEMENTED` only for intentionally unimplemented code paths, not for ordinary error handling.

## Pimpl
Use:
```cpp
#include <radar/mms/common/pimpl.h>
```

Prefer RCommon Pimpl utilities rather than introducing another project-local Pimpl wrapper.

For the conventional private implementation:
```cpp
class Widget
{
 public:
  Widget();
  ~Widget();

 private:
  DECLARE_PRIVATE(Widget)
};
```

Implement:
```cpp
struct Widget::WidgetPrivate
{
  // ...
};
```
and initialize `d` with `std::make_unique<WidgetPrivate>(...)`.

Use `Pimpl<T>` directly when the conventional `DECLARE_PRIVATE` form does not fit.

Use `SharedPimpl<T>` or the shared declaration macros only when shared implementation ownership is intentional. Do not use shared ownership merely for convenience.

Pimpl is particularly appropriate for public library APIs where implementation details, header weight, or binary boundaries should remain private.

## Non-owning pointers
Use:
```cpp
#include <radar/mms/common/memory/observer_ptr.h>
```

Prefer `observer_ptr<T>` for stored non-owning object references when pointer semantics are required.

Use `make_observer(...)` when constructing them.

A raw pointer is still appropriate where an external API defines the convention, especially:
```cpp
QObject* parent
```

and Qt APIs that explicitly communicate ownership through QObject parenting.
Do not replace clear references with `observer_ptr` when nullability is not required.

## Adapters
Before building another non-owning adapter base, check:
```cpp
#include <radar/mms/common/adapter.h>
```

`Adapter<T>` stores a non-owning `observer_ptr` to an externally owned adaptee.

Use `Adapter<T const>` for read-only adaptation.

The adapted object must outlive the adapter.

## Runtime checks
Use:
```cpp
#include <radar/mms/common/check.h>
```

Available checks include:
```cpp
RADAR_HARD_CHECK(condition, ExceptionType, ...)
RADAR_SOFT_CHECK(condition, ExceptionType, ...)
RADAR_NOT_IMPLEMENTED
```

Use `RADAR_HARD_CHECK` for invariants that must be checked in every build.

Use `RADAR_SOFT_CHECK` only for intentionally debug-only checks. Its expression and exception arguments are not evaluated in release builds.

Do not use a debug-only check for validating untrusted input or enforcing behavior required in release builds.

## Math
Before adding small numerical helpers, inspect:
```cpp
#include <radar/mms/common/math.h>
```

Prefer existing helpers such as approximate comparisons and clamping where applicable.

In particular, use the shared floating-point comparison semantics rather than adding slightly different project-local epsilon helpers.

## Logging
Use:
```cpp
#include <radar/mms/common/log.h>
```

Prefer the common logging setup and existing spdlog integration over constructing unrelated project-local logging infrastructure.

Example usage:
```cpp
RADAR_LOG(err, "1 + 1 = {}", 1 + 1);
RADAR_LOG_TO("logger_name", info, "1 + 1 = {}", 1 + 1);
```

If project defines its own macro wrapper over `RADAR_LOG_TO`, prefer that over `RADAR_LOG`.

## Semantic versions
Use:
```cpp
#include <radar/mms/common/version.h>
#include <radar/mms/common/version_range.h>
```

Prefer:
```cpp
Version
VersionRange
```

over another local semantic-version parser or string comparison implementation.
Use their parsing, comparison, formatting, hashing, and optional Qt integration.

## Functional and threading helpers
Before adding another small utility, inspect:
```cpp
#include <radar/mms/common/functional.h>
#include <radar/mms/common/threading.h>
```

RCommon provides reusable helpers including scope-defer, lazy evaluation, overload composition, mutex/rwlock helpers, and spin-wait support.

Use them when they simplify the code and their semantics match. Do not replace clearer standard-library code solely to increase RCommon usage.

## JSON
RCommon provides JSON adapters under:
```text
radar/mms/common/json/
```

When serializing RCommon value types, use the existing adapter rather than writing another serializer for the same type.

Link `radar::mms::common::json` when these adapters are consumed.

## Selection rule
When implementing a new type or helper, ask in this order:

1. Does the C++ standard library already provide the concept?
2. Does RCommon provide the Radar-specific or cross-project version of it?
3. Does another existing project dependency already own the concept?
4. Only then add a project-local implementation.

Prefer RCommon especially when sharing the same concept across repositories.

Do not duplicate an RCommon abstraction under a project-specific name without a concrete semantic reason.