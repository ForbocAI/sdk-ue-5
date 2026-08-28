from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class ExportGroup:
    identifier: str
    label: str
    names: tuple[str, ...]
    evidence: tuple[str, ...]
