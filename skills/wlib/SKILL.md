---
name: wlib
description: How to use WLib, the C++23 utility library of whs31 (headers `<w/common/...>`, namespace `w`, CMake targets `w::lib` and `w::lib::glaze`). Use when you write, change or review C++ code in a project that depends on wlib, or when C++ code needs physical quantities and units, vectors, matrices, quaternions, geodetic coordinates, colors, UUIDs, semantic versions, IP addresses, spdlog loggers, reproducible random numbers, scope guards, mutex guards, per-user application directories, or glaze serialization of these types.
---

# WLib
WLib is a general-purpose C++23 library. It does not use Qt. For Qt and QML, also use the `wlib-qt` skill.

## Rules
- If WLib has a type or a function for a task, use it. Do not write a new vector, quantity, UUID, version, address or scope guard type.
- Keep a physical value in its quantity type. Do not keep it in a `float` with the unit in the variable name.
- Before you use an API for the first time, read its header. Each header has Doxygen comments with examples.
- Do not use names that start with `_`, or names in a `detail` or `_*_impl` namespace. They are not public API.
- Do not forward-declare WLib types. Most of them are aliases of templates, so `class Angle;` does not compile. Include the header.

To find the headers, search `compile_commands.json` for the `-isystem` path that contains `wlib`. The headers are in `w/common/` below that path.

## Dependency
The Conan package is `wlib`, with the user and channel `whs31/dev`. The CMake package is `WLib`.

```python
def requirements(self):
  self.requires("wlib/[>=8 <9]@whs31/dev")
```

Keep the version range that the project has. Do not change the major version unless the task tells you to.

| Target | Contents | Condition |
|---|---|---|
| `w::lib` | The library | Always |
| `w::lib::glaze` | Header-only glaze adapters in `<w/common/glaze/...>` | Conan option `with_glaze=True` (default `False`) |

```python
default_options = {"wlib/*:with_glaze": True}
```

```cmake
find_package(WLib REQUIRED COMPONENTS glaze)  # without glaze: find_package(WLib REQUIRED)
target_link_libraries(my_target PRIVATE w::lib w::lib::glaze)
```

Each target requires C++23 of its consumers.

## Names
- `common` is an inline namespace, so `w::Angle` and `w::common::Angle` are the same type. Write `w::`.
- Include the header of each type that you use. The header name is the type name in snake_case: `<w/common/angular_velocity.h>`.
- `<w/common/all.h>` includes the full library. Include the specific headers.
- All literals are in `w::literals`. Write `using namespace w::literals;` in a function or in a source file, not in a header.
- The macros start with `W_`.

## Precision
- Each quantity and linear algebra type is a template over its scalar, for example `w::BasicAngle<T>` and `w::BasicVector3<T>`.
- The short name is the `float` type: `w::Angle` is `w::BasicAngle<float>`. `w::Real` is `float`. The literals make `float` values.
- Write the `double` type in full: `w::BasicVector3<double>`.
- A `float` value converts to `double` implicitly. A `double` value converts to `float` only explicitly: `w::Angle(precise)`.
- An operation on two precisions computes in `double` and returns the `double` type.
- Use `float` for game, GPU and physics engine data. Use `double` for values that need more than 7 significant digits, for example simulation positions far from the origin.
- The geodetic types always use `double`. The constants in `<w/common/constants.h>` are `double`. Cast them to the precision of the calculation.

## Physical quantities
`Acceleration`, `Angle`, `AngularAcceleration`, `AngularVelocity`, `Capacitance`, `Charge`, `Conductance`, `Current`, `Density`, `Distance`, `Energy`, `Force`, `Frequency`, `MagneticFluxDensity`, `Mass`, `MomentOfInertia`, `Power`, `Pressure`, `Resistance`, `Temperature`, `Torque`, `Velocity`, `Voltage`.

A quantity keeps one value in its SI unit. Make it with a factory or a literal. Read it with the accessor of the unit that you need.

```cpp
using namespace w::literals;

auto const range = w::Distance::from_kilometers(2.5);
auto const climb = 3_mps;               // w::Velocity
auto const heading = 90_deg;            // w::Angle
auto const time = range / climb;        // std::chrono::duration<float>, in seconds
auto const metres = range.as_m();
auto const air = w::Temperature::from_celsius(15.0).as_kelvin();
```

