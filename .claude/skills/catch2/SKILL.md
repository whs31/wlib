---
name: catch2
description: Catch2 testing conventions for C++ repositories. Use when adding, changing, organizing, or reviewing C++ tests, Catch2 targets, regression tests, fixtures, or test data.
---

# Catch2 testing conventions
Use Catch2 for C++ unit, integration, and regression tests unless the repository explicitly uses another framework.

Tests should verify externally observable behavior rather than implementation details.

## Test target
A project has one Catch2 test executable:

```cmake
${PROJECT_NAME}_test
```

Do not create separate test executables for individual subsystems, source files, features, or bug fixes.

Add test sources to the existing `${PROJECT_NAME}_test` target.

A typical structure is:

```cmake
add_executable(${PROJECT_NAME}_test
    test/foo_test.cc
    test/bar_test.cc
)

target_link_libraries(${PROJECT_NAME}_test
    PRIVATE
        ${PROJECT_NAME}
        Catch2::Catch2WithMain
)
```

Use `Catch2::Catch2WithMain` when no custom test application setup is needed.
Use `Catch2::Catch2` with one project-wide `test_main.cc` when initialization
such as `QGuiApplication` is required.

When the repository uses CTest, register the Catch2 executable through the project's existing Catch2/CTest integration.

Prefer `catch_discover_tests` when that is already the repository convention. If not, feel free to introduce it and migrate the tests.

Do not introduce a second test runner or a custom `main()` without a concrete requirement.

## Test cases
Name tests descriptively.
A test name should describe the behavior being verified rather than repeat a function or class name.

Prefer:
```cpp
TEST_CASE("Plan rejects waypoints with invalid coordinates", "[plan]")
```

over:
```cpp
TEST_CASE("Plan test", "[plan]")
```

Use `SECTION` when several closely related scenarios naturally share setup.
Do not combine unrelated behaviors into a single large test merely to reduce the number of `TEST_CASE`s.

## Tags
Tag tests with the subsystem or behavior that owns them.

Examples:

```text
[compiler]
[plan]
[serialization]
[qml]
[route]
[network]
```

Use the owning area, not generic tags such as `[unit]` everywhere.

Multiple tags are appropriate when a test genuinely crosses boundaries:

```cpp
TEST_CASE(
    "Serialized survey preserves generated route",
    "[plan][serialization]"
)
```

Keep the tag vocabulary consistent with the existing repository.

## Regression tests
When fixing a bug, add a focused Catch2 regression test where practical. Regression tests should belong into `regression` subdirectory inside `tests` and have a `regression` tag.

The regression test should:
1. reproduce the observable failure mode,
2. fail without the fix,
3. pass with the fix,
4. remain useful after the implementation changes.

Test the behavior that was broken rather than encoding the internal implementation of the fix.

Avoid oversized regression tests that reproduce an entire application when a smaller observable scenario demonstrates the bug.

## Observable behavior
Test public and externally observable behavior.

Cover relevant edge states, not only the successful path.

Depending on the subsystem, consider:
- valid input
- invalid input
- empty input or empty state
- missing data
- stale state
- disconnected state
- malformed serialized data
- boundary values
- repeated operations
- lifecycle changes
- localized UI state
- asynchronous completion or cancellation
- recovery after failure

Only add cases that are meaningful for the behavior under test. Do not mechanically produce every category for every function.

Prefer assertions about outputs, state transitions, emitted public signals, errors, serialized representations, or other observable effects.

Avoid assertions against private implementation details merely because they are easy to access.

## QML and Lua fixtures

Never embed QML or Lua source code in C++ test strings.
All QML and Lua test inputs, including small snippets, belong in `.qml` and
`.lua` files respectively.

Do not write:

```cpp
auto const qml = R"(
    import QtQuick
    Item {
        // ...
    }
)";
```

or:

```cpp
auto const lua = R"(
    function foo()
        return 42
    end
)";
```

Put source fixtures in their native file types instead:

```text
tests/
├── compiler_test.cc
├── qml_test.cc
├── qml/
│   ├── simple_component.qml
│   ├── invalid_component.qml
├── lua/
│   ├── simple_script.lua
│   └── invalid_script.lua
```

Load the fixture from the test.

This keeps syntax highlighting, tooling, formatting, and test inputs representative of real files.

Tiny isolated string values that merely happen to contain fragments of syntax are acceptable when the string itself is the input being tested. Full programs, components, scripts, documents, or realistic fixtures belong in separate files.

## Test data
Use a `data` directory for golden/reference data when needed.

Examples include:
- JSON
- protobuf fixtures
- serialized plans
- malformed inputs
- binary fixtures
- golden outputs
- images
- configuration files

Prefer:

```text
tests/
├── serialization_test.cc
└── data/
    ├── plan_v1.json
    ├── plan_invalid.json
    └── expected_plan.json
```

Do not encode large golden data directly into C++ source.
Keep fixtures minimal and focused on the behavior being tested.
Name fixtures according to the scenario they represent.

## Golden tests

Use golden/reference files when structural output is large enough that field-by-field assertions would obscure the purpose of the test.

Golden files must be:
- deterministic,
- reviewable,
- checked into the repository,
- stored under `data`,
- updated intentionally.

Do not automatically overwrite expected golden data as part of a normal test run.

A changed golden file should appear as an ordinary source-control diff that can be reviewed.

## Assertions
Use the weakest assertion that communicates the intended requirement correctly.

Use `REQUIRE` when subsequent assertions cannot meaningfully continue after failure.

Use `CHECK` when independent assertions can still provide useful diagnostic information.

Use Catch2 matchers when they clarify intent.

For expected failures, verify the meaningful error behavior rather than merely asserting that "something throws".

Prefer:

```cpp
REQUIRE_THROWS_AS(parse(input), ParseError);
```

or checking the relevant error contents when they are part of the contract.

Avoid tests whose correctness depends on incidental wording unless that wording is explicitly part of the public behavior.

## Determinism

Tests must be deterministic.

Avoid dependencies on:

- wall-clock timing
- execution order
- global mutable state
- machine-specific paths
- network availability
- locale defaults
- random values without a controlled seed

When time, locale, filesystem state, or connectivity is part of the behavior, control it explicitly where practical.

Tests involving localized UI should explicitly select the relevant locale rather than rely on the developer machine's locale.

## Test organization
Keep tests close to the conceptual subsystem they cover while retaining the single `${PROJECT_NAME}_test` executable.

A typical layout is:

```text
tests/
├── compiler_test.cc
├── plan_test.cc
├── serialization_test.cc
├── qml_test.cc
├── qml/
├── lua/
└── data/
    ├── serialization/
    └── golden/
```

Subdirectories inside `data` are useful when the fixture set becomes non-trivial.
Follow an established repository layout when one already exists.

## Selection rule
When changing behavior:
1. Identify the observable contract being changed or fixed.
2. Look for an existing test covering that contract.
3. Extend the existing test when the scenario naturally belongs there.
4. Otherwise add a focused, descriptively named Catch2 test.
5. Put substantial external-language or golden input into `data`.
6. Tag the test with the owning subsystem.
7. Keep all C++ tests in the single `${PROJECT_NAME}_test` target.