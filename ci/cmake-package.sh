#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
  echo "Usage: cmake-package.sh <build-dir>" >&2
  exit 2
fi

build_dir="$(cd "$1" && pwd)"
repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
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

for component in "${components[@]}"; do
  consumer_build="$build_dir/package-smoke/$component"
  cmake \
    -S "$repo_root/tests/cmake_consumer" \
    -B "$consumer_build" \
    -G Ninja \
    -DWLIB_CONSUMER_COMPONENT="$component" \
    -DCMAKE_BUILD_TYPE="$build_type" \
    -DCMAKE_PREFIX_PATH="$install_dir;$build_dir/generators"
  cmake --build "$consumer_build"
  # the run environment finds the shared dependencies of libwlib, e.g. spdlog
  (
    if [[ -f "$build_dir/generators/conanrun.sh" ]]; then
      # shellcheck disable=SC1091
      source "$build_dir/generators/conanrun.sh"
    fi
    "$consumer_build/wlib_consumer"
  )
done
