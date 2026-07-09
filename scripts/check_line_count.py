#!/usr/bin/env python3
"""Reject authored UE SDK/demo files over the 300-line ceiling.

This guard is SDK-owned and can scan either the UE SDK plugin root or a UE
runtime/demo project root:

  python3 scripts/check_line_count.py
  python3 scripts/check_line_count.py --root ../demo-ue-5

Line counts intentionally match the project line-count documentation rule:
authored Source and Content text files are checked, generated/build folders are
ignored, and no path-narrowing arguments are accepted.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import sys


DEFAULT_PROJECT_ROOT = Path(__file__).resolve().parents[1]
SCAN_ROOT_NAMES = ("Source", "Content")
TEXT_SUFFIXES = {
    ".h",
    ".hpp",
    ".cpp",
    ".c",
    ".cs",
    ".json",
    ".csv",
    ".ini",
    ".txt",
    ".inl",
    ".inc",
    ".ipp",
}
EXCLUDED_DIRS = {
    ".git",
    ".vs",
    ".vscode",
    "Binaries",
    "Build",
    "DerivedDataCache",
    "Intermediate",
    "Plugins",
    "Saved",
    "__pycache__",
}
MAX_LINES = 300

GUIDANCE = (
    "Split each file by subdomain and recompose through an adapter/fold. Source: "
    "smaller RTK/ECS role leaves or grouped declaration atoms fed to one reusable "
    "composer. Content: smaller catalog/instance/subdomain JSON leaves composed "
    "through the settings adapters. Folder/namespace boundaries own the domain "
    "words; each leaf stays small. Never use .inl/.inc/.ipp fragments or "
    "non-role implementation leaves as an escape hatch; drill into concrete "
    "subdomains and keep the result visible to the structural guards."
)


def resolve_root(root: Path) -> Path:
    return root.expanduser().resolve()


def has_excluded_part(path: Path) -> bool:
    return any(part in EXCLUDED_DIRS for part in path.parts)


def line_count(path: Path) -> int | None:
    try:
        with path.open("r", encoding="utf-8") as handle:
            return sum(1 for _ in handle)
    except (UnicodeDecodeError, OSError):
        return None


def iter_files(project_root: Path) -> list[Path]:
    scan_roots = [project_root / name for name in SCAN_ROOT_NAMES]
    return sorted(
        path
        for root in scan_roots
        if root.is_dir()
        for path in root.rglob("*")
        if path.is_file()
        and path.suffix in TEXT_SUFFIXES
        and not has_excluded_part(path)
    )


def find_over_limit(project_root: Path) -> list[tuple[int, Path]]:
    over = [
        (count, path)
        for path in iter_files(project_root)
        for count in [line_count(path)]
        if count is not None and count > MAX_LINES
    ]
    return sorted(over, key=lambda item: item[0], reverse=True)


def display(project_root: Path, path: Path) -> str:
    try:
        return path.relative_to(project_root).as_posix()
    except ValueError:
        return path.as_posix()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "This guard scans the selected project Source/Content trees; "
            "path-narrowing arguments are intentionally unsupported."
        ),
    )
    parser.add_argument(
        "--root",
        "--runtime-root",
        dest="root",
        type=Path,
        default=DEFAULT_PROJECT_ROOT,
        help="UE SDK plugin or UE runtime/demo project root to scan.",
    )
    return parser.parse_args()


def main() -> int:
    project_root = resolve_root(parse_args().root)
    over = find_over_limit(project_root)
    if not over:
        print(f"Line-count guard passed for {project_root}. No Source/Content file exceeds {MAX_LINES} lines.")
        return 0

    print(f"Line-count guard failed for {project_root}: {len(over)} file(s) over {MAX_LINES} lines.")
    for count, path in over:
        print(f"{display(project_root, path)}: {count} lines, over the {MAX_LINES}-line limit; split into subdomains")
    print("")
    print(GUIDANCE)
    return 1


if __name__ == "__main__":
    sys.exit(main())