- The explicit constructor takes the SI value. Use a named factory, because it shows the unit.
- `static_cast<double>(quantity)` gives the SI value. `magnitude()` is not public.
- Two values of one quantity type add, subtract and compare. A quantity multiplies and divides by a number. `a / b` of one type gives a number.
- Different quantity types do not mix. Only the kinematic relations exist, with `std::chrono::duration`:
  - `Distance / duration -> Velocity`, `Velocity * duration -> Distance`, `Distance / Velocity -> duration`;
  - `Velocity / duration -> Acceleration`, `Acceleration * duration -> Velocity`;
  - the same relations for `Angle`, `AngularVelocity` and `AngularAcceleration`.
- Compare with a tolerance of the same type: `w::approx_eq(a, b, 1_cm)`.
- Each quantity has `to_string()`, `fmt` and `std::ostream` output, and `std::hash`.
- An `Angle` has `positive()` (`[0, 2π)`), `signed_()` (`[-π, π]`), `angle_to(other)` (the shortest signed turn), `lerp(other, t)` (along the shortest path), `sin()`, `cos()` and `tan()`.
- To add a quantity type, derive it from `w::QuantityBase`. `<w/common/quantity.h>` has an example. The concept `w::Quantity<T>` accepts each quantity.

## Linear algebra and geometry
`Vector2`, `Vector3`, `Vector4`, `Matrix3x3`, `Matrix4x4`, `Quaternion`, `EulerAngles`, `Transform`, `AABB`, `Sphere`, `Plane`, `Ray`, `Frustum`, `Rect`, `Recti`. These types are header-only. `<w/common/conventions.h>` documents the conventions below and checks them with `static_assert`.

- A matrix keeps its rows and multiplies column vectors: `m * v`.
- `a * b` applies `b` first: `projection * view * model * point`.
- The translation is in the last column. Make matrices with `from_rows()`, `from_columns()`, `from_translation()` and `from_scale()`.
- `transform_point()` applies the translation. `transform_direction()` does not apply it.
- A quaternion keeps `w, x, y, z` (Hamilton convention, active rotation, right-handed). The constructor takes `w` first: `Quaternion(w, x, y, z)`.
- Jolt, HLSL and glm keep `x, y, z, w`. At these boundaries, use `Quaternion::from_xyzw()` and `to_xyzw()`.
- No conversion has an unspecified Euler order. `from_yaw_pitch_roll()` and `to_yaw_pitch_roll()` use the aerospace ZYX order of `EulerAngles`. For a different order, use `from_euler<w::EulerOrder::YXZ>()` and `to_euler<...>()`.
- The types do not know their frame. Put the frame in the variable name: `velocity_ned`, `position_world`. The game uses a right-handed frame with y up. helisim uses NED frames of a `LocalTangentPlane`.
- Functions that need a handedness take it as an argument: `look_at_rh()`, `look_at_lh()`, `perspective()`, `orthographic()`, `Quaternion::look_rotation()`.
- Projections give depth in `[0, 1]`. `w::ClipDepth::Reversed` puts the near plane at 1. `perspective()` accepts an infinite far plane.
- The shaders use Diligent's `SHADER_COMPILE_FLAG_PACK_MATRIX_ROW_MAJOR` and `mul(M, v)`. Copy a matrix into a constant buffer without a transpose. Do not add `row_major` or `column_major` qualifiers.
- `Vector3` is 12 bytes, with no SIMD padding. Code that reflects members by offset needs this.

If an operation can fail:
- `Vector3::normalized()` of a zero vector returns the zero vector. `try_normalized()` returns `std::nullopt`.
- `Quaternion::normalized()` and `Matrix4x4::inverse()` throw `w::NumericError`. If failure is possible, use `try_normalized()` and `try_inverse()`.

Colors:
- `Rgba` is linear `float`. `Rgba8` is sRGB bytes. Do color calculations, for example blends, in `Rgba`.
- The compiler checks `"#e4a672"_rgba8`. `Rgba8::from_hex(text)` returns `std::optional`.
- Convert with `Rgba8::to_linear()` and `Rgba::to_srgb8()`. For HSV, use `Hsv`, `from_hsv()` and `to_hsv()`.

