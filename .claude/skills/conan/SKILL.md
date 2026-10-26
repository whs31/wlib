---
name: conan
description: Standard Conan 2 workflow for C++ repositories. Use when configuring, building, installing dependencies, or verifying Conan-based projects.
---

# Conan workflow
Use Conan 2 and the user's default Conan profile.

Do not specify custom host or build profiles unless the repository or task explicitly requires them. 
Do not invent Conan options. Inspect the repository's `conanfile.py` before passing options.

## Configure dependencies
The canonical dependency installation command is:
```sh
conan install . --build=missing
```

Use the default Conan profile.
Do not add explicit profile arguments such as:
```sh
-pr:h ...
-pr:b ...
```
unless the repository explicitly requires them.

## Build
The normal build command is:
```sh
conan build . --build=missing
```

## Test option
Some repositories expose a Conan option named `test`.

This option is optional and must not be assumed to exist.  Inspect the Conan recipe first.

When the repository provides:
```python
options = {
    "test": [True, False],
}
```

or an equivalent package option, enable tests with:
```sh
conan build . -o '&:test=True' --build=missing
```

The `&:` scope intentionally applies the option to the consumer package rather than dependencies with similarly named options.

When tests are requested and the repository has a `test` option, prefer configuring the project with that option rather than manually circumventing the recipe's test configuration.

## Verification workflow
For a Conan C++ repository with no `test` option:

```sh
conan build . --build=missing
```

For a repository with the optional `test` option:
```sh
conan build . -o '&:test=True' --build=missing
```

Then run the test runner defined by the repository, typically CTest.
Do not assume a particular test runner solely because Conan is present.

## Repository conventions
Treat the Conan recipe as authoritative for:
- dependencies
- options
- generators
- layout
- CMake configuration
- package components
- tool requirements

Prefer changing the Conan recipe over adding shell-level workarounds when the repository's build model needs to change.

Preserve existing dependency version policies and repository remotes unless the task explicitly requires changing them.

Do not update dependencies merely because newer versions exist.

## Mixed-language repositories
Conan governs the C/C++ portion of the project.

If the repository also contains Rust, use its Rust workflow separately. A common verification command is:

```sh
cargo test --locked --all-features
```

Use it only when the repository contains an applicable Cargo workspace or package.

Do not treat Cargo commands as part of Conan configuration.