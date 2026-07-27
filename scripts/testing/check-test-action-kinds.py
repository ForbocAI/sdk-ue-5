#!/usr/bin/env python3
"""Validate semantic UE reducer-test action names against their C++ enums."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import sys
from typing import Any


ENUM_PATTERN = re.compile(
    r"enum\s+class\s+E(?P<domain>[A-Za-z0-9_]+)TestActionKind\s*"
    r":\s*uint8\s*\{(?P<body>.*?)\};",
    re.DOTALL,
)
IDENTIFIER_PATTERN = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$")


def parse_args() -> argparse.Namespace:
    """User Story: As a maintainer, I need a portable root override for this strict check. Signature: parse_args() -> argparse.Namespace."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--plugin-root",
        type=Path,
        default=Path(__file__).resolve().parents[2],
        help="UE SDK plugin root",
    )
    return parser.parse_args()


def strip_cpp_comments(text: str) -> str:
    """User Story: As an enum reader, I need comments excluded from semantic members. Signature: strip_cpp_comments(text: str) -> str."""
    without_blocks = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//.*", "", without_blocks)


def enum_members(header: Path) -> tuple[str, tuple[str, ...]]:
    """User Story: As a fixture validator, I need a domain and its actions discovered from C++. Signature: enum_members(header: Path) -> tuple[str, tuple[str, ...]]."""
    match = ENUM_PATTERN.search(header.read_text(encoding="utf-8"))
    if match is None:
        raise ValueError(f"{header}: reducer-test action enum not found")
    members = tuple(
        token
        for raw_member in strip_cpp_comments(match.group("body")).split(",")
        if (token := raw_member.split("=", maxsplit=1)[0].strip())
        and IDENTIFIER_PATTERN.fullmatch(token)
        and token != "Count"
    )
    if not members:
        raise ValueError(f"{header}: reducer-test action enum is empty")
    return match.group("domain"), members


def action_kinds(value: Any) -> tuple[Any, ...]:
    """User Story: As a JSON fixture consumer, I need every nested action kind collected without layout assumptions. Signature: action_kinds(value: Any) -> tuple[Any, ...]."""
    if isinstance(value, dict):
        current = (value["kind"],) if "kind" in value else ()
        children = tuple(
            kind
            for child in value.values()
            for kind in action_kinds(child)
        )
        return current + children
    if isinstance(value, list):
        return tuple(kind for child in value for kind in action_kinds(child))
    return ()


def fixture_kinds(data_root: Path) -> tuple[Any, ...]:
    """User Story: As a test-domain validator, I need action names from every authored JSON fixture. Signature: fixture_kinds(data_root: Path) -> tuple[Any, ...]."""
    json_files = tuple(sorted(data_root.rglob("*.json")))
    if not json_files:
        raise ValueError(f"{data_root}: no JSON fixtures found")
    return tuple(
        kind
        for path in json_files
        for kind in action_kinds(json.loads(path.read_text(encoding="utf-8")))
    )


def validate_domain(
    domain: str, members: tuple[str, ...], data_root: Path
) -> tuple[str, ...]:
    """User Story: As a reducer-test owner, I need nonsemantic, unknown, and uncovered actions rejected. Signature: validate_domain(domain: str, members: tuple[str, ...], data_root: Path) -> tuple[str, ...]."""
    kinds = fixture_kinds(data_root)
    nonsemantic = tuple(repr(kind) for kind in kinds if not isinstance(kind, str))
    authored = frozenset(kind for kind in kinds if isinstance(kind, str))
    expected = frozenset(members)
    findings = (
        tuple(f"{domain}: action kind must be a semantic string, found {kind}" for kind in nonsemantic)
        + tuple(f"{domain}: unknown action kind {kind}" for kind in sorted(authored - expected))
        + tuple(f"{domain}: action kind has no fixture coverage: {kind}" for kind in sorted(expected - authored))
    )
    return findings


def main() -> int:
    """User Story: As CI, I need all discovered reducer-test fixture contracts checked in one command. Signature: main() -> int."""
    plugin_root = parse_args().plugin_root.resolve()
    testing_root = (
        plugin_root
        / "Source"
        / "ForbocAI_SDK"
        / "Public"
        / "Components"
        / "Testing"
    )
    headers = tuple(sorted(testing_root.glob("*/Testing*Types.h")))
    if not headers:
        print(f"[fail] {testing_root}: no reducer-test type headers found", file=sys.stderr)
        return 1

    contracts = tuple(enum_members(header) for header in headers if ENUM_PATTERN.search(header.read_text(encoding="utf-8")))
    findings = tuple(
        finding
        for domain, members in contracts
        for finding in validate_domain(
            domain,
            members,
            plugin_root / "Content" / "Data" / "tests" / domain.lower(),
        )
    )
    if findings:
        print("[fail] Reducer-test action fixture contract violations:", file=sys.stderr)
        print("\n".join(f"  {finding}" for finding in findings), file=sys.stderr)
        return 1
    print(
        "[ok] Reducer-test actions use semantic names with complete enum coverage "
        f"({len(contracts)} domains, {sum(len(members) for _, members in contracts)} actions)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