## Geodesy
`w::Latitude` and `w::Longitude` keep `double` degrees. They are positions, not angles:
- `Latitude ± Angle -> Latitude` and `Latitude - Latitude -> Angle`.
- A longitude difference is the shortest turn across the antimeridian.
- A `Longitude` always wraps into `[-180°, 180°)`.
- A `Latitude` can go past a pole. `clamped()`, `folded()` and `GeodeticPosition::normalized()` bring it back.
- The literals are `59.771_north`, `_south`, `_east` and `_west`, and the signed `_lat` and `_lon`.
- `parse()` reads `59.771°N`, `59°46'15.60"N`, `33 51 36 S` and `50north`. It returns `std::optional`. `to_dms_string()` writes degrees, minutes and seconds.
- They also give the hemisphere, the UTM zone and band, the nautical time zone and the WGS84 radii of curvature.

```cpp
using namespace w::literals;

auto constexpr pulkovo = w::GeodeticPosition {
  .latitude = 59.771_north,
  .longitude = 30.326_east,
  .altitude_msl = 70_meters,  // w::Distance
};

auto const plane = w::LocalTangentPlane(pulkovo);
auto const ned = plane.ned_from_lla(target);  // w::Vector3: metres north, east and down
auto const back = plane.lla_from_ned({10'000.0, 0.0, -150.0});
```

- `LocalTangentPlane` is a flat NED frame at an origin. It keeps the frame in `double` and gives positions as `float` `Vector3`. Use it for local simulation frames.
- `ned_from_lla()` throws `w::NumericError` for a position a quarter of the globe or more from the origin.
- `w::web_mercator` has the XYZ tiles of slippy maps: `project(latitude, longitude, zoom)`, `latitude_at()`, `longitude_at()` and `tiles(zoom)`.
- `w::wgs84` has the ellipsoid constants in metres.

## Identifiers and addresses
| Type | Make | Invalid text |
|---|---|---|
| `Uuid` | `Uuid::new_v4()`, `Uuid::now_v7()`, `Uuid::new_v5()`, `Uuid(text)`, `"..."_uuid` | throws `UuidError` |
| `Version` | `Version::parse(text)`, `"1.2.3-rc.1"_version` | throws `VersionError` |
| `VersionRange` | `VersionRange::parse(text)` | throws `VersionError` |
| `Ipv4Addr`, `Ipv6Addr` | `parse(text)`, `"10.0.0.1"_ipv4`, `"::1"_ipv6` | throws `IpError` |
| `SockAddr`, `SockAddrV4`, `SockAddrV6` | `parse("10.0.0.1:80")`, `parse("[::1]:443")` | throws `IpError` |

- Each type has `to_string()`, `fmt` output, comparison and `std::hash`.
- `VersionRange` reads an npm-like syntax: `1.2.3`, `^1.2.3`, `~1.2.3`, `>=1.2.3 <2.0.0`, `1.x`, `1.2 - 2.3`, `1.2.3 || >=2.0.0` and `*`. Test a version with `range.contains(version)`.
- Do not parse these values by hand or with `std::regex`.

## Errors and checks
- Report failures with exceptions. Each WLib exception derives from `w::Error`, whose constructor takes a `fmt` format string: `throw w::InvalidArgumentError("bound {} is not positive", bound);`
- Use `w::InvalidArgumentError` for a broken precondition. Use `w::NumericError` for a calculation without a finite result. Derive project exceptions from `w::Error`.
- `W_HARD_CHECK(condition, ErrorType, "format {}", args...)` throws in all builds.
- `W_SOFT_CHECK` has the same arguments. It throws only in debug builds (`W_DEBUG`, no `NDEBUG`). Release builds do not evaluate its arguments, so do not put side effects in them.
- `W_NOT_IMPLEMENTED;` throws `w::NotImplementedError` with the source location.
- `w::fatal("format {}", args...)` flushes the loggers, writes `fatal: <message>` to stderr and calls `std::exit(EXIT_FAILURE)`. Destructors do not run. Use it only at the top level of an application, for example around the body of `main()`. A library throws.
- `w::describe(exception_ptr)` returns the message of a captured exception.

