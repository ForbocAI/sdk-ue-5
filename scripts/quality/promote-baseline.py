#!/usr/bin/env python3
"""Promote a passing Unreal CLI quality report to the accepted baseline."""

from __future__ import annotations

import json
from pathlib import Path
import shutil


SCRIPT_PATH = Path(__file__).resolve()
SDK_ROOT = SCRIPT_PATH.parents[2]
REPORT_CONFIG_PATH = (
    SDK_ROOT / "test-game-cli" / "Content" / "Data" / "quality" / "report.json"
)


def report_path(segments: list[str]) -> Path:
    """Resolve one authored report path from the UE test-game project root."""
    return SDK_ROOT / "test-game-cli" / Path(*segments)


def main() -> int:
    """Validate the current absolute gate and promote its exact report bytes."""
    config = json.loads(REPORT_CONFIG_PATH.read_text(encoding="utf-8"))
    current_path = report_path(config["paths"]["current"])
    baseline_path = report_path(config["paths"]["baseline"])
    messages = config["messages"]
    tokens = config["tokens"]
    if not current_path.is_file():
        raise SystemExit(messages["currentMissing"])
    current = json.loads(current_path.read_text(encoding="utf-8"))
    if not current["summary"]["absoluteGatePassed"]:
        raise SystemExit(messages["absoluteGateFailed"])
    baseline_path.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(current_path, baseline_path)
    display_path = baseline_path.relative_to(SDK_ROOT).as_posix()
    print(messages["promoted"].replace(tokens["path"], display_path))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
