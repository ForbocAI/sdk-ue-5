"""Pure API-contract parity analysis for the UE micro-game checker."""

from __future__ import annotations

import re
from typing import Any


def sorted_difference(expected: set[str], actual: set[str], label: str) -> list[str]:
    """Return deterministic missing and unexpected set findings."""
    return [
        *[f"{label}: missing {item}" for item in sorted(expected - actual)],
        *[f"{label}: unexpected {item}" for item in sorted(actual - expected)],
    ]


def nested_object_keys(value: Any) -> set[str]:
    """Discover every wire-field name represented by a JSON-like value."""
    if isinstance(value, dict):
        return set(value) | set().union(*(nested_object_keys(item) for item in value.values()))
    if isinstance(value, list):
        return set().union(*(nested_object_keys(item) for item in value))
    return set()


def parse_contract_schema_fields(source: str) -> set[str]:
    """Discover schema member names from the C++ source-of-truth macro."""
    match = re.search(
        r"#define\s+FORBOCAI_CONTRACT_SCHEMA_FIELDS\(X\)(?P<body>.*?)(?=\n\n)",
        source,
        re.S,
    )
    return set(re.findall(r"\bX\(([A-Za-z][A-Za-z0-9_]*)\)", match.group("body"))) if match else set()


def parse_schema_references(source: str) -> set[str]:
    """Discover schema members consumed by the parser implementation."""
    return set(
        re.findall(
            r"(?:ContractData\(\)\s*\.\s*)?Schema\s*\.\s*([A-Za-z][A-Za-z0-9_]*)",
            source,
        )
    )


def validate_schema_contract(
    contract: dict[str, Any],
    schema: dict[str, Any],
    declared_fields: set[str],
    referenced_fields: set[str],
) -> list[str]:
    """Verify API wire keys, authored schema, declarations, and parser reads agree."""
    failures: list[str] = []
    schema_fields = set(schema)
    schema_wire_names = {value for value in schema.values() if isinstance(value, str) and value}
    if len(schema_wire_names) != len(schema):
        failures.append("UE contract schema values must be unique non-empty strings")
    failures.extend(sorted_difference(nested_object_keys(contract), schema_wire_names, "UE contract wire schema"))
    failures.extend(sorted_difference(schema_fields, declared_fields, "UE contract schema declarations"))
    failures.extend(sorted_difference(schema_fields, referenced_fields, "UE contract parser schema reads"))
    return failures


def validate_runtime_command_groups(
    contract: dict[str, Any],
    runtime_data: dict[str, Any],
    game_data: dict[str, Any],
) -> list[str]:
    """Verify runtime vocabulary is exactly API-required groups plus authored bootstrap groups."""
    required_groups = {
        value for value in contract.get("requiredCommandGroups", []) if isinstance(value, str)
    }
    command_groups = runtime_data.get("commandGroups", {})
    runtime_groups = set(command_groups) if isinstance(command_groups, dict) else set()
    contract_command = game_data.get("contractCommand", {})
    bootstrap_group = contract_command.get("group") if isinstance(contract_command, dict) else None
    bootstrap_groups = {bootstrap_group} if isinstance(bootstrap_group, str) and bootstrap_group else set()
    return sorted_difference(required_groups | bootstrap_groups, runtime_groups, "UE runtime command groups")


def validate_parser_shapes(source: str) -> list[str]:
    """Verify direct typed decoding and dynamic vocabulary validation remain present."""
    requirements = {
        "UE contract parser must decode commands directly into FCommandSpec": (
            r"TParseResult\s*<\s*FCommandSpec\s*>\s+ParseCommand\s*\("
        ),
        "UE contract parser must decode scenarios directly into FScenarioStep": (
            r"TParseResult\s*<\s*FScenarioStep\s*>\s+ParseScenario\s*\("
        ),
        "UE contract parser must preserve expected routes": r"Command\s*\.\s*ExpectedRoutes\s*=",
        "UE contract parser must validate authored command-group vocabulary": (
            r"GameRuntimeData\(\)\s*\.\s*commandGroups\s*\.\s*all\s*\.\s*Contains\s*\("
        ),
    }
    return [message for message, pattern in requirements.items() if not re.search(pattern, source, re.S)]


def validate_schema_loader(source: str) -> list[str]:
    """Verify the typed schema loader consumes every declared field through one macro."""
    requirements = {
        "UE contract schema loader must read field names from authored data": (
            r"DataAdapters::ReadStringField\s*\(\s*Schema\s*,\s*TEXT\s*\(\s*#Name\s*\)\s*\)"
        ),
        "UE contract schema loader must expand every declared schema field": (
            r"FORBOCAI_CONTRACT_SCHEMA_FIELDS\s*\(\s*FORBOCAI_READ_CONTRACT_SCHEMA_FIELD\s*\)"
        ),
    }
    return [message for message, pattern in requirements.items() if not re.search(pattern, source, re.S)]
