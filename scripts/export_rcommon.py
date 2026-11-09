#!/usr/bin/env python3

"""Mirror WLib to Git or replace an rcommon checkout with its translated tree."""

from __future__ import annotations

import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


SOURCE_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_GIT_MIRROR_URL = "https://github.com/whs31/wlib"
SOURCE_DIRECTORIES = (
  ".agents",
  ".claude",
  "ci",
  "cmake",
  "config",
  "docs",
  "include",
  "scripts",
  "skills",
  "src",
  "tests",
)
SOURCE_FILES = (
  ".clang-format",
  ".clang-tidy",
  ".gitattributes",
  ".gitignore",
  ".pre-commit-config.yaml",
  "AGENTS.md",
  "CLAUDE.md",
  "CMakeLists.txt",
  "Justfile",
  "LICENSE",
  "README.md",
  "conandata.yml",
  "conanfile.py",
  "wkit.toml",
)
PRESERVED_TARGET_FILES = {Path(".gitlab-ci.yml")}
EXPORTER = Path("scripts/export_rcommon.py")

TEXT_REPLACEMENTS = (
  ("https://git.whs31.com/shared/support/wlib", "http://gitlab.uav.radar-mms.com/developers/v2/support/rcommon"),
  ("@whs31/dev", "@radar/dev"),
  ("whs31/dev", "radar/dev"),
  ('user = "whs31"', 'user = "radar"'),
  ('DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/w"', 'DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/radar/mms"'),
  ("EXPORT_NAME lib", "EXPORT_NAME common"),
  ("NAMESPACE w::", "NAMESPACE radar::mms::"),
  ("w::lib", "radar::mms::common"),
  ("w::common", "radar::mms::common"),
  ("namespace w {", "namespace radar::mms {"),
  ("w/common", "radar/mms/common"),
  ("W_COMMON", "RADAR_COMMON"),
  ("WLIB", "RCOMMON"),
  ("WLib", "RCommon"),
  ("wlib", "rcommon"),
)
# `W_` only at the start of a word, so that e.g. `ROW_MAJOR` stays as it is.
REGEX_REPLACEMENTS = (
  (re.compile(r"(?<![A-Za-z0-9_])W_"), "RADAR_"),
  (re.compile(r"\busing namespace w(?=;|::)"), "using namespace radar::mms"),
  (re.compile(r"(?<![A-Za-z0-9_])w::"), "radar::mms::"),
)
FORBIDDEN_PATTERN = re.compile(r"\busing namespace w(?=;|::)|(?<![A-Za-z0-9_])w::|(?<![A-Za-z0-9_])W_")


def fail(message: str) -> None:
  raise SystemExit(f"error: {message}")


def is_git_url(destination: str) -> bool:
  return "://" in destination or re.fullmatch(r"[^/\\\s]+@[^:\s]+:.+", destination) is not None


def push_mirror(destination: str) -> None:
  result = subprocess.run(("git", "push", "--mirror", destination), cwd=SOURCE_ROOT)
  if result.returncode != 0:
    raise SystemExit(result.returncode)


def git(*args: str, cwd: Path) -> str:
  result = subprocess.run(
    ("git", *args),
    cwd=cwd,
    check=True,
    stdout=subprocess.PIPE,
    text=True,
  )
  return result.stdout


def source_files() -> list[Path]:
  output = subprocess.run(
    ("git", "ls-files", "-z", "--cached", "--others", "--exclude-standard"),
    cwd=SOURCE_ROOT,
    check=True,
    stdout=subprocess.PIPE,
  ).stdout
  candidates = (Path(item.decode()) for item in output.split(b"\0") if item)
  return sorted(
    relative
    for relative in candidates
    if relative.as_posix() in SOURCE_FILES or relative.parts[0] in SOURCE_DIRECTORIES
  )


def translate_path(relative: Path) -> Path:
  path = relative.as_posix()
  if path == "cmake/WLibConfig.cmake.in":
    return Path("cmake/RCommonConfig.cmake.in")
  if path == "skills/wlib/SKILL.md":
    return Path("skills/rcommon/SKILL.md")
  if path.startswith("include/w/common/"):
    return Path("include/radar/mms/common") / Path(path).relative_to("include/w/common")
  return relative


def translate_text(text: str, relative: Path) -> str:
  for source, target in TEXT_REPLACEMENTS:
    text = text.replace(source, target)
  for pattern, target in REGEX_REPLACEMENTS:
    text = pattern.sub(target, text)

  if relative == Path("conanfile.py"):
    text = text.replace('components["lib"]', 'components["common"]')
    text = text.replace('        "lib",\n        "glaze::glaze",', '        "common",\n        "glaze::glaze",')
  return text


