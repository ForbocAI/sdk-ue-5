from __future__ import annotations

import json
import os
from collections.abc import Sequence
from pathlib import Path

from parity.sdk.normalization import normalize_name
from parity.sdk.types import ParityProgram


TS_SOURCE_SUFFIXES = {".ts", ".tsx"}
UE_SOURCE_SUFFIXES = {".h", ".hpp", ".cpp", ".cs"}
IGNORED_PARTS = {
    "__tests__",
    "__pycache__",
    "dist",
    "Binaries",
    "Intermediate",
    "Saved",
    "Tests",
    "ThirdParty",
    "node_modules",
}


def relative(path: Path, root: Path) -> str:
    return path.relative_to(root).as_posix()


def has_ignored_part(path: Path) -> bool:
    return any(part in IGNORED_PARTS for part in path.parts)


def candidate_ts_roots(root: Path) -> list[Path]:
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    roots = [Path(env_root)] if env_root else []
    siblings = sorted(
        (
            candidate
            for candidate in root.parent.iterdir()
            if candidate.is_dir()
            and candidate != root
            and (candidate / "package.json").is_file()
        ),
        key=lambda candidate: candidate.name.casefold(),
    )
    return roots + siblings


def has_split_cli_contract(root: Path) -> bool:
    return any(
        path.parent.joinpath("commands").is_dir()
        and path.parent.joinpath("roles.json").is_file()
        for path in root.glob("packages/*/data/cli/vocabulary.json")
    )


def resolve_ts_root(root: Path, explicit_root: str | None) -> Path:
    roots = [Path(explicit_root)] if explicit_root else candidate_ts_roots(root)
    match = next(
        (
            candidate.resolve()
            for candidate in roots
            if candidate.is_dir() and has_split_cli_contract(candidate)
        ),
        None,
    )
    if match is not None:
        return match
    searched = "\n".join(f"  - {candidate}" for candidate in roots)
    raise FileNotFoundError(
        "Could not find the TS SDK split CLI contract. Set FORBOCAI_TS_SDK_ROOT "
        f"or pass --ts-sdk-root.\nSearched:\n{searched}"
    )


def read_workspace_patterns(ts_root: Path) -> list[str]:
    package_json = ts_root / "package.json"
    if not package_json.is_file():
        return ["packages/*"]
    data = json.loads(package_json.read_text(encoding="utf-8"))
    workspaces = data.get("workspaces", ["packages/*"])
    if isinstance(workspaces, list):
        return [pattern for pattern in workspaces if isinstance(pattern, str)]
    packages = workspaces.get("packages", []) if isinstance(workspaces, dict) else []
    return [pattern for pattern in packages if isinstance(pattern, str)]


def discover_ts_source_roots(ts_root: Path) -> tuple[Path, ...]:
    roots = {
        source_root.relative_to(ts_root)
        for pattern in read_workspace_patterns(ts_root)
        for package_dir in ts_root.glob(pattern)
        for source_root in (package_dir / "src",)
        if source_root.is_dir()
    }
    fallback = {
        source_root.relative_to(ts_root)
        for source_root in ts_root.glob("packages/*/src")
        if source_root.is_dir()
    }
    return tuple(sorted(roots or fallback, key=lambda path: path.as_posix()))


def discover_ue_source_roots(root: Path) -> tuple[Path, ...]:
    build_files = (*root.glob("Source/*/*.Build.cs"), *root.glob("*/Source/*/*.Build.cs"))
    roots = {path.parent.relative_to(root) for path in build_files}
    fallback = {Path("Source")} if not roots and (root / "Source").is_dir() else set()
    return tuple(sorted(roots or fallback, key=lambda path: path.as_posix()))


def ts_package_name(source_root: Path) -> str:
    parts = source_root.parts
    return (
        parts[1]
        if len(parts) >= 3 and parts[0] == "packages" and parts[2] == "src"
        else source_root.as_posix()
    )


def ue_module_name(source_root: Path) -> str:
    return source_root.name


