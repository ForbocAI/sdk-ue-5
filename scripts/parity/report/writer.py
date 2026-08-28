from __future__ import annotations

import json
import re
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any

from parity.report.contracts import ReportDefinition
from parity.report.model import Capability, Sources, combine_summaries, summarize
from parity.report.validation import validate_document


def _json_text(document: dict[str, Any]) -> str:
    return json.dumps(document, indent=2, sort_keys=True, ensure_ascii=True) + "\n"


def _slug(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", value.lower()).strip("-")


def _chunks(values: Sequence[Capability], size: int) -> list[list[Capability]]:
    return [list(values[index : index + size]) for index in range(0, len(values), size)]


def _schema_reference(from_path: Path, schema_path: Path) -> str:
    return Path(__import__("os").path.relpath(schema_path, from_path.parent)).as_posix()


def _write_validated(
    path: Path,
    document: dict[str, Any],
    schema_path: Path,
) -> None:
    errors = validate_document(schema_path, document)
    if errors:
        raise ValueError("Parity report schema validation failed:\n" + "\n".join(errors))
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(_json_text(document), encoding="utf-8", newline="\n")


def _clear_generated_json(directory: Path) -> None:
    if not directory.is_dir():
        return
    for path in sorted(directory.rglob("*.json"), reverse=True):
        path.unlink()
    for path in sorted(directory.rglob("*"), reverse=True):
        if path.is_dir() and not any(path.iterdir()):
            path.rmdir()


def write_report_set(
    definition: ReportDefinition,
    sources: Sources,
    groups: Mapping[str, Sequence[Capability]],
) -> tuple[Path, dict[str, Any]]:
    ordered_groups = {
        key: sorted(values, key=lambda item: str(item["id"]))
        for key, values in sorted(groups.items())
    }
    _clear_generated_json(definition.directory)
    group_parts: list[dict[str, Any]] = []
    for group, capabilities in ordered_groups.items():
        parts: list[dict[str, Any]] = []
        for index, chunk in enumerate(
            _chunks(capabilities, definition.max_capabilities_per_part), start=1
        ):
            part_id = f"{definition.report_id}.{_slug(group)}.{index:03d}"
            part_path = definition.directory / _slug(group) / f"{index:03d}.json"
            part_summary = summarize(chunk)
            document = {
                "$schema": _schema_reference(part_path, definition.part_schema),
                "schemaVersion": definition.schema_version,
                "reportId": part_id,
                "generatedBy": definition.generated_by,
                "sources": sources,
                "summary": part_summary,
                "capabilities": chunk,
            }
            _write_validated(part_path, document, definition.part_schema)
            parts.append(
                {
                    "id": part_id,
                    "path": part_path.relative_to(definition.directory).as_posix(),
                    "summary": part_summary,
                }
            )
        group_path = definition.directory / _slug(group) / "index.json"
        group_summary = combine_summaries([part["summary"] for part in parts])
        group_index = {
            "$schema": _schema_reference(group_path, definition.index_schema),
            "schemaVersion": definition.schema_version,
            "reportId": f"{definition.report_id}.{_slug(group)}",
            "generatedBy": definition.generated_by,
            "sources": sources,
            "summary": group_summary,
            "parts": parts,
        }
        _write_validated(group_path, group_index, definition.index_schema)
        group_parts.append(
            {
                "id": group_index["reportId"],
                "path": group_path.relative_to(definition.directory).as_posix(),
                "summary": group_summary,
            }
        )
    index_path = definition.directory / "index.json"
    index = {
        "$schema": _schema_reference(index_path, definition.index_schema),
        "schemaVersion": definition.schema_version,
        "reportId": definition.report_id,
        "generatedBy": definition.generated_by,
        "sources": sources,
        "summary": combine_summaries([part["summary"] for part in group_parts]),
        "parts": group_parts,
    }
    _write_validated(index_path, index, definition.index_schema)
    return index_path, index
