#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: cmake-package.sh <build-dir>" >&2
  exit 2
fi

# Git Bash on Windows: paths as Windows spells them (C:/...), which CMake reads
native_pwd() {
  pwd -W 2>/dev/null || pwd
}

build_dir="$(cd "$1" && native_pwd)"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && native_pwd)"
install_dir="$build_dir/package-smoke/install"
build_type="$(sed -n 's/^CMAKE_BUILD_TYPE:STRING=//p' "$build_dir/CMakeCache.txt")"

if [[ -z "$build_type" ]]; then
  echo "Missing CMAKE_BUILD_TYPE in $build_dir/CMakeCache.txt" >&2
  exit 2
fi

cmake -E remove_directory "$build_dir/package-smoke"
cmake --install "$build_dir" --prefix "$install_dir"

cache_option() {
  sed -n "s/^$1:BOOL=//p" "$build_dir/CMakeCache.txt"
}

include_prefix="w/common"

is_on() {
  [[ "${1^^}" =~ ^(ON|TRUE|YES|Y|1)$ ]]
}

components=(core)
if is_on "$(cache_option WLIB_WITH_GLAZE)"; then
  components+=(glaze)
fi

if [[ ! -f "$install_dir/include/$include_prefix/config.h" ]]; then
  echo "Headers were not installed under include/$include_prefix" >&2
  exit 1
fi

# Adapter headers are installed only together with their component.
if [[ " ${components[*]} " != *" glaze "* ]] && [[ -d "$install_dir/include/$include_prefix/glaze" ]]; then
  echo "Headers of the disabled glaze component were installed" >&2
  exit 1
fi
# the Qt integration is the separate package wlib_qt
if [[ -d "$install_dir/include/$include_prefix/qt" ]] || [[ -d "$install_dir/include/$include_prefix/qml" ]]; then
  echo "Headers of the Qt integration were installed" >&2
  exit 1
fi

# Conan's toolchain of the build: the consumer uses the same compiler and, with MSVC, the same C++
# runtime as the package (a /MT package does not link into a /MD consumer)
toolchain=()
if [[ -f "$build_dir/generators/conan_toolchain.cmake" ]]; then
  toolchain=(-DCMAKE_TOOLCHAIN_FILE="$build_dir/generators/conan_toolchain.cmake")
fi

for component in "${components[@]}"; do
  consumer_build="$build_dir/package-smoke/$component"
  cmake \
    -S "$repo_root/tests/cmake_consumer" \
    -B "$consumer_build" \
    -G Ninja \
    "${toolchain[@]}" \
    -DWLIB_CONSUMER_COMPONENT="$component" \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_PREFIX_PATH="$install_dir;$build_dir/generators"
  cmake --build "$consumer_build"
  # the run environment finds the shared dependencies of libwlib, e.g. spdlog. Windows has no
  # conanrun.sh: the caller imports conanrun.bat, and conanbuild.bat for MSVC (devops/ci conan_env)
  (
    if [[ -f "$build_dir/generators/conanrun.sh" ]]; then
      # shellcheck disable=SC1091
      source "$build_dir/generators/conanrun.sh"
    fi
    # Windows has no rpath: a shared wlib.dll is found on PATH, from the installed package
    if [[ -d "$install_dir/bin" ]]; then
      PATH="$(cygpath -u "$install_dir/bin" 2>/dev/null || echo "$install_dir/bin"):$PATH"
    fi
    "$consumer_build/wlib_consumer"
  )
done
