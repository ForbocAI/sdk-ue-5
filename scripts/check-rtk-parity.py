#!/usr/bin/env python3
"""Check the UE Core/rtk.hpp surface against installed Toolkit exports.

This is a source-derived parity check. It reads the TS SDK's installed
@reduxjs/toolkit package, extracts public Toolkit and RTK Query export names,
walks every local header transitively included by UE Core/rtk.hpp, updates the
SDK map, and fails while any exported name is missing from that public surface.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


GENERATED_START = "<!-- BEGIN GENERATED RTK HPP PARITY -->"
GENERATED_END = "<!-- END GENERATED RTK HPP PARITY -->"


@dataclass(frozen=True)
class ExportGroup:
    label: str
    names: tuple[str, ...]


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def strip_comments(text: str) -> str:
    output: list[str] = []
    index = 0
    in_block_comment = False
    in_line_comment = False
    in_single_quote = False
    in_double_quote = False
    escaped = False

    while index < len(text):
        current = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""

        if in_block_comment:
            if current == "\n":
                output.append(current)
            if current == "*" and next_char == "/":
                in_block_comment = False
                index += 2
            else:
                index += 1
            continue

        if in_line_comment:
            if current == "\n":
                in_line_comment = False
                output.append(current)
            index += 1
            continue

        if escaped:
            output.append(current)
            escaped = False
            index += 1
            continue

        if (in_single_quote or in_double_quote) and current == "\\":
            output.append(current)
            escaped = True
            index += 1
            continue

        if not in_single_quote and current == '"':
            in_double_quote = not in_double_quote
            output.append(current)
            index += 1
            continue

        if not in_double_quote and current == "'":
            in_single_quote = not in_single_quote
            output.append(current)
            index += 1
            continue

        if not in_single_quote and not in_double_quote:
            if current == "/" and next_char == "*":
                in_block_comment = True
                index += 2
                continue
            if current == "/" and next_char == "/":
                in_line_comment = True
                index += 2
                continue

        output.append(current)
        index += 1

    return "".join(output)


def candidate_ts_roots(root: Path) -> list[Path]:
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    roots = [Path(env_root)] if env_root else []
    return roots + [root.parent / "sdk", root.parent / "sdk-ts"]


def resolve_ts_root(root: Path, explicit_root: str | None) -> Path:
    roots = [Path(explicit_root)] if explicit_root else candidate_ts_roots(root)
    for ts_root in roots:
        if (ts_root / "node_modules/@reduxjs/toolkit/package.json").is_file():
            return ts_root
    searched = "\n".join(
        f"  - {candidate / 'node_modules/@reduxjs/toolkit/package.json'}"
        for candidate in roots
    )
    raise FileNotFoundError(
        "Could not find installed @reduxjs/toolkit. Set FORBOCAI_TS_SDK_ROOT "
        f"or pass --ts-sdk-root.\nSearched:\n{searched}"
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


def toolkit_version(ts_root: Path) -> str:
    package_json = json.loads(
        read_text(ts_root / "node_modules/@reduxjs/toolkit/package.json")
    )
    return str(package_json.get("version", "unknown"))


def node_runtime_exports(ts_root: Path, specifier: str) -> tuple[str, ...]:
    code = (
        "const mod = require(process.argv[1]);"
        "console.log(JSON.stringify(Object.keys(mod).sort()))"
    )
    result = subprocess.run(
        ["node", "-e", code, specifier],
        cwd=ts_root,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    values = json.loads(result.stdout)
    return tuple(value for value in values if isinstance(value, str))


def exported_name(raw: str) -> str | None:
    value = raw.strip()
    if not value:
        return None
    value = re.sub(r"\s+as\s+", " as ", value)
    if " as " in value:
        value = value.split(" as ", 1)[1].strip()
    value = value.split(":", 1)[-1].strip()
    return value if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", value) else None


def split_export_block(body: str) -> tuple[str, ...]:
    names: list[str] = []
    for raw in body.split(","):
        name = exported_name(raw)
        if name:
            names.append(name)
    return tuple(sorted(set(names)))


def source_exports(path: Path) -> tuple[str, ...]:
    text = strip_comments(read_text(path))
    names: list[str] = []
    for match in re.finditer(
        r"\bexport\s+(?:type\s+)?\{(?P<body>.*?)\}\s+from\b", text, re.S
    ):
        names.extend(split_export_block(match.group("body")))
    for pattern in (
        r"\bexport\s+(?:type\s+)?(?:interface|class|function|const|enum)\s+([A-Za-z_][A-Za-z0-9_]*)",
        r"\bexport\s+type\s+([A-Za-z_][A-Za-z0-9_]*)",
    ):
        names.extend(re.findall(pattern, text))
    return tuple(sorted(set(names)))


def local_include_closure(entrypoint: Path, include_root: Path) -> tuple[Path, ...]:
    """Return the source-derived local header closure exposed by an entrypoint."""
    visited: set[Path] = set()

    def visit(path: Path) -> None:
        """Visit one local header and recursively follow its quoted includes."""
        resolved = path.resolve()
        if resolved in visited or not resolved.is_file():
            return
        try:
            resolved.relative_to(include_root.resolve())
        except ValueError:
            return
        visited.add(resolved)
        source = strip_comments(read_text(resolved))
        for include in re.findall(r'^\s*#include\s+"([^"]+)"', source, re.M):
            candidates = (include_root / include, resolved.parent / include)
            dependency = next(
                (candidate for candidate in candidates if candidate.is_file()),
                None,
            )
            if dependency is not None:
                visit(dependency)

    visit(entrypoint)
    return tuple(sorted(visited))


def rtk_hpp_names(paths: Iterable[Path]) -> tuple[str, ...]:
    """Extract public C++ names from the complete rtk.hpp include closure."""
    text = strip_comments("\n".join(read_text(path) for path in paths))
    names: set[str] = set()
    for pattern in (
        r"\b(?:struct|class)\s+([A-Za-z_][A-Za-z0-9_]*)",
        r"\benum\s+(?:class\s+)?([A-Za-z_][A-Za-z0-9_]*)",
        r"\busing\s+([A-Za-z_][A-Za-z0-9_]*)\s*=",
        r"\btypedef\b[^;{}]*\s+([A-Za-z_][A-Za-z0-9_]*)\s*;",
        r"\b(?:inline\s+)?(?:static\s+)?(?:constexpr\s+)?(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>,\s\*&]*\s+\*?([A-Za-z_][A-Za-z0-9_]*)\s*(?:=|;)",
    ):
        names.update(re.findall(pattern, text))
    for match in re.finditer(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", text):
        name = match.group(1)
        if name not in {
            "if",
            "for",
            "while",
            "switch",
            "return",
            "sizeof",
            "TEXT",
            "check",
        }:
            names.add(name)
    return tuple(sorted(names))


def table(headers: tuple[str, ...], rows: Iterable[tuple[str, ...]]) -> list[str]:
    lines = [
        "| " + " | ".join(headers) + " |",
        "| " + " | ".join(":---" for _ in headers) + " |",
    ]
    lines.extend("| " + " | ".join(row) + " |" for row in rows)
    return lines


def md_code(value: str) -> str:
    return "`" + value.replace("`", "\\`") + "`"


def repository_identity(root: Path) -> str:
    """User Story: Identify a discovered repository without exposing its host path.

    Signature: (Path) -> str.
    """
    return root.name


def replace_generated_section(text: str, section: str) -> str:
    if GENERATED_START in text and GENERATED_END in text:
        pattern = re.compile(
            re.escape(GENERATED_START) + r".*?" + re.escape(GENERATED_END) + r"\n?",
            re.S,
        )
        return pattern.sub(section, text)
    insert_before = "\n<!-- BEGIN GENERATED SDK PARITY INVENTORY -->"
    if insert_before in text:
        return text.replace(insert_before, "\n" + section + insert_before, 1)
    return text.rstrip() + "\n\n" + section


def build_section(
    version: str,
    ts_root: Path,
    ue_root: Path,
    ue_sources: tuple[Path, ...],
    groups: tuple[ExportGroup, ...],
    ue_names: tuple[str, ...],
) -> tuple[str, int]:
    ue_name_set = set(ue_names)
    rows: list[tuple[str, str, str, str]] = []
    missing_total = 0
    lines = [
        GENERATED_START,
        "## Generated RTK HPP Parity",
        "",
        "Generated by `scripts/check-rtk-parity.py` from the installed Toolkit and UE source closure named below.",
        "",
        f"- Toolkit source: `@reduxjs/toolkit` {version} from TS repository `{repository_identity(ts_root)}`",
        "- UE mirror entrypoint: "
        "`Source/ForbocAI_SDK/Public/Core/rtk.hpp` in UE repository "
        f"`{repository_identity(ue_root)}`",
        f"- UE mirror source closure: {len(ue_sources)} transitively included local headers",
        "- This section is export-name derived. It is intentionally strict: the `rtk.hpp` public surface is not exact while any exported Toolkit or RTK Query name is missing.",
        "",
    ]
    for group in groups:
        present = [name for name in group.names if name in ue_name_set]
        missing = [name for name in group.names if name not in ue_name_set]
        missing_total += len(missing)
        rows.append(
            (
                group.label,
                str(len(group.names)),
                str(len(present)),
                str(len(missing)),
            )
        )

    lines.extend(table(("Export group", "Expected", "Present", "Missing"), rows))
    lines.append("")

    for group in groups:
        missing = [name for name in group.names if name not in ue_name_set]
        lines.extend([f"### Missing {group.label}", ""])
        if missing:
            lines.extend("- " + md_code(name) for name in missing)
        else:
            lines.append("- None")
        lines.append("")

    lines.append(GENERATED_END)
    return "\n".join(lines) + "\n", missing_total


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ts-sdk-root", help="Path to the TS SDK repo root.")
    parser.add_argument("--map-path", help="Path to classified/docs/design/sdk/map.md.")
    args = parser.parse_args()

    ue_root = repo_root()
    ts_root = resolve_ts_root(ue_root, args.ts_sdk_root)
    rtk_hpp = ue_root / "Source/ForbocAI_SDK/Public/Core/rtk.hpp"
    public_root = ue_root / "Source/ForbocAI_SDK/Public"
    package_root = ts_root / "node_modules/@reduxjs/toolkit/src"
    version = toolkit_version(ts_root)

    groups = (
        ExportGroup("@reduxjs/toolkit runtime", node_runtime_exports(ts_root, "@reduxjs/toolkit")),
        ExportGroup("@reduxjs/toolkit source exports", source_exports(package_root / "index.ts")),
        ExportGroup("@reduxjs/toolkit/query runtime", node_runtime_exports(ts_root, "@reduxjs/toolkit/query")),
        ExportGroup("@reduxjs/toolkit/query source exports", source_exports(package_root / "query/index.ts")),
    )
    from generate_authored_values import generated_output_paths

    generated = generated_output_paths()
    ue_sources = tuple(
        path
        for path in local_include_closure(rtk_hpp, public_root)
        if path not in generated
    )
    ue_names = rtk_hpp_names(ue_sources)
    section, missing_total = build_section(
        version, ts_root, ue_root, ue_sources, groups, ue_names
    )

    map_path = resolve_map_path(ue_root, args.map_path)
    if map_path and map_path.exists():
        original = read_text(map_path)
        updated = replace_generated_section(original, section)
        map_path.write_text(updated, encoding="utf-8", newline="\n")
        print(f"[ok] Updated RTK HPP parity inventory in {map_path}")
    else:
        print("[warn] map.md not found; RTK HPP parity inventory was not written.")

    print(f"[info] @reduxjs/toolkit version: {version}")
    print(f"[info] UE rtk.hpp source headers discovered: {len(ue_sources)}")
    print(f"[info] UE rtk.hpp public names discovered: {len(ue_names)}")
    for group in groups:
        missing = [name for name in group.names if name not in set(ue_names)]
        print(
            f"[info] {group.label}: expected={len(group.names)} "
            f"present={len(group.names) - len(missing)} missing={len(missing)}"
        )
    if missing_total:
        print(
            "[fail] rtk.hpp public surface is missing "
            f"{missing_total} exported Redux Toolkit / RTK Query names."
        )
        return 1
    print("[ok] rtk.hpp mirrors exported Redux Toolkit / RTK Query names.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
