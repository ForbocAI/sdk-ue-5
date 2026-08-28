from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

from parity.sdk.types import CliContract


def _object(path: Path) -> dict[str, Any]:
    value = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(value, dict):
        raise ValueError(f"Expected a JSON object in {path}")
    return value


def _single(paths: list[Path], label: str) -> Path:
    if len(paths) != 1:
        evidence = ", ".join(str(path) for path in paths) or "none"
        raise ValueError(f"Expected exactly one {label}; found {len(paths)}: {evidence}")
    return paths[0]


def _validate_domain(path: Path, value: dict[str, Any]) -> dict[str, dict[str, Any]]:
    invalid = [
        key
        for key, command in value.items()
        if not isinstance(key, str)
        or not isinstance(command, dict)
        or not isinstance(command.get("surfaces"), list)
    ]
    if invalid:
        raise ValueError(f"Invalid command entries in {path}: {', '.join(invalid)}")
    return {key: command for key, command in value.items()}


def _ts_domain_order(root: Path) -> tuple[tuple[str, ...], Path]:
    source = _single(
        list(root.glob("packages/*/src/systems/cli/catalog/catalogAdapters.ts")),
        "TS CLI catalog adapter",
    )
    domains = tuple(
        re.findall(
            r"from\s+['\"].*?/data/cli/commands/([a-z0-9-]+)\.json['\"]",
            source.read_text(encoding="utf-8"),
        )
    )
    if not domains:
        raise ValueError(f"No command-domain imports found in {source}")
    return tuple(dict.fromkeys(domains)), source


def _ue_domain_order(directory: Path) -> tuple[tuple[str, ...], Path]:
    catalog_path = directory / "catalog.json"
    catalog = _object(catalog_path)
    paths = catalog.get("commands")
    if not isinstance(paths, list) or not all(isinstance(path, str) for path in paths):
        raise ValueError(f"Invalid command paths in {catalog_path}")
    return tuple(Path(path).stem for path in paths), catalog_path


def _load_contract(
    root: Path,
    directory: Path,
    domain_order: tuple[str, ...],
    order_source: Path,
) -> CliContract:
    vocabulary_path = directory / "vocabulary.json"
    roles_path = directory / "roles.json"
    command_paths = tuple(sorted((directory / "commands").glob("*.json")))
    domains = {
        path.stem: _validate_domain(path, _object(path)) for path in command_paths
    }
    missing = sorted(set(domain_order) - domains.keys())
    extra = sorted(domains.keys() - set(domain_order))
    if missing or extra:
        raise ValueError(
            f"CLI domain-order drift in {directory}: missing={missing} extra={extra}"
        )
    return CliContract(
        root,
        directory,
        _object(vocabulary_path),
        _object(roles_path),
        domains,
        domain_order,
        (vocabulary_path, roles_path, *command_paths, order_source),
    )


def load_ts_cli_contract(root: Path) -> CliContract:
    vocabulary = _single(
        list(root.glob("packages/*/data/cli/vocabulary.json")),
        "TS split CLI contract",
    )
    order, source = _ts_domain_order(root)
    return _load_contract(root, vocabulary.parent, order, source)


def load_ue_cli_contract(root: Path) -> CliContract:
    vocabulary = _single(
        list(root.glob("Content/Data/cli/vocabulary.json")),
        "UE split CLI contract",
    )
    order, source = _ue_domain_order(vocabulary.parent)
    return _load_contract(root, vocabulary.parent, order, source)


def node_command_keys(contract: CliContract) -> tuple[str, ...]:
    return tuple(
        key
        for domain in contract.domain_order
        for key, command in contract.domains[domain].items()
        if "node" in command["surfaces"]
    )
