## WLib

<h3 align="center">
    <img src="./docs/images/icon.png" alt=""/><br/>
    <img src="./docs/images/transparent.png" height="30" width="0px"/>
    Common C++23 utility library
    <img src="./docs/images/transparent.png" height="30" width="0px"/><br/>
    <img src="./docs/images/transparent.png" height="30" width="0px"/><br/>
    <img src="./docs/images/badges/cxx.svg" alt=""/><img src="./docs/images/badges/cmake.svg" alt=""/>
</h3>

*WLib* is a general-purpose C++23 utility library: physical quantities, geometry, geodesy,
identifiers, networking, logging, deterministic randomness and more, with optional glaze
integration. The Qt and QML integration is the separate package
[wlib_qt](https://git.whs31.com/shared/support/wlib-qt). It is created and maintained by **whs31**.

The library lives in the `w` namespace with an inline `common` namespace, so `w::Angle` and
`w::common::Angle` name the same type. Its headers are included as `<w/common/...>` and its macros
are prefixed with `W_` (e.g. `W_LOG`, `W_COMMON_API`).

#### Dependencies

- <img src="./docs/images/logos/fmt.png" alt="" height="16px" width="16px"/>[**fmt**](https://github.com/fmtlib/fmt) - modern formatting library (**MIT**)
- <img src="./docs/images/logos/spdlog.jpg" alt="" height="16px" width="16px"/>[**spdlog**](https://github.com/gabime/spdlog) - fast C++ logging library (**MIT**)
- [**glaze**](https://github.com/stephenberry/glaze) - optional serialization adapters exposed by
  the `w::lib::glaze` target, enabled with `with_glaze=True` in Conan or `WLIB_WITH_GLAZE=ON` in
  CMake (**MIT**)
- <img src="./docs/images/logos/catch2.png" alt="" height="16px" width="16px"/>[**catch2**](https://github.com/catchorg/Catch2) - c++ testing
  framework (**BSL-1.0**)
- [**glm**](https://github.com/g-truc/glm) - test-only reference that the linear algebra, the
  projections and the bounds are compared against with random inputs; never a dependency of the
  library (**MIT**)

#### CMake targets

The Conan package `wlib` provides the CMake package `WLib`:

- `w::lib` provides the core library without any serialization dependency.
- `w::lib::glaze` adds the header-only glaze adapters in `<w/common/glaze/...>`. It exists only
  when the package is built with `with_glaze=True` (default `False`).

Every target requires C++23 of its consumers. The Qt targets `w::lib::qt`, `w::lib::qml` and
`w::lib::qt::glaze` come from the `wlib_qt` package and its CMake package `WLibQt`.

```cmake
find_package(WLib REQUIRED COMPONENTS glaze)
target_link_libraries(my_target PRIVATE w::lib w::lib::glaze)
```

The glaze adapters read physical quantities in any unit the quantity has a factory for, e.g.
`{"ampere_hours": 52}` or `{"celsius": 15}`, and, in JSON and TOML, as a string spelled like the C++ literal without its
underscore, e.g. `"52ah"` or `"400hz"`. That suits hand-written TOML configuration; they still
write the SI unit.

Enable the adapters through the consumer's Conan options, e.g. in its `conanfile.py`:

```python
default_options = {"wlib/*:with_glaze": True}
```

or on the command line with `-o "wlib/*:with_glaze=True"`.

#### Numbers and linear algebra

The physical quantities and the linear algebra types are templates over their precision,
`BasicAngle<T>`, `BasicVector3<T>`, ..., for any `std::floating_point` `T`. The plain name is the
`float` instantiation, and double precision is spelled out:

```cpp
auto const heading = w::Angle::from_degrees(90.0);            // w::BasicAngle<float>
auto const precise = w::BasicVector3<double>(1e7, 0.0, 1e-3);  // keeps the millimetre

w::BasicAngle<double> const widened = heading;               // float widens implicitly
auto const narrowed = w::Angle(w::BasicAngle<double>(1.0_rad)); // double narrows only explicitly
auto const sum = w::Distance(1_meters) + w::BasicDistance<double>(2_meters); // BasicDistance<double>
```

A mixed expression computes in `double`, since only the `float` operand converts implicitly.
`w::Real` is `float`, the precision of the aliases and the literals. Colors store `float` only.
The linear algebra is header-only and covers what a game or a simulation needs:

- `Vector2`, `Vector3` (12 bytes, no SIMD padding), `Vector4`, `Matrix3x3`, `Matrix4x4`,
  `Quaternion`, `EulerAngles` and `Transform` (position, rotation, scale);
- `perspective()` and `orthographic()`, both taking the precision from their arguments, with depth in `[0, 1]`, optionally reversed and, for a
  perspective, with an infinite far plane; `look_at_rh()`, `look_at_lh()` and `snap_to_texel()`;
- `AABB`, `Sphere`, `Plane`, `Ray`, `Frustum`, `Rect` and `Recti`;
- `Rgba` (linear `float`) and `Rgba8` (sRGB bytes), with HSV and hex strings, e.g.
  `"#e4a672"_rgba8`.

Matrices are row-major and multiply column vectors, `M * v`; quaternions store `w, x, y, z`; the
math takes the handedness as a parameter. `<w/common/conventions.h>` documents the conventions,
including how shaders read the matrices, and checks them with `static_assert`.

With glaze, vectors, quaternions, matrices and planes are arrays, e.g. `[x, y, z]`, a quaternion
`[w, x, y, z]` and a matrix its rows; an `Rgba8` is a hex string, so a palette lives in TOML as
`sand = "#e4a672"`.

Positions on the Earth need more than a `float`, which resolves a longitude to about a metre and a
half, so geodesy stays in `double`. `Latitude` and `Longitude` are dedicated coordinate
types rather than angles:

- they store `double` degrees, so a coordinate keeps the digits it was written with;
- they are positions, not amounts: `Latitude ± Angle -> Latitude`, `Latitude − Latitude -> Angle`,
  and a longitude difference is the shortest turn across the antimeridian;
- a `Longitude` always wraps into `[-180°, 180°)`; a `Latitude` may run past a pole, and
  `clamped()`, `folded()` and `GeodeticPosition::normalized()` bring it back;
- they know their hemisphere, quadrant, UTM zone and band, nautical time zone, WGS84 radii of
  curvature and geocentric and parametric latitudes;
- they print and parse decimal degrees and degrees-minutes-seconds: `59.771°N`,
  `59°46'15.60"N`, `33 51 36 S`, `50north`;
- with glaze they are plain signed degrees, `{"latitude": 59.771, "longitude": 30.326}`, and a
  text document may spell them any way `parse()` reads.

`LocalTangentPlane` and the `web_mercator` helpers also compute in `double`.

```cpp
using namespace w::literals;

auto constexpr pulkovo = w::GeodeticPosition {
  .latitude = 59.771_north,   // or 59.771_lat
  .longitude = 30.326_east,   // or 30.326_lon
  .altitude_msl = 70_meters,
};

pulkovo.latitude.to_dms_string();     // 59°46'15.60"N
pulkovo.utm_zone()->to_string();      // 36V
(179_east + 3_deg).degrees();         // -178
auto const longitude = w::Longitude::parse("157°51'36\"W");  // std::optional<w::Longitude>
```

#### Migrating from the previous API

- The quantities and the linear algebra types are templates `BasicX<T>`; the old names are the
  `float` aliases. Forward declarations such as `class Angle;` become `#include`s.
- The Qt integration moved into the separate package `wlib_qt`, whose README lists the changes to
  its API. Its targets keep their names, `w::lib::qt`, `w::lib::qml` and `w::lib::qt::glaze`, and
  are found with `find_package(WLibQt)`; the option `with_qt` of `wlib` is gone.
- The physical constants in `<w/common/constants.h>` are `double`; cast them to the precision you
  compute in.
- The spelling is American: `<w/common/color.h>` and `<w/common/glaze/color.h>` replace
  `colour.h` and `glaze/colour.h`. The old headers only forward to the new ones, are deprecated and
  stay for existing users. No public type or function was spelled the British way, and the
  serialized forms are unchanged.

#### Agent skill

[`skills/wlib/SKILL.md`](skills/wlib/SKILL.md) teaches coding agents to use WLib. Copy it to
`.claude/skills/wlib/` or `.agents/skills/wlib/` of a project that depends on `wlib`.

#### Licensing

*WLib* library is distributed under the MIT license. See [LICENSE](./LICENSE) for details.
