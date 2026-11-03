## Project
WLib is a general-purpose C++23 utility library: physical quantities, linear algebra and geometry, geodesy, colors, identifiers, networking, logging, deterministic randomness and more, with optional glaze adapters. The Qt and QML integration is the separate package [wlib_qt](https://git.whs31.com/shared/support/wlib-qt).

## Shared development rules
- Follow `.claude` (or `.agents` if you are not Claude).

## Project-specific conventions
- Preserve the public WLib API unless an API break is intentional.
- Keep the library free of Qt; Qt support belongs to wlib_qt.
- Keep `skills/wlib/SKILL.md` in step with the public API.
