#!/usr/bin/env python3
"""Shared UE check target discovery.

SDK-owned checks run against the UE SDK, the SDK CLI/micro-game target, and every
UE project that consumes this SDK. Consumers are discovered structurally, so the
checks cover the same ground from either checkout shape:

* the SDK repo checked out beside its consumers -- a consumer is a sibling of
  the SDK root;
* the SDK vendored into a consumer as ``<project>/Plugins/ForbocAI_SDK`` -- the
  consumer is the project that owns the ``Plugins`` folder.

A consumer is any directory that owns a ``.uproject`` and a ``Source`` folder.
Callers branch on ``UeTarget.kind`` (``sdk``/``sdk-cli``/``project``), never on
``label``: ``label`` is display text and carries the consumer's folder name.
The caller should not have to pass a runtime root to get full coverage.
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
    kind: str


def _has_source(root: Path) -> bool:
    return (root / "Source").is_dir()


def _is_ue_project(root: Path) -> bool:
    return _has_source(root) and any(root.glob("*.uproject"))


def _vendor_hosts() -> tuple[Path, ...]:
    """Projects that vendor this SDK under their own ``Plugins`` folder."""
    return tuple(
        plugins.parent
        for plugins in SDK_ROOT.parents
        if plugins.name == "Plugins" and _is_ue_project(plugins.parent)
    )


def _workspace_projects() -> tuple[Path, ...]:
    """UE projects checked out beside the SDK repo."""
    return tuple(
        child
        for child in sorted(WORKSPACE_ROOT.iterdir())
        if child.is_dir() and child != SDK_ROOT and _is_ue_project(child)
    )


def _consumer_projects() -> tuple[Path, ...]:
    return tuple(dict.fromkeys((*_vendor_hosts(), *_workspace_projects())))


def ue_targets() -> tuple[UeTarget, ...]:
    candidates = (
        UeTarget("sdk", SDK_ROOT, "sdk"),
        UeTarget("sdk-cli", SDK_ROOT / "micro-game-cli", "sdk-cli"),
        *(UeTarget(root.name, root, "project") for root in _consumer_projects()),
    )
    return tuple(target for target in candidates if _has_source(target.root))


def ue_source_roots() -> tuple[Path, ...]:
    return tuple(target.root / "Source" for target in ue_targets())


def sdk_module_root() -> Path:
    return SDK_ROOT / "Source" / "ForbocAI_SDK"


def sdk_cli_module_root() -> Path:
    return SDK_ROOT / "micro-game-cli" / "Source" / "ForbocAI_MicroGame_CLI"
