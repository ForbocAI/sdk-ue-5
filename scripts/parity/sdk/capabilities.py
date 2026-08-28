from __future__ import annotations

import json
import re
from collections.abc import Sequence
from pathlib import Path
from typing import Any

from parity.report.model import Capability, capability, endpoint_evidence, mismatch
from parity.sdk.types import CliContract, ProgramInventory


def _relative(path: Path, root: Path) -> str:
    return path.relative_to(root).as_posix()


def _compact(value: Any) -> str:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True)


def _changed_keys(left: dict[str, Any], right: dict[str, Any]) -> list[str]:
    return sorted(
        key for key in left.keys() | right.keys() if left.get(key) != right.get(key)
    )


def _contract_capability(
    capability_id: str,
    category: str,
    ts_value: dict[str, Any],
    ue_value: dict[str, Any],
    ts_path: Path,
    ue_path: Path,
    ts_root: Path,
    ue_root: Path,
) -> Capability:
    same = ts_value == ue_value
    changed = _changed_keys(ts_value, ue_value)
    return capability(
        capability_id,
        category,
        "behavioral",
        "same" if same else "mismatch",
        endpoint_evidence(True, (_relative(ts_path, ts_root),)),
        endpoint_evidence(True, (_relative(ue_path, ue_root),)),
        None
        if same
        else mismatch(
            f"Contract fields differ: {', '.join(changed)}.",
            "Make the UE authored subdomain equal to the TS contract, then regenerate the report.",
        ),
    )


def _command_capability(
    domain: str,
    key: str,
    ts: CliContract,
    ue: CliContract,
) -> Capability:
    ts_value = ts.domains.get(domain, {}).get(key)
    ue_value = ue.domains.get(domain, {}).get(key)
    same = ts_value is not None and ts_value == ue_value
    ts_path = ts.directory / "commands" / f"{domain}.json"
    ue_path = ue.directory / "commands" / f"{domain}.json"
    return capability(
        f"cli.command.{key}",
        "cli-command",
        "behavioral",
        "same" if same else "mismatch",
        endpoint_evidence(
            ts_value is not None,
            () if ts_value is None else (_relative(ts_path, ts.root), _compact(ts_value)),
        ),
        endpoint_evidence(
            ue_value is not None,
            () if ue_value is None else (_relative(ue_path, ue.root), _compact(ue_value)),
        ),
        None
        if same
        else mismatch(
            f"Command {key} in domain {domain} is missing or behaviorally different.",
            "Mirror the TS command record in the UE authored command subdomain and route it through the SDK handler.",
        ),
    )


def build_cli_capabilities(
    ts: CliContract, ue: CliContract
) -> dict[str, list[Capability]]:
    vocabulary = _contract_capability(
        "cli.contract.vocabulary",
        "cli-contract",
        ts.vocabulary,
        ue.vocabulary,
        ts.directory / "vocabulary.json",
        ue.directory / "vocabulary.json",
        ts.root,
        ue.root,
    )
    roles = _contract_capability(
        "cli.contract.roles",
        "cli-contract",
        ts.roles,
        ue.roles,
        ts.directory / "roles.json",
        ue.directory / "roles.json",
        ts.root,
        ue.root,
    )
    same_order = ts.domain_order == ue.domain_order
    order = capability(
        "cli.contract.domain-order",
        "cli-contract",
        "behavioral",
        "same" if same_order else "mismatch",
        endpoint_evidence(True, ts.domain_order),
        endpoint_evidence(True, ue.domain_order),
        None
        if same_order
        else mismatch(
            "CLI command-domain composition order differs.",
            "Use the same semantic domain order in both package catalog compositions.",
        ),
    )
    domains = sorted(ts.domains.keys() | ue.domains.keys())
    groups = {
        f"cli-{domain}": [
            _command_capability(domain, key, ts, ue)
            for key in sorted(
                ts.domains.get(domain, {}).keys() | ue.domains.get(domain, {}).keys()
            )
        ]
        for domain in domains
    }
    return {"cli-contract": [vocabulary, roles, order], **groups}


def _program_id(label: str) -> str:
    return re.sub(r"[^a-z0-9]+", "-", label.split(":", 1)[0].lower()).strip("-")


def _mirror_capability(
    program_id: str,
    kind: str,
    ts_count: int,
    ue_count: int,
    mirrors: Sequence[Any],
) -> Capability:
    different = [item for item in mirrors if item.status != "Same"]
    result = "expected-mechanic" if different else "same"
    examples = [
        getattr(item, "ts_folder", None)
        or getattr(item, "ts_file", None)
        or item.ts_symbol.file
        for item in different[:5]
    ]
    return capability(
        f"structure.{program_id}.{kind}",
        "source-structure",
        "runtime-mechanic",
        result,
        endpoint_evidence(True, (f"count={ts_count}", *examples)),
        endpoint_evidence(True, (f"count={ue_count}", f"exact={ts_count - len(different)}")),
        None,
    )


def build_structure_capabilities(
    inventories: Sequence[ProgramInventory],
    unmapped_ts: Sequence[Path],
    unmapped_ue: Sequence[Path],
) -> dict[str, list[Capability]]:
    capabilities = [
        item
        for inventory in inventories
        for item in (
            _mirror_capability(
                _program_id(inventory.program.label),
                "folders",
                len(inventory.ts_folders),
                len(inventory.ue_folders),
                inventory.folder_mirrors,
            ),
            _mirror_capability(
                _program_id(inventory.program.label),
                "files",
                len(inventory.ts_files),
                len(inventory.ue_files),
                inventory.file_mirrors,
            ),
            _mirror_capability(
                _program_id(inventory.program.label),
                "symbols",
                len(inventory.ts_symbols),
                len(inventory.ue_symbols),
                inventory.symbol_mirrors,
            ),
        )
    ]
    roots_same = not unmapped_ts and not unmapped_ue
    capabilities.append(
        capability(
            "structure.source-roots",
            "source-structure",
            "runtime-mechanic",
            "same" if roots_same else "expected-mechanic",
            endpoint_evidence(True, (path.as_posix() for path in unmapped_ts)),
            endpoint_evidence(True, (path.as_posix() for path in unmapped_ue)),
            None,
        )
    )
    return {"source-structure": capabilities}
