#!/usr/bin/env python3
"""UE SDK/demo dead-code and dead-data guard.

This SDK-owned guard scans either the UE SDK plugin root or a UE runtime/demo
project root. It fails on orphan authored headers and orphan authored data:

* DEAD-SRC-001: an authored header (.h/.hpp) no other source file includes.
  Reflection headers, module entry headers, and automation tests are entry
  points and are not classified as orphan headers by text include scans.

* DEAD-DATA-001: an authored Content/Data JSON or CSV file whose Content-
  relative path and filename appear nowhere across Source, Content, and scripts.

No path-narrowing arguments are supported. Delete real orphans or wire them into
the include/manifest/script path that consumes them.
"""

from __future__ import annotations

import argparse
from collections import defaultdict
import json
from pathlib import Path
import re
import sys
from typing import Iterable


DEFAULT_PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE_SUFFIXES = {".h", ".hpp", ".cpp"}
HEADER_SUFFIXES = {".h", ".hpp"}
DATA_SUFFIXES = {".json", ".csv"}
SCRIPT_SUFFIXES = {".py", ".sh", ".ps1", ".cmd", ".bat"}
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

INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.MULTILINE)
REFLECTION_RE = re.compile(r"\b(UCLASS|UINTERFACE|USTRUCT|UENUM|GENERATED_BODY|GENERATED_UCLASS_BODY)\b")


def resolve_root(root: Path) -> Path:
    return root.expanduser().resolve()


def has_excluded_part(path: Path) -> bool:
    return any(part in EXCLUDED_DIRS for part in path.parts)


def read(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""


def iter_files(root: Path, suffixes: set[str]) -> list[Path]:
    if not root.is_dir():
        return []
    return sorted(
        path
        for path in root.rglob("*")
        if path.is_file() and path.suffix in suffixes and not has_excluded_part(path)
    )


def rel(project_root: Path, path: Path) -> str:
    try:
        return path.relative_to(project_root).as_posix()
    except ValueError:
        return path.as_posix()


def json_module_names(path: Path) -> set[str]:
    try:
        parsed = json.loads(read(path))
    except json.JSONDecodeError:
        return set()
    modules = parsed.get("Modules") if isinstance(parsed, dict) else None
    if not isinstance(modules, list):
        return set()
    return {
        item["Name"]
        for item in modules
        if isinstance(item, dict) and isinstance(item.get("Name"), str)
    }


def module_entry_stems(project_root: Path) -> set[str]:
    manifests: Iterable[Path] = [
        *project_root.glob("*.uproject"),
        *project_root.glob("*.uplugin"),
    ]
    return {name for path in manifests for name in json_module_names(path)}


def source_roots(project_root: Path) -> list[Path]:
    source = project_root / "Source"
    return [source] if source.is_dir() else []


def script_roots(project_root: Path) -> list[Path]:
    return [root for root in [project_root / "Scripts", project_root / "scripts"] if root.is_dir()]


def source_files(project_root: Path) -> list[Path]:
    return [
        path
        for root in source_roots(project_root)
        for path in iter_files(root, SOURCE_SUFFIXES)
    ]


def orphan_headers(project_root: Path) -> list[Path]:
    files = source_files(project_root)
    texts = {path: read(path) for path in files}
    module_stems = module_entry_stems(project_root)

    includers: dict[str, set[Path]] = defaultdict(set)
    for path, text in texts.items():
        for target in INCLUDE_RE.findall(text):
            includers[target].add(path)
            includers[Path(target).name].add(path)

    orphans: list[Path] = []
    source_root = project_root / "Source"
    for path in files:
        if path.suffix not in HEADER_SUFFIXES:
            continue
        if "Tests" in path.parts or path.stem in module_stems:
            continue
        if REFLECTION_RE.search(texts[path]):
            continue
        source_rel = path.relative_to(source_root).as_posix()
        sibling_cpp = path.with_suffix(".cpp")
        refs = includers.get(source_rel, set()) | includers.get(path.name, set())
        if not (refs - {sibling_cpp}):
            orphans.append(path)
    return orphans


def reference_blob(project_root: Path) -> str:
    parts: list[str] = []
    for path in source_files(project_root):
        parts.append(read(path))
    for root in [project_root / "Content", *script_roots(project_root)]:
        suffixes = DATA_SUFFIXES if root.name == "Content" else SCRIPT_SUFFIXES
        for path in iter_files(root, suffixes):
            parts.append(read(path))
    return "\n".join(parts)


def orphan_data(project_root: Path) -> list[Path]:
    content_data_root = project_root / "Content" / "Data"
    blob = reference_blob(project_root)
    orphans: list[Path] = []
    for path in iter_files(content_data_root, DATA_SUFFIXES):
        content_rel = "Data/" + path.relative_to(content_data_root).as_posix()
        if content_rel in blob or path.name in blob:
            continue
        orphans.append(path)
    return orphans


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__,
        epilog=(
            "This guard scans the selected project Source/Content/scripts trees; "
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
    headers = orphan_headers(project_root)
    data = orphan_data(project_root)
    total = len(headers) + len(data)

    if total == 0:
        print(f"Dead-code guard passed for {project_root}. No orphan headers or data files.")
        return 0

    print(
        f"Dead-code guard failed for {project_root}: "
        f"{total} orphan file(s) ({len(headers)} header(s), {len(data)} data file(s))."
    )
    for path in headers:
        print(f"{rel(project_root, path)}: DEAD-SRC-001 orphan header -- no other file #includes it; delete it or wire it in.")
    for path in data:
        print(f"{rel(project_root, path)}: DEAD-DATA-001 orphan data -- referenced by no Source/Content/script; delete it or reference it.")
    print("")
    print(
        "An orphan file is stale dead code/data. Confirm it is truly used by "
        "reflection, config, or dynamically built paths and wire that path into "
        "the source tree; otherwise delete it."
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
