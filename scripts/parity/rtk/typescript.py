from __future__ import annotations

import json
import os
import re
import subprocess
from pathlib import Path

from parity.rtk.types import ExportGroup
from parity.source.comments import strip_comments


def candidate_ts_roots(root: Path) -> list[Path]:
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    roots = [Path(env_root)] if env_root else []
    return roots + [root.parent / "sdk", root.parent / "sdk-ts"]


def resolve_ts_root(root: Path, explicit_root: str | None) -> Path:
    roots = [Path(explicit_root)] if explicit_root else candidate_ts_roots(root)
    package_paths = [
        candidate / "node_modules/@reduxjs/toolkit/package.json"
        for candidate in roots
    ]
    match = next(
        (
            candidate
            for candidate, package_path in zip(roots, package_paths)
            if package_path.is_file()
        ),
        None,
    )
    if match is not None:
        return match.resolve()
    searched = "\n".join(f"  - {path}" for path in package_paths)
    raise FileNotFoundError(
        "Could not find installed @reduxjs/toolkit. Set FORBOCAI_TS_SDK_ROOT "
        f"or pass --ts-sdk-root.\nSearched:\n{searched}"
    )


def toolkit_version(ts_root: Path) -> str:
    package_path = ts_root / "node_modules/@reduxjs/toolkit/package.json"
    package = json.loads(package_path.read_text(encoding="utf-8"))
    return str(package.get("version", "unknown"))


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


def node_resolved_module(ts_root: Path, specifier: str) -> Path:
    result = subprocess.run(
        ["node", "-e", "console.log(require.resolve(process.argv[1]))", specifier],
        cwd=ts_root,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    return Path(result.stdout.strip()).resolve()


def exported_name(raw: str) -> str | None:
    value = re.sub(r"\s+as\s+", " as ", raw.strip())
    value = value.split(" as ", 1)[-1].strip() if " as " in value else value
    value = value.split(":", 1)[-1].strip()
    return value if re.match(r"^[A-Za-z_][A-Za-z0-9_]*$", value) else None


def split_export_block(body: str) -> tuple[str, ...]:
    names = [exported_name(raw) for raw in body.split(",")]
    return tuple(sorted({name for name in names if name is not None}))


def source_exports(path: Path) -> tuple[str, ...]:
    text = strip_comments(path.read_text(encoding="utf-8"))
    names: list[str] = []
    names.extend(
        name
        for match in re.finditer(
            r"\bexport\s+(?:type\s+)?\{(?P<body>.*?)\}\s+from\b", text, re.S
        )
        for name in split_export_block(match.group("body"))
    )
    for pattern in (
        r"\bexport\s+(?:type\s+)?(?:interface|class|function|const|enum)\s+([A-Za-z_][A-Za-z0-9_]*)",
        r"\bexport\s+type\s+([A-Za-z_][A-Za-z0-9_]*)",
    ):
        names.extend(re.findall(pattern, text))
    return tuple(sorted(set(names)))


def build_export_groups(ts_root: Path) -> tuple[tuple[ExportGroup, ...], tuple[Path, ...]]:
    source_root = ts_root / "node_modules/@reduxjs/toolkit/src"
    index = source_root / "index.ts"
    query_index = source_root / "query/index.ts"
    toolkit_module = node_resolved_module(ts_root, "@reduxjs/toolkit")
    query_module = node_resolved_module(ts_root, "@reduxjs/toolkit/query")
    groups = (
        ExportGroup("toolkit-runtime", "@reduxjs/toolkit runtime", node_runtime_exports(ts_root, "@reduxjs/toolkit"), (toolkit_module.relative_to(ts_root).as_posix(),)),
        ExportGroup("toolkit-source", "@reduxjs/toolkit source exports", source_exports(index), (index.relative_to(ts_root).as_posix(),)),
        ExportGroup("query-runtime", "@reduxjs/toolkit/query runtime", node_runtime_exports(ts_root, "@reduxjs/toolkit/query"), (query_module.relative_to(ts_root).as_posix(),)),
        ExportGroup("query-source", "@reduxjs/toolkit/query source exports", source_exports(query_index), (query_index.relative_to(ts_root).as_posix(),)),
    )
    package_json = ts_root / "node_modules/@reduxjs/toolkit/package.json"
    return groups, (package_json, index, query_index, toolkit_module, query_module)


def tracked_contract_paths(ts_root: Path) -> tuple[Path, ...]:
    candidates = (
        ts_root / "package.json",
        ts_root / "bun.lock",
        ts_root / "package-lock.json",
    )
    return tuple(path for path in candidates if path.is_file())
