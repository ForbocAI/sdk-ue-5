#!/usr/bin/env python3
"""Assert the TS and UE micro-game share one harness vocabulary contract.

check-sdk-parity.py compares folder/file/symbol *names* and the Node CLI command
key matrix. That is inventory parity: it proves both hosts declare the same
symbols, not that they declare the same *vocabulary values* or behave the same.
A mode, command group, status, or lifecycle event can be added to one host's
authored `runtime.json` and silently omitted from the other -- both still export
a `RunGame`/`runGame` symbol, so name parity stays green while the hosts diverge.

This check closes that gap for the authored harness vocabulary. The runtime
vocabulary sets are the "one vocabulary contract" both hosts read (the UE
`ReadEnabledName` decoder and the TS `namesByKey` adapter derive their tokens
from these exact JSON keys), so their key sets MUST match. Host-only fields
(e.g. UE `wireFields`, `template`) are not part of the shared vocabulary and are
intentionally excluded.

Run from the UE SDK plugin root (same as verify-ue-parity.sh):
    python3 scripts/check-harness-vocabulary-parity.py
"""

from __future__ import annotations

import json
import os
import sys
from pathlib import Path

# runtime.json fields that both hosts decode into a shared name vocabulary.
# Each host reads tokens straight from these JSON keys, so the key sets are the
# contract and must be identical across TS and UE.
SHARED_VOCABULARY_FIELDS = (
    "modes",
    "commandGroups",
    "lifecycleEvents",
    "outputAssertionKinds",
    "runtimeTypes",
    "scenarioEventTypes",
    "statuses",
)

UE_RUNTIME_RELPATH = Path("micro-game-cli/Content/Data/harness/runtime.json")
TS_RUNTIME_RELPATH = Path("packages/micro-game-core/data/harness/runtime.json")


def ue_repo_root() -> Path:
    """Resolve the UE SDK plugin root from this script's location."""
    return Path(__file__).resolve().parents[1]


def resolve_ts_root(ue_root: Path) -> Path:
    """Locate the sibling TS SDK repo (env override or a sibling with the data)."""
    env_root = os.environ.get("FORBOCAI_TS_SDK_ROOT")
    candidates = [Path(env_root)] if env_root else []
    candidates += sorted(
        (
            sibling
            for sibling in ue_root.parent.iterdir()
            if sibling.is_dir() and sibling != ue_root
        ),
        key=lambda candidate: candidate.name.casefold(),
    )
    for candidate in candidates:
        if (candidate / TS_RUNTIME_RELPATH).is_file():
            return candidate
    searched = "\n".join(f"  - {candidate}" for candidate in candidates)
    raise FileNotFoundError(
        "Could not find TS SDK root with harness runtime data. Set "
        f"FORBOCAI_TS_SDK_ROOT.\nSearched:\n{searched}"
    )


def load_json(path: Path) -> dict:
    """Read one authored JSON document."""
    return json.loads(path.read_text(encoding="utf-8"))


def main() -> int:
    """Compare the shared harness vocabulary key sets across TS and UE."""
    ue_root = ue_repo_root()
    ts_root = resolve_ts_root(ue_root)
    ue_runtime = load_json(ue_root / UE_RUNTIME_RELPATH)
    ts_runtime = load_json(ts_root / TS_RUNTIME_RELPATH)

    print(f"[info] TS harness runtime: {ts_root / TS_RUNTIME_RELPATH}")
    print(f"[info] UE harness runtime: {ue_root / UE_RUNTIME_RELPATH}")

    divergences: list[str] = []
    for field in SHARED_VOCABULARY_FIELDS:
        ts_keys = set(ts_runtime.get(field, {}).keys())
        ue_keys = set(ue_runtime.get(field, {}).keys())
        if ts_keys == ue_keys:
            print(f"[ok] {field}: {len(ts_keys)} shared tokens")
            continue
        ts_only = sorted(ts_keys - ue_keys)
        ue_only = sorted(ue_keys - ts_keys)
        divergences.append(field)
        print(f"[FAIL] {field}: vocabulary diverges")
        if ts_only:
            print(f"       present in TS, missing in UE: {ts_only}")
        if ue_only:
            print(f"       present in UE, missing in TS: {ue_only}")

    if divergences:
        print(
            "\nHarness vocabulary parity FAILED. The TS and UE micro-games must "
            "declare identical "
            + ", ".join(SHARED_VOCABULARY_FIELDS)
            + " tokens in their authored runtime.json. Mirror the missing tokens "
            "so both hosts share one vocabulary contract."
        )
        return 1

    print("\nHarness vocabulary parity OK: TS and UE share one vocabulary contract.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