def parity_group(name: str) -> str | None:
    normalized = normalize_name(name)
    if "browser" in normalized:
        return None
    return "micro-game" if "microgame" in normalized else "sdk"


def program_label(
    group: str,
    ts_source_roots: Sequence[Path],
    ue_source_roots: Sequence[Path],
) -> str:
    title = group[:1].upper() + group[1:]
    ts_names = " + ".join(ts_package_name(root) for root in ts_source_roots)
    ue_names = " + ".join(ue_module_name(root) for root in ue_source_roots)
    return f"{title}: TS {ts_names} -> UE {ue_names}"


def build_parity_programs(
    ts_source_roots: Sequence[Path],
    ue_source_roots: Sequence[Path],
) -> tuple[ParityProgram, ...]:
    programs: list[ParityProgram] = []
    for group in ("sdk", "micro-game"):
        ts_matches = tuple(
            sorted(
                (
                    root
                    for root in ts_source_roots
                    if parity_group(ts_package_name(root)) == group
                ),
                key=lambda root: ts_package_name(root).casefold(),
            )
        )
        ue_matches = tuple(
            sorted(
                (
                    root
                    for root in ue_source_roots
                    if parity_group(ue_module_name(root)) == group
                ),
                key=lambda root: ue_module_name(root).casefold(),
            )
        )
        if ts_matches and ue_matches:
            programs.append(
                ParityProgram(
                    program_label(group, ts_matches, ue_matches),
                    ts_matches,
                    ue_matches,
                )
            )
    return tuple(programs)


def unmapped_ts_roots(
    roots: Sequence[Path], programs: Sequence[ParityProgram]
) -> tuple[Path, ...]:
    mapped = {root for program in programs for root in program.ts_source_roots}
    return tuple(root for root in roots if root not in mapped)


def unmapped_ue_roots(
    roots: Sequence[Path], programs: Sequence[ParityProgram]
) -> tuple[Path, ...]:
    mapped = {root for program in programs for root in program.ue_source_roots}
    return tuple(root for root in roots if root not in mapped)


def iter_ts_files(ts_root: Path, source_roots: Sequence[Path]) -> list[Path]:
    return sorted(
        (
            path
            for source_root in source_roots
            for path in (ts_root / source_root).rglob("*")
            if path.is_file()
            and path.suffix in TS_SOURCE_SUFFIXES
            and not path.name.endswith(".d.ts")
            and not has_ignored_part(path.relative_to(ts_root))
        ),
        key=lambda path: relative(path, ts_root),
    )


def iter_ue_files(root: Path, source_roots: Sequence[Path]) -> list[Path]:
    return sorted(
        {
            path
            for source_root in source_roots
            for path in (root / source_root).rglob("*")
            if path.is_file()
            and path.suffix in UE_SOURCE_SUFFIXES
            and not has_ignored_part(path.relative_to(root))
        },
        key=lambda path: relative(path, root),
    )


def source_root_for_ts_path(
    path: str, source_roots: Sequence[Path]
) -> str | None:
    return next(
        (
            root.as_posix()
            for root in source_roots
            if path == root.as_posix() or path.startswith(f"{root.as_posix()}/")
        ),
        None,
    )


def iter_ts_folders(
    ts_files: Sequence[Path], ts_root: Path, source_roots: Sequence[Path]
) -> list[str]:
    folders: set[str] = set()
    for path in ts_files:
        source_root = source_root_for_ts_path(relative(path, ts_root), source_roots)
        parent = path.relative_to(ts_root).parent
        while source_root and parent.as_posix().startswith(source_root):
            folders.add(parent.as_posix())
            if parent.as_posix() == source_root:
                break
            parent = parent.parent
    return sorted(folders)


def iter_ue_folders(ue_files: Sequence[Path], ue_root: Path) -> list[str]:
    folders: set[str] = set()
    for path in ue_files:
        parent = path.relative_to(ue_root).parent
        while parent.as_posix() != ".":
            folders.add(parent.as_posix())
            parent = parent.parent
    return sorted(folders)
