#!/usr/bin/env python3
"""Prepare the allowlisted source tree consumed by Unreal BuildPlugin."""

from __future__ import annotations

import json
from pathlib import Path
import shutil


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parents[1]
CONFIG_PATH = SCRIPT_DIR / "data" / "package.json"


def load_config() -> dict[str, object]:
    """User Story: As release automation, I need one package manifest loaded once. Signature: load_config() -> dict[str, object]."""
    return json.loads(CONFIG_PATH.read_text(encoding="utf-8"))


def is_excluded(relative_path: Path, patterns: list[str]) -> bool:
    """User Story: As package preparation, I need internal paths rejected by authored rules. Signature: is_excluded(relative_path: Path, patterns: list[str]) -> bool."""
    normalized = relative_path.as_posix()
    return any(relative_path.match(pattern) or normalized.startswith(pattern.removesuffix("/**") + "/") for pattern in patterns)


def copy_entry(source: Path, destination: Path, patterns: list[str]) -> int:
    """User Story: As package preparation, I need allowlisted files copied without internal artifacts. Signature: copy_entry(source: Path, destination: Path, patterns: list[str]) -> int."""
    if source.is_symlink():
        raise RuntimeError(f"Package inputs must not be symbolic links: {source}")
    if source.is_file():
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, destination)
        return 1
    copied = 0
    for child in sorted(source.rglob("*")):
        relative = child.relative_to(PROJECT_ROOT)
        if is_excluded(relative, patterns):
            continue
        if child.is_symlink():
            raise RuntimeError(f"Package inputs must not be symbolic links: {child}")
        if child.is_file():
            target = destination / child.relative_to(source)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(child, target)
            copied += 1
    return copied


def main() -> int:
    """User Story: As a publisher, I need a clean source package before Unreal compiles it. Signature: main() -> int."""
    config = load_config()
    paths = config["paths"]
    stage = PROJECT_ROOT / str(paths["stageDirectory"])
    if stage.exists():
        shutil.rmtree(stage)
    stage.mkdir(parents=True)

    patterns = [str(value) for value in config["exclude"]]
    copied = 0
    for value in config["include"]:
        relative = Path(str(value))
        source = PROJECT_ROOT / relative
        if not source.exists():
            raise FileNotFoundError(f"Required package input does not exist: {source}")
        copied += copy_entry(source, stage / relative, patterns)

    print(f"Prepared {copied} Fab package file(s) at {stage}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