def stage_tree(stage: Path) -> int:
  destinations: set[Path] = set()
  for relative in source_files():
    destination = translate_path(relative)
    if destination in destinations:
      fail(f"two source files map to {destination}")
    destinations.add(destination)

    source = SOURCE_ROOT / relative
    output = stage / destination
    output.parent.mkdir(parents=True, exist_ok=True)
    data = source.read_bytes()
    try:
      text = data.decode("utf-8")
    except UnicodeDecodeError:
      output.write_bytes(data)
    else:
      # The exporter is shared tooling: keeping its mapping table verbatim also lets an exported
      # rcommon checkout mirror itself without rewriting the translation rules.
      translated = text if relative == EXPORTER else translate_text(text, relative)
      forbidden = (
        "w/common",
        "W_COMMON",
        "WLIB",
        "WLib",
        "wlib",
      )
      if relative != EXPORTER:
        for marker in forbidden:
          if marker in translated:
            fail(f"generated {destination} still contains {marker!r}")
        if FORBIDDEN_PATTERN.search(translated):
          fail(f"generated {destination} still contains the WLib namespace or a W_ macro")
      output.write_text(translated, encoding="utf-8")

  cmake = (stage / "CMakeLists.txt").read_text(encoding="utf-8")
  conan = (stage / "conanfile.py").read_text(encoding="utf-8")
  namespace = (stage / "include/radar/mms/common/detail/namespace.h").read_text(encoding="utf-8")
  expected = (
    ("project(rcommon", cmake),
    ("add_library(radar::mms::common ALIAS", cmake),
    ("NAMESPACE radar::mms::", cmake),
    ('DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/radar/mms"', cmake),
    ('name = "rcommon"', conan),
    ('user = "radar"', conan),
    ("@radar/dev", conan),
    ("namespace radar::mms { inline namespace common {", namespace),
  )
  for marker, contents in expected:
    if marker not in contents:
      fail(f"generated tree is missing {marker!r}")
  return len(destinations)


def validate_target(target: Path) -> None:
  if target == SOURCE_ROOT:
    fail("the target cannot be the WLib checkout")
  if not target.is_dir():
    fail(f"target is not a directory: {target}")

  try:
    repository = Path(git("rev-parse", "--show-toplevel", cwd=target).strip()).resolve()
  except subprocess.CalledProcessError:
    fail(f"target is not a Git checkout: {target}")
  if repository != target:
    fail(f"target must be the repository root: {repository}")

  conanfile = target / "conanfile.py"
  if not conanfile.is_file() or 'name = "rcommon"' not in conanfile.read_text(encoding="utf-8"):
    fail(f"target does not look like the rcommon repository: {target}")

  status = git("status", "--porcelain", "--untracked-files=normal", cwd=target)
  if status:
    fail("target has uncommitted files; commit or stash them before mirroring")


def format_tree(stage: Path) -> None:
  try:
    subprocess.run(("wkit", "fmt", "--quiet"), cwd=stage, check=True)
  except FileNotFoundError:
    fail("wkit is required to format the translated tree")
  except subprocess.CalledProcessError:
    fail("wkit could not format the translated tree")


def tracked_files(target: Path) -> list[Path]:
  output = subprocess.run(
    ("git", "ls-files", "-z"),
    cwd=target,
    check=True,
    stdout=subprocess.PIPE,
  ).stdout
  return [Path(item.decode()) for item in output.split(b"\0") if item]


def replace_target(stage: Path, target: Path) -> tuple[int, int]:
  preserved = {
    relative: (target / relative).read_bytes()
    for relative in PRESERVED_TARGET_FILES
    if (target / relative).is_file()
  }
  tracked = tracked_files(target)

  for directory in SOURCE_DIRECTORIES:
    path = target / directory
    if path.is_dir():
      shutil.rmtree(path)
    elif path.exists():
      path.unlink()

  removed = sum(relative not in PRESERVED_TARGET_FILES for relative in tracked)
  for relative in tracked:
    if relative in PRESERVED_TARGET_FILES:
      continue
    path = target / relative
    if path.is_file() or path.is_symlink():
      path.unlink()

  written = 0
  for source in sorted(path for path in stage.rglob("*") if path.is_file()):
    relative = source.relative_to(stage)
    destination = target / relative
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)
    written += 1

  for relative, contents in preserved.items():
    if (target / relative).read_bytes() != contents:
      fail(f"preserved file changed unexpectedly: {relative}")
  return removed, written


def main() -> None:
  if len(sys.argv) > 2:
    fail(f"usage: {Path(sys.argv[0]).name} [PATH_TO_RCOMMON_REPO|GIT_URL]")

  destination = sys.argv[1] if len(sys.argv) == 2 else DEFAULT_GIT_MIRROR_URL
  if is_git_url(destination):
    push_mirror(destination)
    return

  target = Path(destination).expanduser().resolve()
  validate_target(target)
  with tempfile.TemporaryDirectory(prefix="wlib-rcommon-") as temporary:
    staged = stage_tree(Path(temporary))
    format_tree(Path(temporary))
    removed, written = replace_target(Path(temporary), target)

  print(
    f"Mirrored {written} files to {target} "
    f"({removed} old tracked files removed; {staged} files staged)."
  )
  print("Preserved .gitlab-ci.yml and Git metadata.")


if __name__ == "__main__":
  main()
