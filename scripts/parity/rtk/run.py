from __future__ import annotations

from pathlib import Path

from generate_authored_values import generated_output_paths
from parity.report.contracts import load_report_definition
from parity.report.sources import source_record
from parity.report.writer import write_report_set
from parity.rtk.capabilities import build_capability_groups
from parity.rtk.typescript import (
    build_export_groups,
    resolve_ts_root,
    toolkit_version,
    tracked_contract_paths,
)
from parity.rtk.unreal import local_include_closure, name_index


def repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def run(explicit_ts_root: str | None = None) -> int:
    ue_root = repo_root()
    ts_root = resolve_ts_root(ue_root, explicit_ts_root)
    groups, ts_contract_paths = build_export_groups(ts_root)
    rtk_hpp = ue_root / "Source/ForbocAI_SDK/Public/Core/rtk.hpp"
    public_root = ue_root / "Source/ForbocAI_SDK/Public"
    generated = {path.resolve() for path in generated_output_paths()}
    ue_sources = tuple(
        path
        for path in local_include_closure(rtk_hpp, public_root)
        if path.resolve() not in generated
    )
    capabilities = build_capability_groups(
        groups,
        name_index(ue_sources, ue_root),
    )
    sources = {
        "typescript": source_record(
            ts_root,
            ts_contract_paths,
            tracked_contract_paths(ts_root),
            "redux-toolkit-runtime-and-source",
        ),
        "unreal": source_record(
            ue_root,
            ue_sources,
            (rtk_hpp, public_root),
            bundle_label="rtk-hpp-public-closure",
        ),
    }
    definition = load_report_definition(ue_root, "rtk")
    index_path, report = write_report_set(definition, sources, capabilities)
    summary = report["summary"]
    print(f"[info] @reduxjs/toolkit version: {toolkit_version(ts_root)}")
    print(f"[info] UE rtk.hpp source headers discovered: {len(ue_sources)}")
    print(f"[info] RTK capabilities compared: {summary['capabilityCount']}")
    print(f"[info] Deterministic report: {index_path.relative_to(ue_root)}")
    if summary["mismatchCount"]:
        print(f"[fail] RTK public-surface mismatches: {summary['mismatchCount']}")
        return 1
    print("[ok] UE rtk.hpp mirrors the exported Redux Toolkit and RTK Query surface.")
    return 0
