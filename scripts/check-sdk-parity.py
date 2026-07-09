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
import os
import re
import sys
from dataclasses import dataclass
from datetime import date
from pathlib import Path
from typing import Callable, Sequence, TypeVar


GENERATED_START = "<!-- BEGIN GENERATED SDK PARITY INVENTORY -->"
GENERATED_END = "<!-- END GENERATED SDK PARITY INVENTORY -->"

TS_SOURCE_ROOTS = (
    ("core", Path("packages/core/src")),
    ("node", Path("packages/node/src")),
    ("test-game", Path("packages/test-game/src")),
)

UE_SOURCE_ROOTS = (Path("Source/ForbocAI_SDK"),)
UE_SCRIPT_ROOT = Path("scripts")
UE_SOURCE_SUFFIXES = {".h", ".hpp", ".cpp", ".cs"}
IGNORED_PARTS = {"__tests__", "__pycache__", "dist", "Binaries", "Intermediate", "Saved", "Tests", "ThirdParty"}

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
    ue_folders: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class FileMirror:
    ts_file: str
    ue_files: tuple[str, ...]
    status: str
    notes: str


@dataclass(frozen=True)
class SymbolMirror:
    ts_symbol: Symbol
    ue_symbols: tuple[Symbol, ...]
    status: str
    notes: str


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
    return roots + [
        root.parent / "sdk",
        root.parent / "sdk-ts",
        root.parent / "sdk-node",
    ]


def resolve_ts_root(root: Path, explicit_root: str | None) -> Path:
    roots = [Path(explicit_root)] if explicit_root else candidate_ts_roots(root)
    for ts_root in roots:
        if (ts_root / "packages/core/src/cliCommandMatrix.ts").is_file():
            return ts_root

    searched = "\n".join(
        f"  - {candidate / 'packages/core/src/cliCommandMatrix.ts'}"
        for candidate in roots
    )
    raise FileNotFoundError(
        "Could not find TS SDK root. Set FORBOCAI_TS_SDK_ROOT or pass "
        f"--ts-sdk-root.\nSearched:\n{searched}"
    )


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


def iter_ts_files(ts_root: Path) -> list[Path]:
    files: list[Path] = []
    for _, source_root in TS_SOURCE_ROOTS:
        root = ts_root / source_root
        if not root.is_dir():
            continue
        files.extend(
            path
            for path in root.rglob("*.ts")
            if path.is_file()
            and not path.name.endswith(".d.ts")
            and not has_ignored_part(path.relative_to(ts_root))
        )
    return sorted(files, key=lambda path: relative(path, ts_root))


def iter_ue_files(root: Path) -> list[Path]:
    source_files: list[Path] = []
    for source_root in UE_SOURCE_ROOTS:
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

    script_root = root / UE_SCRIPT_ROOT
    if script_root.is_dir():
        source_files.extend(
            path
            for path in script_root.rglob("*")
            if path.is_file()
            and not has_ignored_part(path.relative_to(root))
        )

    source_files.extend(path for path in root.glob("*.uproject") if path.is_file())
    return sorted(set(source_files), key=lambda path: relative(path, root))


def source_root_for_ts_path(path: str) -> str | None:
    return next(
        (
            source_root.as_posix()
            for _, source_root in TS_SOURCE_ROOTS
            if path == source_root.as_posix() or path.startswith(f"{source_root.as_posix()}/")
        ),
        None,
    )


def iter_ts_folders(ts_files: Sequence[Path], ts_root: Path) -> list[str]:
    folders: set[str] = set()
    for path in ts_files:
        rel = relative(path, ts_root)
        source_root = source_root_for_ts_path(rel)
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


def path_name(path: str) -> str:
    return Path(path).stem if Path(path).suffix else Path(path).name


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
        return "Same normalized name."
    if candidates:
        return "No same normalized name; listed UE item(s) are dynamic nearest-name candidates."
    return "No UE item has the same normalized name; no close dynamic candidate was found."


def extract_ts_node_keys(path: Path) -> list[str]:
    text = read_text(path)
    match = re.search(
        r"NODE_CLI_COMMAND_KEYS\s*=\s*\[(?P<body>.*?)\]\s+as\s+const",
        text,
        re.S,
    )
    if not match:
        raise ValueError(f"Could not find NODE_CLI_COMMAND_KEYS in {path}")
    return re.findall(r"'([^']+)'", match.group("body"))


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
            r"(?:[A-Za-z_][A-Za-z0-9_:<>*&,\s]*\s+)+([A-Za-z_][A-Za-z0-9_]*)\s*\(",
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


