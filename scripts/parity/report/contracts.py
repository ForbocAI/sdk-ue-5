from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class ReportDefinition:
    schema_version: int
    max_capabilities_per_part: int
    report_id: str
    generated_by: str
    directory: Path
    index_schema: Path
    part_schema: Path


def load_report_definition(root: Path, key: str) -> ReportDefinition:
    contract_path = root / "reports/parity/contract.json"
    contract = json.loads(contract_path.read_text(encoding="utf-8"))
    report = contract["reports"][key]
    schemas = contract["schemas"]
    return ReportDefinition(
        schema_version=int(contract["schemaVersion"]),
        max_capabilities_per_part=int(contract["maxCapabilitiesPerPart"]),
        report_id=str(report["id"]),
        generated_by=str(report["generatedBy"]),
        directory=root / str(report["directory"]),
        index_schema=root / str(schemas["index"]),
        part_schema=root / str(schemas["part"]),
    )
