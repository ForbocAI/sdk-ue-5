#!/usr/bin/env python3
"""Strict first-party C++ source-structure guard for every UE target."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from ue_targets import UeTarget, ue_targets


SOURCE_SUFFIXES = {".cpp", ".h", ".hpp"}
HEADER_SUFFIXES = {".h", ".hpp"}
EXCLUDED_PARTS = {"Binaries", "Intermediate", "Saved", "ThirdParty"}
LOCAL_INCLUDE_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.MULTILINE)
PRAGMA_ONCE_RE = re.compile(r"^\s*#\s*pragma\s+once\b", re.MULTILINE)
HEADER_GUARD_RE = re.compile(
    r"^\s*#\s*ifndef\s+([A-Za-z_]\w*)\s*$"
    r"[\s\S]*?^\s*#\s*define\s+\1\b",
    re.MULTILINE,
)
NAMESPACE_BEFORE_BRACE_RE = re.compile(
    r"\bnamespace(?:\s+([A-Za-z_]\w*(?:\s*::\s*[A-Za-z_]\w*)*))?\s*$"
)


@dataclass(frozen=True)
class Brace:
    line: int
    column: int
    namespace: str | None


@dataclass(frozen=True)
class Violation:
    code: str
    path: Path
    line: int
    message: str


def first_party_sources(target: UeTarget) -> list[Path]:
    source_root = target.root / "Source"
    return sorted(
        path
        for path in source_root.rglob("*")
        if path.is_file()
        and path.suffix in SOURCE_SUFFIXES
        and not any(part in EXCLUDED_PARTS for part in path.parts)
    )


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8", errors="replace")


def without_comments_and_literals(source: str) -> str:
    """Replace comments and literals with spaces while preserving newlines."""

    output: list[str] = []
    index = 0
    length = len(source)
    state = "code"

    while index < length:
        char = source[index]
        following = source[index + 1] if index + 1 < length else ""

        if state == "code":
            if char == "/" and following == "/":
                output.extend((" ", " "))
                index += 2
                state = "line_comment"
                continue
            if char == "/" and following == "*":
                output.extend((" ", " "))
                index += 2
                state = "block_comment"
                continue
            if char == 'R' and following == '"':
                delimiter_end = source.find("(", index + 2, index + 20)
                if delimiter_end != -1:
                    delimiter = source[index + 2 : delimiter_end]
                    terminator = ")" + delimiter + '"'
                    literal_end = source.find(terminator, delimiter_end + 1)
                    if literal_end != -1:
                        end = literal_end + len(terminator)
                        output.extend(
                            "\n" if value == "\n" else " "
                            for value in source[index:end]
                        )
                        index = end
                        continue
            if char == '"':
                output.append(" ")
                index += 1
                state = "string"
                continue
            if char == "'":
                output.append(" ")
                index += 1
                state = "character"
                continue
            output.append(char)
            index += 1
            continue

        if state == "line_comment":
            output.append("\n" if char == "\n" else " ")
            index += 1
            if char == "\n":
                state = "code"
            continue

        if state == "block_comment":
            if char == "*" and following == "/":
                output.extend((" ", " "))
                index += 2
                state = "code"
                continue
            output.append("\n" if char == "\n" else " ")
            index += 1
            continue

        if state in {"string", "character"}:
            delimiter = '"' if state == "string" else "'"
            if char == "\\" and index + 1 < length:
                output.extend((" ", "\n" if following == "\n" else " "))
                index += 2
                continue
            output.append("\n" if char == "\n" else " ")
            index += 1
            if char == delimiter:
                state = "code"

    return "".join(output)


def line_and_column(source: str, index: int) -> tuple[int, int]:
    line = source.count("\n", 0, index) + 1
    line_start = source.rfind("\n", 0, index) + 1
    return line, index - line_start + 1


def source_brace_violations(path: Path, source: str) -> list[Violation]:
    cleaned = without_comments_and_literals(source)
    stack: list[Brace] = []
    violations: list[Violation] = []

    for index, char in enumerate(cleaned):
        if char == "{":
            line, column = line_and_column(cleaned, index)
            line_start = cleaned.rfind("\n", 0, index) + 1
            prefix = cleaned[line_start:index]
            namespace_match = NAMESPACE_BEFORE_BRACE_RE.search(prefix)
            namespace = (
                re.sub(r"\s+", "", namespace_match.group(1))
                if namespace_match and namespace_match.group(1)
                else ""
                if namespace_match
                else None
            )
            active_namespaces = [
                brace.namespace
                for brace in stack
                if brace.namespace not in {None, ""}
            ]
            if namespace and active_namespaces:
                first_segment = namespace.split("::", 1)[0]
                last_active = active_namespaces[-1].split("::")[-1]
                if first_segment == last_active:
                    violations.append(
                        Violation(
                            "CPP-NAMESPACE-002",
                            path,
                            line,
                            f"namespace {namespace} repeats active namespace {last_active}",
                        )
                    )
            stack.append(Brace(line, column, namespace))
            continue

        if char == "}":
            line, _ = line_and_column(cleaned, index)
            if stack:
                stack.pop()
            else:
                violations.append(
                    Violation(
                        "CPP-STRUCT-001",
                        path,
                        line,
                        "unmatched closing brace",
                    )
                )

    violations.extend(
        Violation(
            "CPP-STRUCT-002",
            path,
            brace.line,
            "unclosed namespace" if brace.namespace is not None else "unclosed brace",
        )
        for brace in stack
    )
    return violations


def header_guard_violations(path: Path, source: str) -> list[Violation]:
    if path.suffix not in HEADER_SUFFIXES:
        return []
    if PRAGMA_ONCE_RE.search(source) or HEADER_GUARD_RE.search(source):
        return []
    return [
        Violation(
            "CPP-HEADER-001",
            path,
            1,
            "first-party header has no #pragma once or include guard",
        )
    ]


def companion_header(target: UeTarget, path: Path) -> Path | None:
    for suffix in HEADER_SUFFIXES:
        candidate = path.with_suffix(suffix)
        if candidate.is_file():
            return candidate
    return None


def include_order_violations(
    target: UeTarget, path: Path, source: str
) -> list[Violation]:
    if path.suffix != ".cpp":
        return []
    companion = companion_header(target, path)
    if companion is None:
        return []
    includes = LOCAL_INCLUDE_RE.findall(source)
    if not includes:
        return [
            Violation(
                "CPP-INCLUDE-001",
                path,
                1,
                f"translation unit does not include companion header {companion.name}",
            )
        ]
    relative_parts = companion.relative_to(target.root / "Source").parts
    boundary_indexes = [
        relative_parts.index(boundary)
        for boundary in ("Public", "Private")
        if boundary in relative_parts
    ]
    expected_parts = (
        relative_parts[min(boundary_indexes) + 1 :]
        if boundary_indexes
        else relative_parts
    )
    expected = Path(*expected_parts).as_posix()
    if includes[0] in {expected, companion.name}:
        return []
    return [
        Violation(
            "CPP-INCLUDE-001",
            path,
            1,
            f"first local include is {includes[0]}, expected {expected}",
        )
    ]


def target_violations(target: UeTarget) -> list[Violation]:
    violations: list[Violation] = []
    for path in first_party_sources(target):
        source = read(path)
        violations.extend(source_brace_violations(path, source))
        violations.extend(header_guard_violations(path, source))
        violations.extend(include_order_violations(target, path, source))
    return violations


def display_path(target: UeTarget, path: Path) -> str:
    return path.relative_to(target.root).as_posix()


def main() -> int:
    total = 0
    for target in ue_targets():
        violations = target_violations(target)
        total += len(violations)
        if not violations:
            print(f"C++ source structure passed for {target.label}.")
            continue
        print(f"C++ source structure failed for {target.label}: {len(violations)} issue(s)")
        for violation in violations:
            print(
                f"  {violation.code} {display_path(target, violation.path)}:"
                f"{violation.line} {violation.message}"
            )

    if total:
        print(f"C++ source structure failed: {total} total issue(s).")
        return 1
    print("C++ source structure passed for all discovered UE targets.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