def build_folder_mirrors(ts_folders: Sequence[str], ue_folders: Sequence[str]) -> list[FolderMirror]:
    ue_by_name = index_by_normalized_name(ue_folders, path_name)
    mirrors: list[FolderMirror] = []
    for folder in ts_folders:
        same = tuple(sorted(ue_by_name.get(normalize_name(path_name(folder)), [])))
        candidates = same or closest_candidates(folder, ue_folders, path_name, lambda value: value)
        mirrors.append(
            FolderMirror(
                folder,
                candidates,
                mirror_status(candidates, bool(same)),
                mirror_notes(candidates, bool(same)),
            )
        )
    return mirrors


def build_file_mirrors(ts_files: Sequence[Path], ts_root: Path, ue_files: Sequence[Path], ue_root: Path) -> list[FileMirror]:
    ue_rel = [relative(path, ue_root) for path in ue_files]
    ue_by_name = index_by_normalized_name(ue_rel, path_name)
    mirrors: list[FileMirror] = []
    for path in ts_files:
        rel = relative(path, ts_root)
        same = tuple(sorted(ue_by_name.get(normalize_name(path_name(rel)), [])))
        candidates = same or closest_candidates(rel, ue_rel, path_name, lambda value: value)
        mirrors.append(
            FileMirror(
                rel,
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


def package_for_ts_path(path: str) -> str:
    if path.startswith("packages/core/"):
        return "core"
    if path.startswith("packages/node/"):
        return "node-cli" if "/cli" in path else "node"
    if path.startswith("packages/test-game/"):
        return "test-game"
    return "other"


def package_label(package: str) -> str:
    labels = {
        "core": "Core",
        "node": "Node",
        "node-cli": "Node CLI",
        "test-game": "Test Game",
        "other": "Other",
    }
    return labels.get(package, package)


def package_order(package: str) -> int:
    order = {"core": 0, "node": 1, "node-cli": 2, "test-game": 3, "other": 4}
    return order.get(package, 99)


def grouped(items: Sequence[T], package_of: Callable[[T], str]) -> list[tuple[str, list[T]]]:
    packages = sorted({package_of(item) for item in items}, key=package_order)
    return [(package, [item for item in items if package_of(item) == package]) for package in packages]


def status_count(items: Sequence[FolderMirror | FileMirror | SymbolMirror], status: str) -> int:
    return sum(1 for item in items if item.status == status)


def folder_mirror_rows(folder_mirrors: Sequence[FolderMirror]) -> list[tuple[str, ...]]:
    return [
        (md_code(mirror.ts_folder), md_code(path_name(mirror.ts_folder)), format_list(mirror.ue_folders), mirror.status, md(mirror.notes))
        for mirror in folder_mirrors
    ]


def file_mirror_rows(file_mirrors: Sequence[FileMirror]) -> list[tuple[str, ...]]:
    return [
        (md_code(mirror.ts_file), md_code(path_name(mirror.ts_file)), format_list(mirror.ue_files), mirror.status, md(mirror.notes))
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


def build_generated_section(
    ts_root: Path,
    ue_root: Path,
    ts_folders: Sequence[str],
    ue_folders: Sequence[str],
    folder_mirrors: Sequence[FolderMirror],
    ts_files: Sequence[Path],
    ue_files: Sequence[Path],
    file_mirrors: Sequence[FileMirror],
    ts_symbols: Sequence[Symbol],
    ue_symbols: Sequence[Symbol],
    symbol_mirrors: Sequence[SymbolMirror],
    ts_keys: Sequence[str],
    ue_keys: Sequence[str],
) -> str:
    generated_on = date.today().isoformat()
    ue_rel_files = [relative(path, ue_root) for path in ue_files]
    ue_only_folders = [folder for folder in ue_folders if folder not in used_same_ue_folders(folder_mirrors)]
    ue_only_files = [file for file in ue_rel_files if file not in used_same_ue_files(file_mirrors)]
    used_symbols = used_same_ue_symbols(symbol_mirrors)
    ue_only_symbols = [
        symbol
        for symbol in ue_symbols
        if (symbol.name, symbol.kind, symbol.file) not in used_symbols
    ]

    lines: list[str] = [
        GENERATED_START,
        "## Generated SDK Parity Inventory",
        "",
        f"Generated by `scripts/check-sdk-parity.py --update-map` on {generated_on}.",
        "",
        "This section is source-derived. It compares discovered folder, file, and exported symbol names without manual TS-to-UE mirror tables.",
        "",
        "### Generated Summary",
        "",
        *table(
            ("Metric", "Count"),
            [
                ("TS source folders scanned", str(len(ts_folders))),
                ("UE source folders scanned", str(len(ue_folders))),
                ("TS folders with same UE name", str(status_count(folder_mirrors, "Same"))),
                ("TS folders with different/no same UE name", str(status_count(folder_mirrors, "Different"))),
                ("UE folders without same TS name", str(len(ue_only_folders))),
                ("TS source files scanned", str(len(ts_files))),
                ("UE source files scanned", str(len(ue_files))),
                ("TS files with same UE name", str(status_count(file_mirrors, "Same"))),
                ("TS files with different/no same UE name", str(status_count(file_mirrors, "Different"))),
                ("UE files without same TS name", str(len(ue_only_files))),
                ("TS exported symbols/functions scanned", str(len(ts_symbols))),
                ("TS exported symbols/functions with same UE name", str(status_count(symbol_mirrors, "Same"))),
                ("TS exported symbols/functions with different/no same UE name", str(status_count(symbol_mirrors, "Different"))),
                ("UE symbols/functions without same TS name", str(len(ue_only_symbols))),
                ("Node CLI command keys", str(len(ts_keys))),
            ],
        ),
        "",
        f"- TS root: `{ts_root}`",
        f"- UE root: `{ue_root}`",
        "- `Same` means a UE folder/file/symbol with the same normalized name was found.",
        "- `Different` means no same normalized name was found; listed UE item(s), when present, are closest dynamic name candidates.",
        "- Function names are included in the generated symbol/function mirrors.",
        "",
        "### Generated Package Coverage",
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
            package_summary_rows(folder_mirrors, file_mirrors, symbol_mirrors),
        ),
        "",
        "### Generated Folder Name Mirrors",
        "",
    ]

    for package, mirrors in grouped(folder_mirrors, lambda mirror: package_for_ts_path(mirror.ts_folder)):
        lines.extend(
            [
                f"#### {package_label(package)} Folder Names",
                "",
                *table(
                    ("TypeScript folder", "TS name", "UE folder(s)", "Status", "Notes"),
                    folder_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
            "### Generated UE Folder Names Without Same TS Name",
            "",
            *table(
                ("UE folder", "UE name", "Notes"),
                [(md_code(path), md_code(path_name(path)), "No TS source folder currently has the same normalized name.") for path in ue_only_folders],
            ),
            "",
            "### Generated File Name Mirrors",
            "",
        ]
    )

    for package, mirrors in grouped(file_mirrors, lambda mirror: package_for_ts_path(mirror.ts_file)):
        lines.extend(
            [
                f"#### {package_label(package)} File Names",
                "",
                *table(
                    ("TypeScript file", "TS name", "UE file(s)", "Status", "Notes"),
                    file_mirror_rows(mirrors),
                ),
                "",
            ]
        )

    lines.extend(
        [
            "### Generated UE File Names Without Same TS Name",
            "",
            *table(
                ("UE file", "UE name", "Notes"),
                [(md_code(path), md_code(path_name(path)), "No TS source file currently has the same normalized name.") for path in ue_only_files],
            ),
            "",
            "### Generated Symbol And Function Name Mirrors",
            "",
        ]
    )

    for package, mirrors in grouped(symbol_mirrors, lambda mirror: package_for_ts_path(mirror.ts_symbol.file)):
        lines.extend(
            [
                f"#### {package_label(package)} Symbol And Function Names",
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
            "### Generated UE Symbol And Function Names Without Same TS Name",
            "",
            *table(
                ("UE symbol/function", "Kind", "Notes"),
                [
                    (
                        md_code(f"{symbol.name} ({symbol.file})"),
                        md(symbol.kind),
                        "No exported TS symbol/function currently has the same normalized name.",
                    )
                    for symbol in ue_only_symbols
                ],
            ),
            "",
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


def check_cli_command_parity(ts_root: Path, ue_root: Path) -> tuple[int, list[str], list[str]]:
    ts_matrix = ts_root / "packages/core/src/cliCommandMatrix.ts"
    ue_matrix = ue_root / "Source/ForbocAI_SDK/Public/CLI/CliCommandMatrix.h"

    ts_keys = extract_ts_node_keys(ts_matrix)
    ue_keys = extract_ue_node_keys(ue_matrix)

    missing = [key for key in ts_keys if key not in ue_keys]
    extra = [key for key in ue_keys if key not in ts_keys]
    order_matches = ts_keys == ue_keys

    print("[check] UE SDK parity")
    print(f"[info] TS SDK root: {ts_root}")
    print(f"[info] UE SDK root: {ue_root}")
    print(f"[info] TS CLI matrix: {ts_matrix}")
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
    print("[hint] Update Source/ForbocAI_SDK/Public/CLI/CliCommandMatrix.h")
    print("       so the BEGIN_NODE_CLI_COMMAND_KEYS block mirrors TS exactly.")
    return 1, ts_keys, ue_keys


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compare TS SDK and UE SDK folder, file, symbol, and CLI-key names."
    )
    parser.add_argument("--ts-sdk-root", help="Path to the TS SDK repo root.")
    parser.add_argument("--map-path", help="Path to classified/docs/design/sdk/map.md.")
    parser.add_argument("--runtime-root", help=argparse.SUPPRESS)
    parser.add_argument("--update-map", action="store_true", help="Rewrite the generated inventory section in map.md.")
    parser.add_argument("--require-map", action="store_true", help="Fail when map.md cannot be found.")
    args = parser.parse_args()

    ue_root = repo_root()
    ts_root = resolve_ts_root(ue_root, args.ts_sdk_root)
    map_path = resolve_map_path(ue_root, args.map_path)

    cli_status, ts_keys, ue_keys = check_cli_command_parity(ts_root, ue_root)

    ts_files = iter_ts_files(ts_root)
    ue_files = iter_ue_files(ue_root)
    ts_folders = iter_ts_folders(ts_files, ts_root)
    ue_folders = iter_ue_folders(ue_files, ue_root)
    folder_mirrors = build_folder_mirrors(ts_folders, ue_folders)
    file_mirrors = build_file_mirrors(ts_files, ts_root, ue_files, ue_root)

    ts_symbols = [symbol for path in ts_files for symbol in extract_ts_symbols(path, ts_root)]
    ue_symbols = [symbol for path in ue_files for symbol in extract_ue_symbols(path, ue_root)]
    symbol_mirrors = build_symbol_mirrors(ts_symbols, ue_symbols)

    section = build_generated_section(
        ts_root,
        ue_root,
        ts_folders,
        ue_folders,
        folder_mirrors,
        ts_files,
        ue_files,
        file_mirrors,
        ts_symbols,
        ue_symbols,
        symbol_mirrors,
        ts_keys,
        ue_keys,
    )

    print(f"[info] TS source folders scanned: {len(ts_folders)}")
    print(f"[info] UE source folders scanned: {len(ue_folders)}")
    print(f"[info] TS folders without same UE name: {len(folder_mirrors) - status_count(folder_mirrors, 'Same')}")
    print(f"[info] TS source files scanned: {len(ts_files)}")
    print(f"[info] UE source files scanned: {len(ue_files)}")
    print(f"[info] TS files without same UE name: {len(file_mirrors) - status_count(file_mirrors, 'Same')}")
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
        if args.update_map:
            map_path.write_text(updated, encoding="utf-8", newline="\n")
            print(f"[ok] Updated generated SDK parity inventory in {map_path}")
        elif original != updated:
            print(f"[fail] Generated SDK parity inventory is stale in {map_path}")
            print("[hint] Run: python3 scripts/check-sdk-parity.py --update-map")
            map_status = 1
        else:
            print(f"[ok] Generated SDK parity inventory is current in {map_path}")

    return 1 if cli_status or map_status else 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[fail] {exc}", file=sys.stderr)
        raise SystemExit(1)
