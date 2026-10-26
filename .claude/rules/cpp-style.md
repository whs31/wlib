---
paths:
  - "**/*.{c,cc,cpp,cxx,h,hh,hpp,hxx}"
---

# C++ coding conventions
Follow the repository's configured C++ language standard. Do not change the language standard as part of an unrelated change.

Prefer narrow changes consistent with the surrounding architecture.

## Formatting and expressions
- Do not fight the repository formatter.
- Prefer the alternative operator spellings `and`, `or`, and `not`.
- Always qualify non-static data members and member-function calls with `this->`, including inside lambdas.
- Do not use `this->` for static members, static functions, free functions, or constructor initializer names.

## Naming
Unless a repository defines a more specific convention:
- namespaces: `lower_case`
- functions and methods: `lower_case`
- local variables and parameters: `lower_case`
- local compile-time constants: `lower_case`
- classes, structs, aliases, and other types: `CamelCase`
- scoped-enum values: `CamelCase`
- private and protected data members: `m_lower_case`

Qt/QML-facing APIs follow Qt conventions where required:
- QML properties: `lowerCamelCase`
- signals: `lowerCamelCase`
- invokable methods: `lowerCamelCase`

Their underlying C++ accessors may remain `lower_case`.

Lua-facing APIs (if any) follow Lua conventions where required:
- Lua properties: `lower_case`
- Lua methods: `lower_case`
- Lua classes: `CamelCase`

Files:
- Header files: `lower_case` + `.h`
- Source files: `lower_case` + `.cc`

## Types and ownership
Use modern, explicit types and ownership semantics.
- Declare every local variable with `auto`, adding `const`, references, or pointers as needed; do not spell out local variable types.
- Write function return types before the function name; do not use trailing return types.
- Make single-argument constructors `explicit`.
- Mark overrides with `override`.
- Use `final` for intentionally closed extension points.
- Prefer `enum class` with an intentional underlying type when appropriate.
- Use `[[nodiscard]]` when ignoring a result is likely to be a bug.
- Use `noexcept` for operations that are intentionally non-throwing.
- Use `[[maybe_unused]]` for intentionally unused entities (like lock guards).
- Use `nullptr`.
- Use C++ casts instead of C-style casts.
- Prefer RAII.
- Prefer values and references where ownership is not involved.
- Use `std::unique_ptr` for sole ownership.
- Use Qt parenting for owned `QObject`s.
- Use `QPointer` for asynchronous observation of destructible `QObject`s.
- When storing a movable argument, prefer pass-by-value followed by `std::move`.
- Use `const` wherever practical, including implementation locals and by-value parameters in `.cc` files.

Prefer typed domain values over strings, primitive numbers, maps, and `QVariant`.

Keep `QVariant` at genuinely dynamic API boundaries and validate or convert it immediately.

Before introducing a common infrastructure type or helper in the project, check whether WLib already provides the concept. In particular, prefer WLib for identifiers, physical quantities, geometry, errors, Pimpl, and non-owning pointers when its semantics match.

## Headers and includes
Keep headers self-contained and expose the smallest practical interface.

Prefer forward declarations when a complete type is unnecessary.

Use `QT_FORWARD_DECLARE_CLASS` for Qt classes where appropriate.

A `.cc` file starts with its corresponding public header.

After the corresponding header, group includes in this order:

1. C headers like `<cstdint>`
2. C++ standard library
3. Qt
4. external 3rd party libraries
5. external 1st party libraries
6. public local headers 
7. private local headers

Prefer to place private local headers into the `src/private` directory and include it through `#include <private/....h>`

Preserve deliberate local ordering when required.

Use `<...>` include syntax.

Use `#pragma once`.

Keep source-only implementation details in an unnamed namespace.

Prefer specific headers over umbrella headers.

## Comments
Comments explain why, constraints, or non-obvious behavior; do not narrate the code. Preferred comment format:
```cpp
// note: [<author> : <yyyy-mm-dd>]
// <actual comment in lowercase>
```

Do not narrate code that is already self-explanatory.

Every TODO must describe an actionable missing piece. If implementation is blocked, state the prerequisite that is missing.

## Compatibility

Keep platform-independent code portable across Linux, MinGW, and MSVC unless a repository explicitly supports a narrower target.

Isolate unavoidable platform-specific code and guard it explicitly.

Repository-specific API, ABI, wire-format, persistence-format, and compatibility guarantees belong to that repository's `AGENTS.md` and take precedence over these general conventions.
