#!/usr/bin/env python3
"""Shared UE check target discovery.

SDK-owned checks run against the UE SDK, the SDK CLI/test-game target, and the
sibling UE demo when it is present. The caller should not have to pass a runtime
root to get full coverage.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path


SDK_ROOT = Path(__file__).resolve().parents[1]
WORKSPACE_ROOT = SDK_ROOT.parent


@dataclass(frozen=True)
class UeTarget:
    label: str
    root: Path


def _has_source(root: Path) -> bool:
    return (root / "Source").is_dir()


def ue_targets() -> tuple[UeTarget, ...]:
    candidates = (
        UeTarget("sdk", SDK_ROOT),
        UeTarget("sdk-cli", SDK_ROOT / "test-game-cli"),
        UeTarget("demo", WORKSPACE_ROOT / "demo-ue-5"),
    )
    return tuple(target for target in candidates if _has_source(target.root))


def ue_source_roots() -> tuple[Path, ...]:
    return tuple(target.root / "Source" for target in ue_targets())


def sdk_module_root() -> Path:
    return SDK_ROOT / "Source" / "ForbocAI_SDK"


def sdk_cli_module_root() -> Path:
    return SDK_ROOT / "test-game-cli" / "Source" / "ForbocAI_TestGame_CLI"
