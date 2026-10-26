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
identifiers, networking, logging, deterministic randomness and more, with optional Qt and glaze
integration. It is created and maintained by **whs31**.

The library lives in the `w` namespace with an inline `common` namespace, so `w::Angle` and
`w::common::Angle` name the same type. Its headers are included as `<w/common/...>` and its macros
are prefixed with `W_` (e.g. `W_LOG`, `W_COMMON_API`).

#### Dependencies

- <img src="./docs/images/logos/fmt.png" alt="" height="16px" width="16px"/>[**fmt**](https://github.com/fmtlib/fmt) - modern formatting library (**MIT**)
- <img src="./docs/images/logos/spdlog.jpg" alt="" height="16px" width="16px"/>[**spdlog**](https://github.com/gabime/spdlog) - fast C++ logging library (**MIT**)
- [**glaze**](https://github.com/stephenberry/glaze) - optional serialization adapters exposed by
  the `w::lib::glaze` target, enabled with `with_glaze=True` in Conan or `WLIB_WITH_GLAZE=ON` in
  CMake (**MIT**)
- [**Qt 6**](https://www.qt.io/) - optional Core, Gui, Network, and Positioning integration enabled
  with `with_qt=True` in Conan or `WLIB_WITH_QT=ON` in CMake (**LGPL/GPL/commercial**)
- <img src="./docs/images/logos/catch2.png" alt="" height="16px" width="16px"/>[**catch2**](https://github.com/catchorg/Catch2) - c++ testing
  framework (**BSL-1.0**)

#### CMake targets

The Conan package `wlib` provides the CMake package `WLib`:

- `w::lib` provides the core library without any serialization dependency.
- `w::lib::glaze` adds the header-only glaze adapters in `<w/common/glaze/...>`. It exists only
  when the package is built with `with_glaze=True` (default `False`).

```cmake
find_package(WLib REQUIRED COMPONENTS glaze)
target_link_libraries(my_target PRIVATE w::lib w::lib::glaze)
```

With Qt enabled, the glaze adapters also cover Qt Core, Gui, and Positioning types. They read
physical quantities in any unit the quantity has a factory for, e.g. `{"ampere_hours": 52}` or
`{"celsius": 15}`, and, in JSON and TOML, as a string spelled like the C++ literal without its
underscore, e.g. `"52ah"` or `"400hz"`. That suits hand-written TOML configuration; they still
write the SI unit.

Enable the adapters through the consumer's Conan options, e.g. in its `conanfile.py`:

```python
default_options = {"wlib/*:with_glaze": True}
```

or on the command line with `-o "wlib/*:with_glaze=True"`.

#### Precision

The physical quantities (`Distance`, `Angle`, `Mass`, `Velocity`, ...) and the linear algebra
types (`Vector2`, `Vector3`, `Point2`, `Point3`, `Quaternion`, `Matrix3x3`, `EulerAngles`) store
`w::Real`, which the Conan option `precision` selects:

| `precision`       | `w::Real` | CMake                   | Macro in `<w/common/config.h>` |
|-------------------|-----------|-------------------------|--------------------------------|
| `float` (default) | `float`   | `WLIB_PRECISION=float`  | —                              |
| `double`          | `double`  | `WLIB_PRECISION=double` | `W_COMMON_DOUBLE_PRECISION`    |

```python
default_options = {"wlib/*:precision": "double"}
```

The choice is part of the ABI, and an installed package records it in `WLib_PRECISION` after
`find_package(WLib)`.

Positions on the Earth need more than a `float`, which resolves a longitude to about a metre and a
half, so geodesy stays in `double` either way. `Latitude` and `Longitude` are dedicated coordinate
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

#### Licensing

*WLib* library is distributed under the MIT license. See [LICENSE](./LICENSE) for details.
