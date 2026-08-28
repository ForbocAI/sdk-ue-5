from __future__ import annotations

import re
from collections.abc import Iterable
from pathlib import Path

from parity.source.comments import strip_comments


def local_include_closure(entrypoint: Path, include_root: Path) -> tuple[Path, ...]:
    visited: set[Path] = set()

    def visit(path: Path) -> None:
        resolved = path.resolve()
        contained = resolved.is_file() and include_root.resolve() in resolved.parents
        if resolved in visited or not contained:
            return
        visited.add(resolved)
        source = strip_comments(resolved.read_text(encoding="utf-8"))
        includes = re.findall(r'^\s*#include\s+"([^"]+)"', source, re.M)
        dependencies = (
            next(
                (
                    candidate
                    for candidate in (include_root / include, resolved.parent / include)
                    if candidate.is_file()
                ),
                None,
            )
            for include in includes
        )
        for dependency in dependencies:
            if dependency is not None:
                visit(dependency)

    visit(entrypoint)
    return tuple(sorted(visited))


def names_in_source(path: Path) -> tuple[str, ...]:
    text = strip_comments(path.read_text(encoding="utf-8"))
    names: set[str] = set()
    patterns = (
        r"\b(?:struct|class)\s+([A-Za-z_][A-Za-z0-9_]*)",
        r"\benum\s+(?:class\s+)?([A-Za-z_][A-Za-z0-9_]*)",
        r"\busing\s+([A-Za-z_][A-Za-z0-9_]*)\s*=",
        r"\btypedef\b[^;{}]*\s+([A-Za-z_][A-Za-z0-9_]*)\s*;",
        r"\b(?:inline\s+)?(?:static\s+)?(?:constexpr\s+)?(?:const\s+)?[A-Za-z_][A-Za-z0-9_:<>,\s\*&]*\s+\*?([A-Za-z_][A-Za-z0-9_]*)\s*(?:=|;)",
    )
    for pattern in patterns:
        names.update(re.findall(pattern, text))
    excluded = {"if", "for", "while", "switch", "return", "sizeof", "TEXT", "check"}
    names.update(
        name
        for name in re.findall(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\(", text)
        if name not in excluded
    )
    return tuple(sorted(names))


def name_index(paths: Iterable[Path], root: Path) -> dict[str, tuple[str, ...]]:
    pairs = (
        (name, path.relative_to(root).as_posix())
        for path in paths
        for name in names_in_source(path)
    )
    index: dict[str, list[str]] = {}
    for name, path in pairs:
        index.setdefault(name, []).append(path)
    return {name: tuple(sorted(set(values))) for name, values in index.items()}