## Logs
WLib configures spdlog.

```cpp
auto const logger = w::create_logger({
  .name = "app",
  .is_default = true,
  .target = w::LogTarget::Stdout | w::LogTarget::File,
  .pattern = std::string(w::aligned_log_pattern),
  .filename = "app.log",
  .folder = w::application_directory_path(w::ApplicationDirectory::State, "Vendor", "product"),
});

W_LOG(info, "loaded {} vehicles", count);        // the default logger
W_LOG_TO("app.net", warn, "retry {}", attempt);  // a named logger
w::shutdown_log();                               // before the application stops
```

- Write the `LoggerConfig` fields in their declaration order, or the designated initializers do not compile.
- The level tokens are `trace`, `debug`, `info`, `warn`, `err` and `critical`.
- The default level comes from the `LOG_LEVEL` environment variable, through `w::get_env_log_level()`.
- If no logger has the name, `W_LOG_TO` does nothing and does not evaluate its arguments.
- To remove lower levels at compile time, define `W_LOG_ACTIVE_LEVEL`, for example as `W_LOG_LEVEL_INFO`.
- To show messages in a UI, use `on_warn_or_error`, or `w::attach_log_sink()` and `w::detach_log_sink()` at run time.

## Reproducible random numbers
`<w/common/random.h>` and `<w/common/noise.h>` give the same values for the same seed on each platform and compiler.

```cpp
auto stream = w::RandomStream::derive(run_seed, "vt30-1", "imu.gyro");
auto const u = stream.uniform();          // [0, 1)
auto const k = stream.below(6);           // [0, 6)
auto const g = stream.normal(0.0, 0.02);  // mean, standard deviation
```

- Use one stream for each independent noise source. Then a new draw in one source does not change the draws of the others.
- Keep the stream in the object that uses it. It is 32 trivially copyable bytes, so it goes into snapshots. Do not keep a stream in a global variable.
- A copy of a stream gives the same draws as the original.
- The names in `derive()` must be stable. Do not use pointers or the creation order.
- Do not write `state` by hand. The all-zero state never changes.
- For values that must replay, do not use `<random>` distributions, `std::log` or `std::exp`. Use `RandomStream`, `w::portable_log()` and `w::portable_exp()`.
- If your own calculation must replay, compile it with `-ffp-contract=off` (MSVC: `/fp:precise`). WLib compiles its random sources with these flags.
- The noise processes are `w::WhiteNoise::from_psd()`, `w::GaussMarkov::with()`, `w::RandomWalk::with()` and `w::turn_on_bias()`. Each `sample(stream)` draws exactly once.

## Other utilities
The headers are below `<w/common/...>`.

| Task | Use | Header |
|---|---|---|
| Run code at scope exit | `auto guard = w::defer([&]() noexcept { ... });`, `guard.dismiss()`. The callback must be `noexcept` and return `void` | `functional/defer.h` |
| Compute a value on first use | `w::lazy(callable)` | `functional/lazy.h` |
| Select one overload of a function | `w::overload<int>(&parse)`, `w::const_overload<...>`, `w::non_const_overload<...>` | `functional/overload.h` |
| Non-owning pointer | `w::observer_ptr<T>`, `w::make_observer(p)` | `memory/observer_ptr.h` |
| Mark an owning raw pointer | `w::Owner<T*>` | `memory/owner.h` |
| C cleanup function in `std::unique_ptr` | `w::Deleter<int, std::FILE*, std::fclose>` | `memory/deleter.h` |
| Pimpl with `const` propagation | `w::Pimpl<T>`, `w::SharedPimpl<T>`, `DECLARE_PRIVATE(Class)` | `pimpl.h` |
| Value under a mutex | `w::MutexGuard(mutex, value)`, `w::RwLockReadGuard`, `w::RwLockWriteGuard` | `threading.h` |
| Spin wait | `w::yield()` | `threading/pause.h` |
| 1-D lookup table without allocation | `w::Curve<X, Y, MaxPoints>`, `interpolate(x)` | `curve.h` |
| Cache of pure function results, one for each thread | `w::Memo<Words, Value, Size>`, keys from `w::bits_of()` | `memo.h` |
| Tolerant comparison, clamp, interpolation, wrap | `w::approx_eq()`, `w::is_null()`, `w::clamp()`, `w::lerp()`, `w::normalize()`, `w::rem_euclid()` | `math.h` |
| Per-user directories | `w::application_directory_path(w::ApplicationDirectory::Config, "Vendor", "product")` | `paths.h` |
| Home and executable paths | `w::user_home_path()`, `w::application_file_path()`, `w::application_dir_path()` | `paths.h` |
| One process at a time | `w::LockFile(path)`, `try_lock()`, `lock()` | `lockfile.h` |
| Dependency order of UUID nodes | `w::GraphResolver`, `enqueue()`, `resolve(&has_cycle)` | `graph_resolver.h` |
| ASCII and hexadecimal characters | `w::is_digit()`, `w::is_hex()`, `w::to_lower()` and others | `char_utils.h` |
| Pointer and number concepts | `w::Number`, `w::Pointer`, `w::SmartPointer`, `w::PointerLike` | `concepts.h` |

