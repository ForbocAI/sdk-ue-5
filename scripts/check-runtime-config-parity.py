#!/usr/bin/env python3
"""Verify that TS and UE core own one equivalent endpoint-selection contract."""

from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any, Iterable


UE_RUNTIME_RELPATH = Path("Content/Data/config/runtime.json")
TS_RUNTIME_RELPATH = Path("packages/core/data/config/runtime.json")
UE_SOURCE_RELPATH = Path("Source/ForbocAI_SDK")
TS_SOURCE_RELPATH = Path("packages/core/src")
UE_SCRIPT_DEFAULT_PATHS = (
    Path("scripts/lib/test-environment.sh"),
    Path("scripts/lib/TestEnvironment.ps1"),
)
SOURCE_SUFFIXES = frozenset({".cpp", ".h", ".hpp", ".ts", ".tsx"})
IGNORED_SOURCE_PARTS = frozenset({"__tests__", "Tests", "ThirdParty"})


def ue_repo_root() -> Path:
    """Resolve the UE SDK plugin root from this script's location."""
    return Path(__file__).resolve().parents[1]


def resolve_ts_root(ue_root: Path) -> Path:
    """Locate the sibling TS SDK repo from an override or authored contract."""
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    candidates = [Path(env_root)] if env_root else []
    candidates.extend(
        sorted(
            (
                sibling
                for sibling in ue_root.parent.iterdir()
                if sibling.is_dir() and sibling != ue_root
            ),
            key=lambda candidate: candidate.name.casefold(),
        )
    )
    matching = next(
        (
            candidate
            for candidate in candidates
            if (candidate / TS_RUNTIME_RELPATH).is_file()
        ),
        None,
    )
    if matching is not None:
        return matching
    searched = "\n".join(f"  - {candidate}" for candidate in candidates)
    raise FileNotFoundError(
        "Could not find the TS core runtime contract. Set FORBOCAI_TS_SDK_ROOT."
        f"\nSearched:\n{searched}"
    )


def load_json(path: Path) -> dict[str, Any]:
    """Read one authored JSON object."""
    return json.loads(path.read_text(encoding="utf-8"))


def ts_contract(runtime: dict[str, Any]) -> dict[str, Any]:
    """Project the TS runtime document onto the shared endpoint contract."""
    return {
        "localApiUrl": runtime["localApiUrl"],
        "productionApiUrl": runtime["productionApiUrl"],
        "statusPath": runtime["statusPath"],
        "availabilityTimeoutMs": runtime["availabilityTimeoutMs"],
        "methods": runtime["methods"],
        "sources": runtime["sources"],
    }


def ue_contract(runtime: dict[str, Any]) -> dict[str, Any]:
    """Project the UE runtime document onto the shared endpoint contract."""
    connection = runtime["connection"]
    return {
        "localApiUrl": connection["localApiUrl"],
        "productionApiUrl": runtime["defaults"]["apiUrl"],
        "statusPath": connection["statusPath"],
        "availabilityTimeoutMs": connection["availabilityTimeoutMs"],
        "methods": connection["methods"],
        "sources": connection["sources"],
    }


def source_files(root: Path) -> Iterable[Path]:
    """Yield first-party runtime source files from one core root."""
    return (
        path
        for path in root.rglob("*")
        if path.is_file()
        and path.suffix in SOURCE_SUFFIXES
        and not any(part in IGNORED_SOURCE_PARTS for part in path.parts)
    )


def literal_hits(root: Path, literal: str) -> list[Path]:
    """Find runtime source files that duplicate an authored endpoint literal."""
    return [
        path
        for path in source_files(root)
        if literal in path.read_text(encoding="utf-8", errors="ignore")
    ]


def main() -> int:
    """Compare endpoint behavior and enforce core-owned URL defaults."""
    ue_root = ue_repo_root()
    ts_root = resolve_ts_root(ue_root)
    ue_path = ue_root / UE_RUNTIME_RELPATH
    ts_path = ts_root / TS_RUNTIME_RELPATH
    ue_values = ue_contract(load_json(ue_path))
    ts_values = ts_contract(load_json(ts_path))
    failures: list[str] = []

    for field in ts_values:
        if ts_values[field] == ue_values[field]:
            print(f"[ok] {field}: {ts_values[field]}")
        else:
            failures.append(
                f"{field}: TS {ts_values[field]!r} != UE {ue_values[field]!r}"
            )

    production_url = ts_values["productionApiUrl"]
    authority_roots = (
        ("TS", ts_root / TS_SOURCE_RELPATH),
        ("UE", ue_root / UE_SOURCE_RELPATH),
    )
    for label, source_root in authority_roots:
        hits = literal_hits(source_root, production_url)
        if hits:
            rendered = ", ".join(str(path.relative_to(source_root)) for path in hits)
            failures.append(
                f"{label} runtime source duplicates the production URL: {rendered}"
            )
        else:
            print(f"[ok] {label} runtime source reads the production URL from core data")

    script_hits = [
        path
        for path in UE_SCRIPT_DEFAULT_PATHS
        if production_url
        in (ue_root / path).read_text(encoding="utf-8", errors="ignore")
    ]
    if script_hits:
        failures.append(
            "UE script helpers duplicate the production URL: "
            + ", ".join(str(path) for path in script_hits)
        )
    else:
        print("[ok] UE script helpers do not assign an endpoint default")

    if failures:
        print("\nRuntime endpoint parity FAILED:")
        for failure in failures:
            print(f"  - {failure}")
        return 1

    print("\nRuntime endpoint parity OK: TS and UE share one core contract.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
