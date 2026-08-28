from __future__ import annotations

from pathlib import Path

from parity.report.contracts import load_report_definition
from parity.report.sources import source_record
from parity.report.writer import write_report_set
from parity.sdk.capabilities import build_cli_capabilities, build_structure_capabilities
from parity.sdk.commands import load_ts_cli_contract, load_ue_cli_contract
from parity.sdk.discovery import (
    build_parity_programs,
    discover_ts_source_roots,
    discover_ue_source_roots,
    iter_ts_files,
    iter_ue_files,
    resolve_ts_root,
    unmapped_ts_roots,
    unmapped_ue_roots,
)
from parity.sdk.mirrors import build_program_inventory


def repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def run(explicit_ts_root: str | None = None) -> int:
    ue_root = repo_root()
    ts_root = resolve_ts_root(ue_root, explicit_ts_root)
    ts_source_roots = discover_ts_source_roots(ts_root)
    ue_source_roots = discover_ue_source_roots(ue_root)
    programs = build_parity_programs(ts_source_roots, ue_source_roots)
    if not programs:
        raise ValueError("No TS and UE parity programs were discovered.")
    inventories = [
        build_program_inventory(ts_root, ue_root, program) for program in programs
    ]
    ts_cli = load_ts_cli_contract(ts_root)
    ue_cli = load_ue_cli_contract(ue_root)
    groups = {
        **build_cli_capabilities(ts_cli, ue_cli),
        **build_structure_capabilities(
            inventories,
            unmapped_ts_roots(ts_source_roots, programs),
            unmapped_ue_roots(ue_source_roots, programs),
        ),
    }
    ts_sources = tuple(iter_ts_files(ts_root, ts_source_roots))
    ue_sources = tuple(iter_ue_files(ue_root, ue_source_roots))
    ts_commit_paths = tuple(ts_root / path for path in ts_source_roots)
    ue_commit_paths = tuple(ue_root / path for path in ue_source_roots)
    sources = {
        "typescript": source_record(
            ts_root,
            (*ts_sources, *ts_cli.files),
            (*ts_commit_paths, *ts_cli.files),
            bundle_label="sdk-source-and-cli-contract",
        ),
        "unreal": source_record(
            ue_root,
            (*ue_sources, *ue_cli.files),
            (*ue_commit_paths, *ue_cli.files),
            bundle_label="sdk-source-and-cli-contract",
        ),
    }
    definition = load_report_definition(ue_root, "sdk")
    index_path, report = write_report_set(definition, sources, groups)
    summary = report["summary"]
    print(f"[info] Parity programs discovered: {len(programs)}")
    print(f"[info] SDK capabilities compared: {summary['capabilityCount']}")
    print(f"[info] Expected runtime mechanics: {summary['expectedMechanicCount']}")
    print(f"[info] Deterministic report: {index_path.relative_to(ue_root)}")
    if summary["mismatchCount"]:
        print(f"[fail] Behavioral SDK parity mismatches: {summary['mismatchCount']}")
        return 1
    print("[ok] UE and TS behavioral SDK contracts match.")
    return 0
