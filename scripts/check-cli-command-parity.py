#!/usr/bin/env python3
"""Verify UE CLI command keys mirror the TS SDK Node CLI matrix."""

from __future__ import annotations

import argparse
import os
import re
import sys
from pathlib import Path


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def candidate_ts_roots(root: Path) -> list[Path]:
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    roots = [Path(env_root)] if env_root else []
    return roots + [
        root.parent / "sdk",
        root.parent / "sdk-ts",
        root.parent / "sdk-node",
    ]


def resolve_ts_matrix(root: Path, explicit_root: str | None) -> Path:
    roots = [Path(explicit_root)] if explicit_root else candidate_ts_roots(root)
    for ts_root in roots:
        matrix = ts_root / "packages" / "core" / "src" / "cliCommandMatrix.ts"
        if matrix.is_file():
            return matrix

    searched = "\n".join(
        f"  - {candidate / 'packages/core/src/cliCommandMatrix.ts'}"
        for candidate in roots
    )
    raise FileNotFoundError(
        "Could not find TS CLI matrix. Set FORBOCAI_TS_SDK_ROOT or pass "
        f"--ts-sdk-root.\nSearched:\n{searched}"
    )


def extract_ts_node_keys(path: Path) -> list[str]:
    text = path.read_text(encoding="utf-8")
    match = re.search(
        r"NODE_CLI_COMMAND_KEYS\s*=\s*\[(?P<body>.*?)\]\s+as\s+const",
        text,
        re.S,
    )
    if not match:
        raise ValueError(f"Could not find NODE_CLI_COMMAND_KEYS in {path}")
    return re.findall(r"'([^']+)'", match.group("body"))


def extract_ue_node_keys(path: Path) -> list[str]:
    text = path.read_text(encoding="utf-8")
    match = re.search(
        r"BEGIN_NODE_CLI_COMMAND_KEYS(?P<body>.*?)END_NODE_CLI_COMMAND_KEYS",
        text,
        re.S,
    )
    if not match:
        raise ValueError(f"Could not find UE node CLI command marker block in {path}")
    return re.findall(r'\{\s*TEXT\("([^"]+)"\)', match.group("body"))


def format_list(values: list[str]) -> str:
    return "\n".join(f"  - {value}" for value in values) if values else "  (none)"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compare UE CLI command keys with the TS SDK Node CLI matrix."
    )
    parser.add_argument("--ts-sdk-root", help="Path to the TS SDK repo root.")
    args = parser.parse_args()

    root = repo_root()
    ts_matrix = resolve_ts_matrix(root, args.ts_sdk_root)
    ue_matrix = root / "Source" / "ForbocAI_SDK" / "Public" / "CLI" / "CliCommandMatrix.h"

    ts_keys = extract_ts_node_keys(ts_matrix)
    ue_keys = extract_ue_node_keys(ue_matrix)

    missing = [key for key in ts_keys if key not in ue_keys]
    extra = [key for key in ue_keys if key not in ts_keys]
    order_matches = ts_keys == ue_keys

    print("[check] UE CLI command parity")
    print(f"[info] TS matrix: {ts_matrix}")
    print(f"[info] UE matrix: {ue_matrix}")

    if not missing and not extra and order_matches:
        print(f"[ok] UE mirrors TS NODE_CLI_COMMAND_KEYS ({len(ts_keys)} commands)")
        return 0

    if missing:
        print("[fail] Commands present in TS but missing in UE:")
        print(format_list(missing))
    if extra:
        print("[fail] Commands present in UE node-parity block but absent from TS:")
        print(format_list(extra))
    if not order_matches and not missing and not extra:
        print("[fail] Command sets match, but order differs from TS matrix.")

    print("")
    print("[hint] Update Source/ForbocAI_SDK/Public/CLI/CliCommandMatrix.h")
    print("       so the BEGIN_NODE_CLI_COMMAND_KEYS block mirrors TS exactly.")
    return 1


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"[fail] {exc}", file=sys.stderr)
        raise SystemExit(1)