- `application_directory_path()` does not create the directory. It throws `w::PathError` if a base directory is not available. `w::ApplicationLayout::Portable` puts all files below `data/` next to the executable.
- `GraphResolver::resolve()` puts each dependency before its dependents. The order of unrelated nodes is not stable.

## glaze
Set `with_glaze=True` and link `w::lib::glaze`. Include the adapter header of each type, with the same name below `glaze/`. Also include the glaze header of the format.

```cpp
#include <glaze/toml.hpp>
#include <w/common/glaze/chrono.h>    // std::chrono::duration
#include <w/common/glaze/distance.h>
#include <w/common/glaze/read.h>

struct Link
{
  w::Distance range;
  std::chrono::milliseconds timeout;
  std::optional<std::string> name;
};

auto const link = w::read_glaze_file<Link, w::strict_toml>(path);
```

- `w::read_glaze<T, Opts>(text)` and `w::read_glaze_file<T, Opts>(path)` throw `w::DocumentError`. The message shows the incorrect line and the file name.
- `w::strict_json` and `w::strict_toml` refuse unknown keys and missing keys. A `std::optional` member can be missing.
- If a type refuses a value, for example an invalid UUID, the read fails with `glz::error_code::constraint_violated`.

| Type | Document form |
|---|---|
| Quantity | Writes `{"<SI unit>": value}`, for example `{"meters": 2500}`. Reads each unit key, for example `{"kilometers": 2.5}`. JSON and TOML also read the literal suffix without its underscore: `"2.5km"`, `"52ah"` |
| `Angle` | Writes `{"angle": 90, "units": "degrees"}`. Also reads `{"radians": 1.5}` and `"90deg"` |
| `std::chrono::duration` | `"250ms"`, `"1.5s"`, `"2min"`, `{"seconds": 0.25}` |
| `Vector2`, `Vector3`, `Vector4` | `[x, y, z]` |
| `Quaternion` | `[w, x, y, z]` |
| `Matrix3x3`, `Matrix4x4` | An array of rows |
| `Plane` | `[a, b, c, d]`: the normal and the distance |
| `Transform` | `{"position": [x, y, z], "rotation": [w, x, y, z], "scale": [x, y, z]}` |
| `EulerAngles` | `{"roll": angle, "pitch": angle, "yaw": angle}` |
| `AABB`, `Sphere`, `Ray` | `{"min", "max"}`, `{"center", "radius"}`, `{"origin", "direction"}` |
| `Rect` | `[x, y, width, height]` |
| `Rgba` | `[r, g, b, a]`, linear |
| `Rgba8` | `"#e4a672"` |
| `Latitude`, `Longitude` | Signed degrees: `59.771`. Also reads each text form of `parse()` |
| `Uuid`, `Ipv4Addr`, `Ipv6Addr` | The canonical string |
| `SockAddrV4` | `{"ip": "192.168.0.1", "port": 8080}` |
| `Curve` | `{"arguments": [...], "values": [...]}` |
| `RandomStream` | `{"state": [s0, s1, s2, s3]}` |
