#!/usr/bin/env bash
# Builds and tests every Conan configuration of the GitLab test:cpp:x86_64 matrix locally.
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

run_configuration() {
  local with_qt="$1" with_glaze="$2"
  local output_folder="build/ci-qt-${with_qt,,}-glaze-${with_glaze,,}"
  conan build . \
    --build=missing \
    -cc core:non_interactive=True \
    -s:h build_type=Debug \
    -s:b build_type=Debug \
    -o "&:shared=True" \
    -o "&:test=True" \
    -o "&:with_qt=$with_qt" \
    -o "&:with_glaze=$with_glaze" \
    -of "$output_folder"
  ctest --test-dir "$output_folder/build/Debug" --output-on-failure
  bash ci/cmake-package.sh "$output_folder/build/Debug"
  mapfile -t libraries < <(find "$output_folder/build/Debug" -type f -name 'libwlib.so*')
  test "${#libraries[@]}" -gt 0
  linkage="$(ldd "${libraries[0]}")"
  if [[ "$with_qt" == "True" ]]; then
    grep -Fq "libQt6Core" <<< "$linkage"
    grep -Fq "libQt6Gui" <<< "$linkage"
    grep -Fq "libQt6Network" <<< "$linkage"
  else
    ! grep -Fq "libQt6Core" <<< "$linkage"
    ! grep -Fq "libQt6Gui" <<< "$linkage"
    ! grep -Fq "libQt6Network" <<< "$linkage"
  fi
}

for with_qt in False True; do
  for with_glaze in False True; do
    run_configuration "$with_qt" "$with_glaze"
  done
done
