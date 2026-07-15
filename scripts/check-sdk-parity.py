#!/usr/bin/env python3
"""Audit TS SDK <-> UE SDK name parity and keep the SDK map current.

The strict check is the Node CLI command key matrix. The generated inventory is
intentionally name-driven: it scans TS and UE source roots, compares normalized
folder/file/symbol names, and writes same/different/missing rows to map.md.
There are no manual TS-to-UE mirror tables in this script.
"""

from __future__ import annotations

import argparse
import difflib
import json
import os
import re
import sys
from dataclasses import dataclass
from datetime import date
from pathlib import Path
from typing import Callable, Sequence, TypeVar


GENERATED_START = "<!-- BEGIN GENERATED SDK PARITY INVENTORY -->"
GENERATED_END = "<!-- END GENERATED SDK PARITY INVENTORY -->"

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

CANDIDATE_LIMIT = 3
CANDIDATE_THRESHOLD = 0.62


@dataclass(frozen=True)
class Symbol:
    name: str
    kind: str
    file: str


@dataclass(frozen=True)
class FolderMirror:
    ts_folder: str
    ts_signature: str
    ue_folders: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class FileMirror:
    ts_file: str
    ts_signature: str
    ue_files: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class SymbolMirror:
    ts_symbol: Symbol
    ue_symbols: tuple[Symbol, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class ParityProgram:
    label: str
    ts_source_roots: tuple[Path, ...]
    ue_source_roots: tuple[Path, ...]


@dataclass(frozen=True)
class ProgramInventory:
    program: ParityProgram
    ts_folders: list[str]
    ue_folders: list[str]
    folder_mirrors: list[FolderMirror]
    ts_files: list[Path]
    ue_files: list[Path]
    file_mirrors: list[FileMirror]
    ts_symbols: list[Symbol]
    ue_symbols: list[Symbol]
    symbol_mirrors: list[SymbolMirror]


T = TypeVar("T")


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def relative(path: Path, root: Path) -> str:
    return path.relative_to(root).as_posix()


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def has_ignored_part(path: Path) -> bool:
    return any(part in IGNORED_PARTS for part in path.parts)


def candidate_ts_roots(root: Path) -> list[Path]:
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    roots = [Path(env_root)] if env_root else []
    siblings = sorted(
        (
            candidate
            for candidate in root.parent.iterdir()
            if candidate.is_dir() and candidate != root and (candidate / "package.json").is_file()
        ),
        key=lambda candidate: candidate.name.casefold(),
    )
    return roots + siblings


def resolve_ts_root(root: Path, explicit_root: str | None) -> Path:
    roots = [Path(explicit_root)] if explicit_root else candidate_ts_roots(root)
    for ts_root in roots:
        try:
            source_roots = discover_ts_source_roots(ts_root)
            find_ts_cli_data(ts_root, source_roots)
            return ts_root
        except (FileNotFoundError, ValueError, json.JSONDecodeError):
            continue

    searched = "\n".join(f"  - {candidate}" for candidate in roots)
    raise FileNotFoundError(
        "Could not find TS SDK root. Set FORBOCAI_TS_SDK_ROOT or pass "
        f"--ts-sdk-root.\nSearched:\n{searched}"
    )


def read_workspace_patterns(ts_root: Path) -> list[str]:
    package_json = ts_root / "package.json"
    if not package_json.is_file():
        return ["packages/*"]

    data = json.loads(read_text(package_json))
    workspaces = data.get("workspaces", ["packages/*"])
    if isinstance(workspaces, list):
        return [pattern for pattern in workspaces if isinstance(pattern, str)]
    if isinstance(workspaces, dict):
        packages = workspaces.get("packages", [])
        return [pattern for pattern in packages if isinstance(pattern, str)]
    return ["packages/*"]


def discover_ts_source_roots(ts_root: Path) -> tuple[Path, ...]:
    roots: set[Path] = set()
    for pattern in read_workspace_patterns(ts_root):
        for package_dir in ts_root.glob(pattern):
            source_root = package_dir / "src"
            if source_root.is_dir():
                roots.add(source_root.relative_to(ts_root))

    if not roots:
        roots.update(
            source_root.relative_to(ts_root)
            for source_root in ts_root.glob("packages/*/src")
            if source_root.is_dir()
        )

    return tuple(sorted(roots, key=lambda path: path.as_posix()))


def discover_ue_source_roots(root: Path) -> tuple[Path, ...]:
    roots: set[Path] = set()
    for build_file in root.glob("Source/*/*.Build.cs"):
        roots.add(build_file.parent.relative_to(root))
    for build_file in root.glob("*/Source/*/*.Build.cs"):
        roots.add(build_file.parent.relative_to(root))

    if not roots and (root / "Source").is_dir():
        roots.add(Path("Source"))

    return tuple(sorted(roots, key=lambda path: path.as_posix()))


def ts_package_name(source_root: Path) -> str:
    parts = source_root.parts
    return parts[1] if len(parts) >= 3 and parts[0] == "packages" and parts[2] == "src" else source_root.as_posix()


def ue_module_name(source_root: Path) -> str:
    return source_root.name


def parity_group(name: str) -> str | None:
    normalized = normalize_name(name)
    if "browser" in normalized:
        return None
    return "test-game" if "testgame" in normalized else "sdk"


def program_label(
    group: str,
    ts_source_roots: Sequence[Path],
    ue_source_roots: Sequence[Path],
) -> str:
    title = group.replace("-", " ").title()
    ts_names = " + ".join(ts_package_name(root) for root in ts_source_roots)
    ue_names = " + ".join(ue_module_name(root) for root in ue_source_roots)
    return f"{title}: TS {ts_names} -> UE {ue_names}"


def build_parity_programs(
    ts_source_roots: Sequence[Path],
    ue_source_roots: Sequence[Path],
) -> tuple[ParityProgram, ...]:
    programs: list[ParityProgram] = []
    for group in ("sdk", "test-game"):
        matching_ts_roots = tuple(
            sorted(
                (
                    source_root
                    for source_root in ts_source_roots
                    if parity_group(ts_package_name(source_root)) == group
                ),
                key=lambda source_root: ts_package_name(source_root).casefold(),
            )
        )
        matching_ue_roots = tuple(
            sorted(
                (
                    source_root
                    for source_root in ue_source_roots
                    if parity_group(ue_module_name(source_root)) == group
                ),
                key=lambda source_root: ue_module_name(source_root).casefold(),
            )
        )
        if matching_ts_roots and matching_ue_roots:
            programs.append(
                ParityProgram(
                    program_label(group, matching_ts_roots, matching_ue_roots),
                    matching_ts_roots,
                    matching_ue_roots,
                )
            )
    return tuple(programs)


def unmapped_ts_roots(ts_source_roots: Sequence[Path], programs: Sequence[ParityProgram]) -> tuple[Path, ...]:
    mapped = {source_root for program in programs for source_root in program.ts_source_roots}
    return tuple(source_root for source_root in ts_source_roots if source_root not in mapped)


def unmapped_ue_roots(ue_source_roots: Sequence[Path], programs: Sequence[ParityProgram]) -> tuple[Path, ...]:
    mapped = {source_root for program in programs for source_root in program.ue_source_roots}
    return tuple(source_root for source_root in ue_source_roots if source_root not in mapped)


def candidate_map_paths(root: Path) -> list[Path]:
    env_path = os.environ.get("FORBOCAI_SDK_MAP_PATH")
    paths = [Path(env_path)] if env_path else []
    return paths + [
        root.parent / "classified/docs/design/sdk/map.md",
        root.parent / "docs/design/sdk/map.md",
    ]


def resolve_map_path(root: Path, explicit_path: str | None) -> Path | None:
    if explicit_path:
        return Path(explicit_path)
    return next((path for path in candidate_map_paths(root) if path.is_file()), None)


def iter_ts_files(ts_root: Path, source_roots: Sequence[Path]) -> list[Path]:
    files: list[Path] = []
    for source_root in source_roots:
        root = ts_root / source_root
        if not root.is_dir():
            continue
        files.extend(
            path
            for path in root.rglob("*")
            if path.is_file()
            and path.suffix in TS_SOURCE_SUFFIXES
            and not path.name.endswith(".d.ts")
            and not has_ignored_part(path.relative_to(ts_root))
        )
    return sorted(files, key=lambda path: relative(path, ts_root))


def iter_ue_files(root: Path, source_roots: Sequence[Path]) -> list[Path]:
    source_files: list[Path] = []
    for source_root in source_roots:
        base = root / source_root
        if not base.is_dir():
            continue
        source_files.extend(
            path
            for path in base.rglob("*")
            if path.is_file()
            and path.suffix in UE_SOURCE_SUFFIXES
            and not has_ignored_part(path.relative_to(root))
        )

    return sorted(set(source_files), key=lambda path: relative(path, root))


def source_root_for_ts_path(path: str, source_roots: Sequence[Path]) -> str | None:
    return next(
        (
            source_root.as_posix()
            for source_root in source_roots
            if path == source_root.as_posix() or path.startswith(f"{source_root.as_posix()}/")
        ),
        None,
    )


def iter_ts_folders(ts_files: Sequence[Path], ts_root: Path, source_roots: Sequence[Path]) -> list[str]:
    folders: set[str] = set()
    for path in ts_files:
        rel = relative(path, ts_root)
        source_root = source_root_for_ts_path(rel, source_roots)
        if not source_root:
            continue

        parent = Path(rel).parent
        while True:
            folder = parent.as_posix()
            if folder == "." or not (folder == source_root or folder.startswith(f"{source_root}/")):
                break
            folders.add(folder)
            if folder == source_root:
                break
            parent = parent.parent

    return sorted(folders)


def iter_ue_folders(ue_files: Sequence[Path], ue_root: Path) -> list[str]:
    folders: set[str] = set()
    for path in ue_files:
        parent = Path(relative(path, ue_root)).parent
        while True:
            folder = parent.as_posix()
            if folder == ".":
                break
            folders.add(folder)
            parent = parent.parent
    return sorted(folders)


def normalize_name(name: str) -> str:
    without_unreal_prefix = re.sub(r"^(F|E|U|I)(?=[A-Z])", "", name)
    return re.sub(r"[^a-z0-9]", "", without_unreal_prefix.lower())


def normalize_path_name(name: str) -> str:
    return re.sub(r"[^a-z0-9]", "", name.lower())


def path_name(path: str) -> str:
    return Path(path).stem if Path(path).suffix else Path(path).name


def package_for_ts_path(path: str) -> str:
    parts = Path(path).parts
    if len(parts) >= 2 and parts[0] == "packages":
        return parts[1]
    return parts[0] if parts else "other"


def ts_path_signature(path: str, source_roots: Sequence[Path]) -> str:
    source_root = source_root_for_ts_path(path, source_roots)
    if source_root is None:
        return normalize_path_signature(path)

    relative_path = Path(path).relative_to(source_root).as_posix()
    if relative_path == ".":
        return normalize_path_name(package_for_ts_path(path))
    return normalize_path_signature(relative_path)


def normalized_path_segments(path: str) -> tuple[str, ...]:
    raw_parts = Path(path).parts
    if Path(path).suffix:
        raw_parts = (*raw_parts[:-1], Path(path).stem)
    return tuple(segment for segment in (normalize_path_name(part) for part in raw_parts) if segment)


def normalize_path_signature(path: str) -> str:
    return "/".join(normalized_path_segments(path))


def ue_suffix_index(paths: Sequence[str]) -> dict[str, list[str]]:
    index: dict[str, list[str]] = {}
    for path in paths:
        segments = normalized_path_segments(path)
        for start in range(len(segments)):
            signature = "/".join(segments[start:])
            index.setdefault(signature, []).append(path)
    return index


def name_similarity(left: str, right: str) -> float:
    return difflib.SequenceMatcher(None, normalize_name(left), normalize_name(right)).ratio()


def index_by_normalized_name(values: Sequence[T], name_of: Callable[[T], str]) -> dict[str, list[T]]:
    index: dict[str, list[T]] = {}
    for value in values:
        index.setdefault(normalize_name(name_of(value)), []).append(value)
    return index


def closest_candidates(
    ts_name: str,
    ue_values: Sequence[T],
    name_of: Callable[[T], str],
    sort_of: Callable[[T], str],
    limit: int = CANDIDATE_LIMIT,
    threshold: float = CANDIDATE_THRESHOLD,
) -> tuple[T, ...]:
    scored = [
        (name_similarity(ts_name, name_of(value)), sort_of(value), value)
        for value in ue_values
        if normalize_name(ts_name) != normalize_name(name_of(value))
    ]
    filtered = [item for item in scored if item[0] >= threshold]
    return tuple(value for _, _, value in sorted(filtered, key=lambda item: (-item[0], item[1]))[:limit])


def mirror_status(candidates: Sequence[object], exact: bool) -> str:
    return "Same" if exact else "Different"


def mirror_notes(candidates: Sequence[object], exact: bool) -> str:
    if exact:
        return "Same normalized path/name signature."
    if candidates:
        return "No same normalized path/name signature; listed UE item(s) are dynamic nearest candidates."
    return "No UE item has the same normalized path/name signature; no close dynamic candidate was found."


def iter_ts_data_files(ts_root: Path, source_roots: Sequence[Path]) -> tuple[Path, ...]:
    package_roots = {
        (ts_root / source_root).parent
        for source_root in source_roots
        if source_root.name == "src"
    }
    return tuple(
        sorted(
            (
                path
                for package_root in package_roots
                for path in (package_root / "data").rglob("*.json")
                if path.is_file()
            ),
            key=lambda path: path.as_posix(),
        )
    )


def extract_ts_node_keys(path: Path) -> list[str]:
    data = json.loads(read_text(path))
    if not isinstance(data, dict):
        raise ValueError(f"Expected object-authored CLI data in {path}")

    commands = data.get("commands")
    surfaces = data.get("surfaces")
    if not isinstance(commands, dict) or not isinstance(surfaces, dict) or "node" not in surfaces:
        raise ValueError(f"Could not find a Node CLI command catalog in {path}")

    keys: list[str] = []
    for key, command in commands.items():
        if not isinstance(key, str) or not isinstance(command, dict):
            raise ValueError(f"Invalid CLI command entry in {path}")
        command_surfaces = command.get("surfaces")
        if not isinstance(command_surfaces, list) or not all(
            isinstance(surface, str) for surface in command_surfaces
        ):
            raise ValueError(f"Invalid CLI command surfaces in {path}")
        if "node" in command_surfaces:
            keys.append(key)

    if not keys:
        raise ValueError(f"Node CLI command catalog is empty in {path}")
    return keys


def find_ts_cli_data(
    ts_root: Path,
    source_roots: Sequence[Path],
) -> tuple[Path, list[str]]:
    return find_matrix_source(
        iter_ts_data_files(ts_root, source_roots),
        extract_ts_node_keys,
        "TS CLI authored-data catalog",
    )


def extract_ue_node_keys(path: Path) -> list[str]:
    text = read_text(path)
    match = re.search(
        r"BEGIN_NODE_CLI_COMMAND_KEYS(?P<body>.*?)END_NODE_CLI_COMMAND_KEYS",
        text,
        re.S,
    )
    if not match:
        raise ValueError(f"Could not find UE node CLI command marker block in {path}")
    return re.findall(r'\{\s*TEXT\("([^"]+)"\)', match.group("body"))


def find_matrix_source(
    source_files: Sequence[Path],
    extractor: Callable[[Path], list[str]],
    label: str,
) -> tuple[Path, list[str]]:
    matches: list[tuple[Path, list[str]]] = []
    for path in source_files:
        try:
            keys = extractor(path)
        except (UnicodeDecodeError, ValueError, json.JSONDecodeError):
            continue
        matches.append((path, keys))

    if len(matches) != 1:
        paths = ", ".join(str(path) for path, _keys in matches) or "none"
        raise ValueError(f"Expected exactly one {label}; found {len(matches)}: {paths}")
    return matches[0]


def strip_comments(text: str) -> str:
    without_block = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//.*", "", without_block)


def strip_leading_template_declaration(line: str) -> str:
    """Remove simple leading C++ template declarations before symbol matching."""
    previous = None
    stripped = line.strip()
    while previous != stripped:
        previous = stripped
        stripped = re.sub(r"^template\s*<[^<>]*>\s*", "", stripped).strip()
    return stripped


def split_names(body: str) -> list[str]:
    names: list[str] = []
    for raw in body.split(","):
        item = raw.strip()
        if not item:
            continue
        item = re.sub(r"\s+as\s+\w+$", "", item)
        item = item.split(":")[-1].strip()
        item = re.sub(r"=.*$", "", item).strip()
        if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", item):
            names.append(item)
    return names


def extract_ts_symbols(path: Path, ts_root: Path) -> list[Symbol]:
    rel = relative(path, ts_root)
    if path.name == "index.ts":
        return []

    text = strip_comments(read_text(path))
    symbols: list[Symbol] = []
    patterns = (
        (r"\bexport\s+(?:async\s+)?function\s+([A-Za-z_][A-Za-z0-9_]*)", "function"),
        (r"\bexport\s+const\s+([A-Za-z_][A-Za-z0-9_]*)\b", "const"),
        (r"\bexport\s+class\s+([A-Za-z_][A-Za-z0-9_]*)", "class"),
        (r"\bexport\s+interface\s+([A-Za-z_][A-Za-z0-9_]*)", "interface"),
        (r"\bexport\s+type\s+([A-Za-z_][A-Za-z0-9_]*)", "type"),
        (r"\bexport\s+enum\s+([A-Za-z_][A-Za-z0-9_]*)", "enum"),
    )
    for pattern, kind in patterns:
        symbols.extend(Symbol(match, kind, rel) for match in re.findall(pattern, text))

    for body in re.findall(r"\bexport\s+const\s+\{(?P<body>.*?)\}\s*=", text, re.S):
        symbols.extend(Symbol(name, "const", rel) for name in split_names(body))

    for body in re.findall(r"\bexport\s+\{(?P<body>.*?)\}", text, re.S):
        symbols.extend(Symbol(name, "re-export", rel) for name in split_names(body))

    return sorted({f"{symbol.name}:{symbol.kind}:{symbol.file}": symbol for symbol in symbols}.values(), key=lambda item: (item.file, item.name))


def extract_ue_symbols(path: Path, root: Path) -> list[Symbol]:
    if path.suffix not in {".h", ".hpp", ".cpp"}:
        return []

    rel = relative(path, root)
    text = strip_comments(read_text(path))
    symbols: list[Symbol] = []
    lines = text.splitlines()
    for index, line in enumerate(lines):
        indent = len(line) - len(line.lstrip(" "))
        if indent > 2:
            continue
        stripped = strip_leading_template_declaration(line)
        if not stripped or stripped.startswith("#") or stripped.startswith("template"):
            continue
        if re.match(r"namespace(?:\s+[A-Za-z_][A-Za-z0-9_]*)?\s*\{", stripped):
            continue

        type_match = re.match(r"(?:USTRUCT\([^)]*\)\s*)?(?:struct|class)\s+(?:FORBOCAI_SDK_API\s+)?([A-Za-z_][A-Za-z0-9_]*)", stripped)
        enum_match = re.match(r"enum\s+class\s+([A-Za-z_][A-Za-z0-9_]*)", stripped)
        using_match = re.match(r"using\s+([A-Za-z_][A-Za-z0-9_]*)\s*=", stripped)
        typedef_match = re.match(r"typedef\s+.+?\s+([A-Za-z_][A-Za-z0-9_]*)\s*;", stripped)
        if type_match:
            symbols.append(Symbol(type_match.group(1), "type", rel))
            continue
        if enum_match:
            symbols.append(Symbol(enum_match.group(1), "enum", rel))
            continue
        if using_match:
            symbols.append(Symbol(using_match.group(1), "type", rel))
            continue
        if typedef_match:
            symbols.append(Symbol(typedef_match.group(1), "type", rel))
            continue

        next_stripped = ""
        if "(" not in stripped and index + 1 < len(lines):
            next_line = lines[index + 1]
            next_indent = len(next_line) - len(next_line.lstrip(" "))
            next_stripped = strip_leading_template_declaration(next_line) if next_indent <= 2 else ""
        declaration = f"{stripped} {next_stripped}".strip()

        function_match = re.match(
            r"(?:FORBOCAI_SDK_API\s+)?(?:virtual\s+)?(?:static\s+)?(?:inline\s+)?"
            r"(?:constexpr\s+)?[A-Za-z_][A-Za-z0-9_:<>*&,\s]*?"
            r"(?:\s|[*&])([A-Za-z_][A-Za-z0-9_]*)\s*\(",
            declaration,
        )
        if function_match:
            name = function_match.group(1)
            if name not in {"if", "for", "while", "switch", "return"}:
                symbols.append(Symbol(name, "function", rel))
            continue

        value_match = re.match(
            r"(?:FORBOCAI_SDK_API\s+)?(?:static\s+)?(?:constexpr\s+)?(?:const\s+)?(?:inline\s+)?"
            r"(?:[A-Za-z_][A-Za-z0-9_:<>*&,\s]*\s+)+([A-Za-z_][A-Za-z0-9_]*)\s*(?:=|;|\{)",
            declaration,
        )
        if indent == 0 and value_match:
            symbols.append(Symbol(value_match.group(1), "const", rel))

    return sorted({f"{symbol.name}:{symbol.kind}:{symbol.file}": symbol for symbol in symbols}.values(), key=lambda item: (item.file, item.name))


def build_folder_mirrors(
    ts_folders: Sequence[str],
    ue_folders: Sequence[str],
    ts_source_roots: Sequence[Path],
) -> list[FolderMirror]:
    ue_by_signature = ue_suffix_index(ue_folders)
    mirrors: list[FolderMirror] = []
    for folder in ts_folders:
        signature = ts_path_signature(folder, ts_source_roots)
        same = tuple(sorted(ue_by_signature.get(signature, [])))
        candidates = same or closest_candidates(
            signature,
            ue_folders,
            normalize_path_signature,
            lambda value: value,
        )
        mirrors.append(
            FolderMirror(
                folder,
                signature,
                candidates,
                mirror_status(candidates, bool(same)),
                mirror_notes(candidates, bool(same)),
            )
        )
    return mirrors


def build_file_mirrors(
    ts_files: Sequence[Path],
    ts_root: Path,
    ue_files: Sequence[Path],
    ue_root: Path,
    ts_source_roots: Sequence[Path],
) -> list[FileMirror]:
    ue_rel = [relative(path, ue_root) for path in ue_files]
    ue_by_signature = ue_suffix_index(ue_rel)
    mirrors: list[FileMirror] = []
    for path in ts_files:
        rel = relative(path, ts_root)
        signature = ts_path_signature(rel, ts_source_roots)
        same = tuple(sorted(ue_by_signature.get(signature, [])))
        candidates = same or closest_candidates(
            signature,
            ue_rel,
            normalize_path_signature,
            lambda value: value,
        )
        mirrors.append(
            FileMirror(
                rel,
                signature,
                candidates,
                mirror_status(candidates, bool(same)),
                mirror_notes(candidates, bool(same)),
            )
        )
    return mirrors


def build_symbol_mirrors(ts_symbols: Sequence[Symbol], ue_symbols: Sequence[Symbol]) -> list[SymbolMirror]:
    ue_by_name = index_by_normalized_name(ue_symbols, lambda symbol: symbol.name)
    mirrors: list[SymbolMirror] = []
    for symbol in ts_symbols:
        same = tuple(sorted(ue_by_name.get(normalize_name(symbol.name), []), key=lambda item: (item.file, item.kind, item.name)))
        same_kind_values = [item for item in ue_symbols if item.kind == symbol.kind]
        search_values = same_kind_values if same_kind_values else list(ue_symbols)
        candidates = same or closest_candidates(
            symbol.name,
            search_values,
            lambda value: value.name,
            lambda value: f"{value.file}:{value.kind}:{value.name}",
        )
        mirrors.append(
            SymbolMirror(
                symbol,
                candidates,
                mirror_status(candidates, bool(same)),
                mirror_notes(candidates, bool(same)),
            )
        )
    return mirrors


def build_program_inventory(ts_root: Path, ue_root: Path, program: ParityProgram) -> ProgramInventory:
    ts_files = iter_ts_files(ts_root, program.ts_source_roots)
    ue_files = iter_ue_files(ue_root, program.ue_source_roots)
    ts_folders = iter_ts_folders(ts_files, ts_root, program.ts_source_roots)
    ue_folders = iter_ue_folders(ue_files, ue_root)
    folder_mirrors = build_folder_mirrors(ts_folders, ue_folders, program.ts_source_roots)
    file_mirrors = build_file_mirrors(ts_files, ts_root, ue_files, ue_root, program.ts_source_roots)
    ts_symbols = [symbol for path in ts_files for symbol in extract_ts_symbols(path, ts_root)]
    ue_symbols = [symbol for path in ue_files for symbol in extract_ue_symbols(path, ue_root)]
    symbol_mirrors = build_symbol_mirrors(ts_symbols, ue_symbols)

    return ProgramInventory(
        program,
        ts_folders,
        ue_folders,
        folder_mirrors,
        ts_files,
        ue_files,
        file_mirrors,
        ts_symbols,
        ue_symbols,
        symbol_mirrors,
    )


def used_same_ue_folders(folder_mirrors: Sequence[FolderMirror]) -> set[str]:
    return {folder for mirror in folder_mirrors if mirror.status == "Same" for folder in mirror.ue_folders}


def used_same_ue_files(file_mirrors: Sequence[FileMirror]) -> set[str]:
    return {file for mirror in file_mirrors if mirror.status == "Same" for file in mirror.ue_files}


def used_same_ue_symbols(symbol_mirrors: Sequence[SymbolMirror]) -> set[tuple[str, str, str]]:
    return {
        (symbol.name, symbol.kind, symbol.file)
        for mirror in symbol_mirrors
        if mirror.status == "Same"
        for symbol in mirror.ue_symbols
    }


def md(value: str) -> str:
    escaped = value.replace("|", "\\|").replace("\n", " ")
    return escaped if escaped else "-"


def md_code(value: str) -> str:
    return f"`{md(value)}`" if value else "-"


def format_list(values: Sequence[str]) -> str:
    return "<br>".join(md_code(value) for value in values) if values else "-"


def format_symbol_list(values: Sequence[Symbol]) -> str:
    if not values:
        return "-"
    return "<br>".join(md_code(f"{value.name} [{value.kind}] ({value.file})") for value in values)


def table(headers: tuple[str, ...], rows: list[tuple[str, ...]]) -> list[str]:
    lines = [
        "| " + " | ".join(headers) + " |",
        "| " + " | ".join(":---" for _ in headers) + " |",
    ]
    lines.extend("| " + " | ".join(row) + " |" for row in rows)
    return lines


def package_label(package: str) -> str:
    return package.replace("-", " ").title() if package else "Other"


def package_order(package: str) -> tuple[str, str]:
    return normalize_name(package), package.casefold()


def grouped(items: Sequence[T], package_of: Callable[[T], str]) -> list[tuple[str, list[T]]]:
    packages = sorted({package_of(item) for item in items}, key=package_order)
    return [(package, [item for item in items if package_of(item) == package]) for package in packages]


def status_count(items: Sequence[FolderMirror | FileMirror | SymbolMirror], status: str) -> int:
    return sum(1 for item in items if item.status == status)


def folder_mirror_rows(folder_mirrors: Sequence[FolderMirror]) -> list[tuple[str, ...]]:
    return [
        (
            md_code(mirror.ts_folder),
            md_code(mirror.ts_signature),
            format_list(mirror.ue_folders),
            mirror.status,
            md(mirror.notes),
        )
        for mirror in folder_mirrors
    ]


def file_mirror_rows(file_mirrors: Sequence[FileMirror]) -> list[tuple[str, ...]]:
    return [
        (
            md_code(mirror.ts_file),
            md_code(mirror.ts_signature),
            format_list(mirror.ue_files),
            mirror.status,
            md(mirror.notes),
        )
        for mirror in file_mirrors
    ]


def symbol_mirror_rows(symbol_mirrors: Sequence[SymbolMirror]) -> list[tuple[str, ...]]:
    return [
        (
            md_code(f"{mirror.ts_symbol.name} ({mirror.ts_symbol.file})"),
            md(mirror.ts_symbol.kind),
            format_symbol_list(mirror.ue_symbols),
            mirror.status,
            md(mirror.notes),
        )
        for mirror in symbol_mirrors
    ]


def package_summary_rows(
    folder_mirrors: Sequence[FolderMirror],
    file_mirrors: Sequence[FileMirror],
    symbol_mirrors: Sequence[SymbolMirror],
) -> list[tuple[str, ...]]:
    packages = sorted(
        {
            *{package_for_ts_path(mirror.ts_folder) for mirror in folder_mirrors},
            *{package_for_ts_path(mirror.ts_file) for mirror in file_mirrors},
            *{package_for_ts_path(mirror.ts_symbol.file) for mirror in symbol_mirrors},
        },
        key=package_order,
    )
    rows: list[tuple[str, ...]] = []
    for package in packages:
        folders = [mirror for mirror in folder_mirrors if package_for_ts_path(mirror.ts_folder) == package]
        files = [mirror for mirror in file_mirrors if package_for_ts_path(mirror.ts_file) == package]
        symbols = [mirror for mirror in symbol_mirrors if package_for_ts_path(mirror.ts_symbol.file) == package]
        rows.append(
            (
                package_label(package),
                str(len(folders)),
                str(status_count(folders, "Same")),
                str(status_count(folders, "Different")),
                str(len(files)),
                str(status_count(files, "Same")),
                str(status_count(files, "Different")),
                str(len(symbols)),
                str(status_count(symbols, "Same")),
                str(status_count(symbols, "Different")),
            )
        )
    return rows


def format_roots(roots: Sequence[Path]) -> str:
    return ", ".join(md_code(root.as_posix()) for root in roots) if roots else "-"


def flatten(sequence: Sequence[Sequence[T]]) -> list[T]:
    return [item for items in sequence for item in items]


def build_program_section(inventory: ProgramInventory, ts_root: Path, ue_root: Path) -> list[str]:
    program = inventory.program
    ue_rel_files = [relative(path, ue_root) for path in inventory.ue_files]
    ue_only_folders = [
        folder for folder in inventory.ue_folders if folder not in used_same_ue_folders(inventory.folder_mirrors)
    ]
    ue_only_files = [file for file in ue_rel_files if file not in used_same_ue_files(inventory.file_mirrors)]
    used_symbols = used_same_ue_symbols(inventory.symbol_mirrors)
    ue_only_symbols = [
        symbol
        for symbol in inventory.ue_symbols
        if (symbol.name, symbol.kind, symbol.file) not in used_symbols
    ]

    lines: list[str] = [
        f"### {program.label}",
        "",
        f"- TS source roots: {format_roots(program.ts_source_roots)}",
        f"- UE source roots: {format_roots(program.ue_source_roots)}",
        "",
        *table(
            ("Metric", "Count"),
            [
                ("TS source folders scanned", str(len(inventory.ts_folders))),
                ("UE source folders scanned", str(len(inventory.ue_folders))),
                ("TS folders with same UE path/name signature", str(status_count(inventory.folder_mirrors, "Same"))),
                ("TS folders with different/no same UE path/name signature", str(status_count(inventory.folder_mirrors, "Different"))),
                ("UE folders without same TS path/name signature", str(len(ue_only_folders))),
                ("TS source files scanned", str(len(inventory.ts_files))),
                ("UE source files scanned", str(len(inventory.ue_files))),
                ("TS files with same UE path/name signature", str(status_count(inventory.file_mirrors, "Same"))),
                ("TS files with different/no same UE path/name signature", str(status_count(inventory.file_mirrors, "Different"))),
                ("UE files without same TS path/name signature", str(len(ue_only_files))),
                ("TS exported symbols/functions scanned", str(len(inventory.ts_symbols))),
                ("TS exported symbols/functions with same UE name", str(status_count(inventory.symbol_mirrors, "Same"))),
                ("TS exported symbols/functions with different/no same UE name", str(status_count(inventory.symbol_mirrors, "Different"))),
                ("UE symbols/functions without same TS name", str(len(ue_only_symbols))),
            ],
        ),
        "",
        "#### Package Coverage",
        "",
        *table(
            (
                "Area",
                "TS folders",
                "Folder same",
                "Folder different",
                "TS files",
                "File same",
                "File different",
                "TS symbols/functions",
                "Symbol/function same",
                "Symbol/function different",
            ),
            package_summary_rows(inventory.folder_mirrors, inventory.file_mirrors, inventory.symbol_mirrors),
        ),
        "",
        "#### Folder Name Mirrors",
        "",
    ]

    for package, mirrors in grouped(inventory.folder_mirrors, lambda mirror: package_for_ts_path(mirror.ts_folder)):
        lines.extend(
            [
                f"##### {package_label(package)} Folder Names",
                "",
                *table(
                    ("TypeScript folder", "TS path/name signature", "UE folder(s)", "Status", "Notes"),
                    folder_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
            "#### UE Folder Names Without Same TS Path/Name Signature",
            "",
            *table(
                ("UE folder", "UE path/name signature", "Notes"),
                [
                    (
                        md_code(path),
                        md_code(normalize_path_signature(path)),
                        "No TS source folder currently has the same normalized path/name signature in this parity program.",
                    )
                    for path in ue_only_folders
                ],
            ),
            "",
            "#### File Name Mirrors",
            "",
        ]
    )

    for package, mirrors in grouped(inventory.file_mirrors, lambda mirror: package_for_ts_path(mirror.ts_file)):
        lines.extend(
            [
                f"##### {package_label(package)} File Names",
                "",
                *table(
                    ("TypeScript file", "TS path/name signature", "UE file(s)", "Status", "Notes"),
                    file_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
            "#### UE File Names Without Same TS Path/Name Signature",
            "",
            *table(
                ("UE file", "UE path/name signature", "Notes"),
                [
                    (
                        md_code(path),
                        md_code(normalize_path_signature(path)),
                        "No TS source file currently has the same normalized path/name signature in this parity program.",
                    )
                    for path in ue_only_files
                ],
            ),
            "",
            "#### Symbol And Function Name Mirrors",
            "",
        ]
    )

    for package, mirrors in grouped(inventory.symbol_mirrors, lambda mirror: package_for_ts_path(mirror.ts_symbol.file)):
        lines.extend(
            [
                f"##### {package_label(package)} Symbol And Function Names",
                "",
                *table(
                    ("TypeScript symbol/function", "Kind", "UE symbol/function(s)", "Status", "Notes"),
                    symbol_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
            "#### UE Symbol And Function Names Without Same TS Name",
            "",
            *table(
                ("UE symbol/function", "Kind", "Notes"),
                [
                    (
                        md_code(f"{symbol.name} ({symbol.file})"),
                        md(symbol.kind),
                        "No exported TS symbol/function currently has the same normalized name in this parity program.",
                    )
                    for symbol in ue_only_symbols
                ],
            ),
            "",
        ]
    )
    return lines


def build_generated_section(
    ts_root: Path,
    ue_root: Path,
    inventories: Sequence[ProgramInventory],
    ts_source_roots: Sequence[Path],
    ue_source_roots: Sequence[Path],
    unmapped_ts_source_roots: Sequence[Path],
    unmapped_ue_source_roots: Sequence[Path],
    ts_keys: Sequence[str],
    ue_keys: Sequence[str],
) -> str:
    generated_on = date.today().isoformat()
    folder_mirrors = flatten([inventory.folder_mirrors for inventory in inventories])
    file_mirrors = flatten([inventory.file_mirrors for inventory in inventories])
    symbol_mirrors = flatten([inventory.symbol_mirrors for inventory in inventories])
    ts_folders = flatten([inventory.ts_folders for inventory in inventories])
    ue_folders = flatten([inventory.ue_folders for inventory in inventories])
    ts_files = flatten([inventory.ts_files for inventory in inventories])
    ue_files = flatten([inventory.ue_files for inventory in inventories])
    ts_symbols = flatten([inventory.ts_symbols for inventory in inventories])
    ue_symbols = flatten([inventory.ue_symbols for inventory in inventories])

    lines: list[str] = [
        GENERATED_START,
        "## Generated SDK Parity Inventory",
        "",
        f"Generated by `scripts/check-sdk-parity.py` on {generated_on}.",
        "",
        "This section is source-derived. It compares discovered folder, file, and exported symbol/function names inside the active parity programs without manual TS-to-UE exception tables.",
        "",
        "### Generated Summary",
        "",
        *table(
            ("Metric", "Count"),
            [
                ("Parity programs scanned", str(len(inventories))),
                ("Discovered TS source roots", str(len(ts_source_roots))),
                ("Discovered UE source roots", str(len(ue_source_roots))),
                ("Unmapped TS source roots", str(len(unmapped_ts_source_roots))),
                ("Unmapped UE source roots", str(len(unmapped_ue_source_roots))),
                ("TS source folders scanned", str(len(ts_folders))),
                ("UE source folders scanned", str(len(ue_folders))),
                ("TS folders with same UE path/name signature", str(status_count(folder_mirrors, "Same"))),
                ("TS folders with different/no same UE path/name signature", str(status_count(folder_mirrors, "Different"))),
                ("TS source files scanned", str(len(ts_files))),
                ("UE source files scanned", str(len(ue_files))),
                ("TS files with same UE path/name signature", str(status_count(file_mirrors, "Same"))),
                ("TS files with different/no same UE path/name signature", str(status_count(file_mirrors, "Different"))),
                ("TS exported symbols/functions scanned", str(len(ts_symbols))),
                ("UE symbols/functions scanned", str(len(ue_symbols))),
                ("TS exported symbols/functions with same UE name", str(status_count(symbol_mirrors, "Same"))),
                ("TS exported symbols/functions with different/no same UE name", str(status_count(symbol_mirrors, "Different"))),
                ("Node CLI command keys", str(len(ts_keys))),
            ],
        ),
        "",
        f"- TS root: `{ts_root}`",
        f"- UE root: `{ue_root}`",
        f"- Discovered TS source roots: {format_roots(ts_source_roots)}",
        f"- Discovered UE source roots: {format_roots(ue_source_roots)}",
        f"- Unmapped TS source roots: {format_roots(unmapped_ts_source_roots)}",
        f"- Unmapped UE source roots: {format_roots(unmapped_ue_source_roots)}",
        "- Active parity programs: " + "; ".join(
            f"{inventory.program.label}"
            for inventory in inventories
        ) + ".",
        "- `Same` for folders/files means a UE path contains the same normalized TS path/name suffix inside that parity program.",
        "- `Different` means no same normalized path/name suffix was found; listed UE item(s), when present, are closest dynamic candidates from that same parity program.",
        "- Function names are included in the generated symbol/function mirrors.",
        "",
    ]

    for inventory in inventories:
        lines.extend(build_program_section(inventory, ts_root, ue_root))

    lines.extend(
        [
            "### Generated Node CLI Command Keys",
            "",
            *table(
                ("Order", "TS key", "UE key", "Status"),
                [
                    (
                        str(index + 1),
                        md_code(ts_key),
                        md_code(ue_keys[index]) if index < len(ue_keys) else "-",
                        "Same" if index < len(ue_keys) and ts_key == ue_keys[index] else "Different",
                    )
                    for index, ts_key in enumerate(ts_keys)
                ],
            ),
            GENERATED_END,
        ]
    )
    return "\n".join(lines) + "\n"


def replace_generated_section(text: str, section: str) -> str:
    if GENERATED_START in text and GENERATED_END in text:
        pattern = re.compile(
            re.escape(GENERATED_START) + r".*?" + re.escape(GENERATED_END) + r"\n?",
            re.S,
        )
        return pattern.sub(section, text)

    insert_before = "\n## 11. Maintenance Rule"
    if insert_before in text:
        return text.replace(insert_before, "\n" + section + insert_before, 1)

    return text.rstrip() + "\n\n" + section


def update_last_updated(text: str) -> str:
    today = date.today().isoformat()
    return re.sub(r"Last updated:\s*\d{4}-\d{2}-\d{2}", f"Last updated: {today}", text, count=1)


def check_cli_command_parity(
    ts_root: Path,
    ue_root: Path,
    ts_source_roots: Sequence[Path],
    ue_source_roots: Sequence[Path],
) -> tuple[int, list[str], list[str]]:
    ts_matrix, ts_keys = find_ts_cli_data(ts_root, ts_source_roots)
    ue_matrix, ue_keys = find_matrix_source(
        tuple(
            path
            for path in iter_ue_files(ue_root, ue_source_roots)
            if path.suffix in UE_SOURCE_SUFFIXES
        ),
        extract_ue_node_keys,
        "UE CLI command matrix",
    )

    missing = [key for key in ts_keys if key not in ue_keys]
    extra = [key for key in ue_keys if key not in ts_keys]
    order_matches = ts_keys == ue_keys

    print("[check] UE SDK parity")
    print(f"[info] TS SDK root: {ts_root}")
    print(f"[info] UE SDK root: {ue_root}")
    print(f"[info] TS CLI authored data: {ts_matrix}")
    print(f"[info] UE CLI matrix: {ue_matrix}")

    if not missing and not extra and order_matches:
        print(f"[ok] UE mirrors TS NODE_CLI_COMMAND_KEYS ({len(ts_keys)} commands)")
        return 0, ts_keys, ue_keys

    if missing:
        print("[fail] Commands present in TS but missing in UE:")
        print("\n".join(f"  - {value}" for value in missing))
    if extra:
        print("[fail] Commands present in UE node-parity block but absent from TS:")
        print("\n".join(f"  - {value}" for value in extra))
    if not order_matches and not missing and not extra:
        print("[fail] Command sets match, but order differs from TS matrix.")

    print("")
    print("[hint] Update Source/ForbocAI_SDK/Public/Features/CLI/CLIAdapters.h")
    print("       so the BEGIN_NODE_CLI_COMMAND_KEYS block mirrors TS exactly.")
    return 1, ts_keys, ue_keys


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compare TS SDK and UE SDK folder, file, symbol, and CLI-key names."
    )
    parser.add_argument("--ts-sdk-root", help="Path to the TS SDK repo root.")
    parser.add_argument("--map-path", help="Path to classified/docs/design/sdk/map.md.")
    parser.add_argument("--require-map", action="store_true", help="Fail when map.md cannot be found.")
    args = parser.parse_args()

    ue_root = repo_root()
    ts_root = resolve_ts_root(ue_root, args.ts_sdk_root)
    map_path = resolve_map_path(ue_root, args.map_path)

    ts_source_roots = discover_ts_source_roots(ts_root)
    ue_source_roots = discover_ue_source_roots(ue_root)
    cli_status, ts_keys, ue_keys = check_cli_command_parity(
        ts_root,
        ue_root,
        ts_source_roots,
        ue_source_roots,
    )
    programs = build_parity_programs(ts_source_roots, ue_source_roots)
    if not programs:
        raise ValueError("No TS/UE parity programs were discovered.")

    inventories = [build_program_inventory(ts_root, ue_root, program) for program in programs]
    unmapped_ts_source_roots = unmapped_ts_roots(ts_source_roots, programs)
    unmapped_ue_source_roots = unmapped_ue_roots(ue_source_roots, programs)

    folder_mirrors = flatten([inventory.folder_mirrors for inventory in inventories])
    file_mirrors = flatten([inventory.file_mirrors for inventory in inventories])
    symbol_mirrors = flatten([inventory.symbol_mirrors for inventory in inventories])
    ts_folders = flatten([inventory.ts_folders for inventory in inventories])
    ue_folders = flatten([inventory.ue_folders for inventory in inventories])
    ts_files = flatten([inventory.ts_files for inventory in inventories])
    ue_files = flatten([inventory.ue_files for inventory in inventories])

    section = build_generated_section(
        ts_root,
        ue_root,
        inventories,
        ts_source_roots,
        ue_source_roots,
        unmapped_ts_source_roots,
        unmapped_ue_source_roots,
        ts_keys,
        ue_keys,
    )

    print(f"[info] Discovered TS source roots: {', '.join(root.as_posix() for root in ts_source_roots)}")
    print(f"[info] Discovered UE source roots: {', '.join(root.as_posix() for root in ue_source_roots)}")
    for inventory in inventories:
        print(f"[info] {inventory.program.label}")
        print(f"       TS roots: {', '.join(root.as_posix() for root in inventory.program.ts_source_roots)}")
        print(f"       UE roots: {', '.join(root.as_posix() for root in inventory.program.ue_source_roots)}")
    print(f"[info] TS source folders scanned: {len(ts_folders)}")
    print(f"[info] UE source folders scanned: {len(ue_folders)}")
    print(f"[info] TS folders without same UE path/name signature: {len(folder_mirrors) - status_count(folder_mirrors, 'Same')}")
    print(f"[info] TS source files scanned: {len(ts_files)}")
    print(f"[info] UE source files scanned: {len(ue_files)}")
    print(f"[info] TS files without same UE path/name signature: {len(file_mirrors) - status_count(file_mirrors, 'Same')}")
    print(f"[info] TS exported symbols/functions without same UE name: {len(symbol_mirrors) - status_count(symbol_mirrors, 'Same')}")

    map_status = 0
    if map_path is None:
        message = "[warn] map.md not found; generated inventory freshness was not checked."
        print(message)
        map_status = 1 if args.require_map else 0
    elif not map_path.exists():
        print(f"[fail] map.md not found: {map_path}")
        map_status = 1
    else:
        original = read_text(map_path)
        updated = update_last_updated(replace_generated_section(original, section))
        map_path.write_text(updated, encoding="utf-8", newline="\n")
        if original != updated:
            print(f"[ok] Updated generated SDK parity inventory in {map_path}")
        else:
            print(f"[ok] Generated SDK parity inventory is current in {map_path}")

    return 1 if cli_status or map_status else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[fail] {exc}", file=sys.stderr)
        raise SystemExit(1)
