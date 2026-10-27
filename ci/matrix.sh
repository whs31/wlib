#!/usr/bin/env bash
# Builds and tests every Conan configuration of the GitLab test:cpp:x86_64 matrix locally.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

run_configuration() {
  local with_glaze="$1" shared="$2"
  local output_folder="build/ci-glaze-${with_glaze,,}-shared-${shared,,}"
  conan build . \
    --build=missing \
    -cc core:non_interactive=True \
    -s:h build_type=Debug \
    -s:b build_type=Debug \
    -o "&:shared=$shared" \
    -o "&:test=True" \
    -o "&:with_glaze=$with_glaze" \
    -of "$output_folder"
  ctest --test-dir "$output_folder/build/Debug" --output-on-failure
  bash ci/cmake-package.sh "$output_folder/build/Debug"
  if [[ "$shared" == True ]]; then
    mapfile -t libraries < <(find "$output_folder/build/Debug" -type f -name 'libwlib.so*')
    test "${#libraries[@]}" -gt 0
    linkage="$(ldd "${libraries[0]}")"
    # WLib never links Qt; the Qt integration is the separate package wlib_qt
    ! grep -Fq "libQt6" <<< "$linkage"
  fi
}

for with_glaze in False True; do
  for shared in False True; do
    run_configuration "$with_glaze" "$shared"
  done
done
