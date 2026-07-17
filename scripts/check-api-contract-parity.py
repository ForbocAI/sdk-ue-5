#!/usr/bin/env python3
"""Static UE parity guard for the API-published test-game contract."""

from __future__ import annotations

import json
import os
import re
import sys
import urllib.request
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR / "parity"))

from api_contract import (
    parse_contract_schema_fields,
    parse_schema_references,
    sorted_difference,
    validate_parser_shapes,
    validate_runtime_command_groups,
    validate_schema_contract,
    validate_schema_loader,
)

PLUGIN_ROOT = SCRIPT_DIR.parent
REPO_ROOT = PLUGIN_ROOT
WORKSPACE_ROOT = REPO_ROOT.parent


def source_text(root: Path, *, domain: str | None = None) -> str:
    paths = sorted(
        path
        for path in root.rglob("*")
        if path.is_file() and path.suffix.lower() in {".h", ".hpp", ".cpp"}
        and (domain is None or domain in path.parts)
    )
    if not paths:
        raise FileNotFoundError(f"no UE sources discovered under {root}")
    return "\n".join(path.read_text(encoding="utf-8") for path in paths)


def get_contract_data() -> dict:
    api_url = os.environ.get("FORBOCAI_API_URL")
    if api_url:
        try:
            url = api_url.rstrip("/") + "/test-game/contract"
            request = urllib.request.Request(url)
            api_key = os.environ.get("FORBOCAI_API_KEY")
            if api_key:
                request.add_header("Authorization", f"Bearer {api_key}")
            with urllib.request.urlopen(request) as response:
                if response.status == 200:
                    print(f"Fetched contract from {url}")
                    return json.loads(response.read().decode("utf-8"))
        except Exception as e:
            print(f"Failed to fetch contract from {api_url}: {e}", file=sys.stderr)

    candidates: list[Path] = []
    for env_name in ("API_TEST_GAME_CONTRACT", "TEST_GAME_CONTRACT", "UE_API_CONTRACT"):
        value = os.environ.get(env_name)
        if value:
            raw = Path(value)
            candidates.append(raw if raw.is_absolute() else Path.cwd() / raw)

    candidates.extend(
        [
            Path.cwd() / "api-checkout/api/contract/test-game-contract.json",
            REPO_ROOT / "api-checkout/api/contract/test-game-contract.json",
            WORKSPACE_ROOT / "api/api/contract/test-game-contract.json",
        ]
    )

    for candidate in candidates:
        if candidate.exists():
            print(f"API contract: {candidate.resolve()}")
            return json.loads(candidate.read_text(encoding="utf-8"))

    print("API test-game contract not found.", file=sys.stderr)
    print(
        "Set API_TEST_GAME_CONTRACT to ForbocAI/api/api/contract/test-game-contract.json or FORBOCAI_API_URL.",
        file=sys.stderr,
    )
    sys.exit(2)


def validate_contract_shape(contract: dict) -> list[str]:
    failures: list[str] = []
    if not contract.get("version"):
        failures.append("contract is missing version")
    if not contract.get("slotContractVersion"):
        failures.append("contract is missing slotContractVersion")
    if not isinstance(contract.get("requiredCommandGroups"), list):
        failures.append("contract is missing requiredCommandGroups")
    if not isinstance(contract.get("aliasRules"), dict):
        failures.append("contract is missing aliasRules")
    elif any(not value for value in contract["aliasRules"].values()):
        failures.append("contract contains an empty alias rule")
    if not isinstance(contract.get("scenarios"), list):
        failures.append("contract is missing scenarios")
    return failures


def validate_contract_matrix(contract: dict) -> list[str]:
    failures: list[str] = []
    scenarios = contract.get("scenarios") or []
    required_groups = set(contract.get("requiredCommandGroups") or [])
    seen_ids: set[str] = set()
    used_groups: set[str] = set()

    for scenario in scenarios:
        scenario_id = scenario.get("id")
        if not scenario_id:
            failures.append("scenario is missing id")
            continue
        if scenario_id in seen_ids:
            failures.append(f"duplicate scenario id: {scenario_id}")
        seen_ids.add(scenario_id)

        for command in scenario.get("commands") or []:
            group = command.get("group")
            if group not in required_groups:
                failures.append(
                    f"{scenario_id}: command group {group} is not in requiredCommandGroups"
                )
            used_groups.add(group)
            routes = command.get("expectedRoutes")
            if not isinstance(routes, list) or not routes:
                failures.append(f"{scenario_id}: command {command.get('command')} has no expectedRoutes")

    failures.extend(sorted_difference(required_groups, used_groups, "contract command group usage"))
    return failures


def parse_transcript_fields(header: str) -> set[str]:
    match = re.search(
        r"struct\s+FTranscriptEntry\s*\{(?P<body>.*?)^\};",
        header,
        re.S | re.M,
    )
    if not match:
        return set()
    return set(re.findall(r"\b(?:FString|ECommandGroup|TArray<FString>|ETranscriptStatus)\s+(\w+);", match.group("body")))


def validate_ue_sources(contract: dict) -> list[str]:
    failures: list[str] = []
    test_game_root = (
        PLUGIN_ROOT
        / "test-game-cli"
        / "Source"
        / "ForbocAI_TestGame_CLI"
        / "Public"
        / "TestGame"
    )
    all_sources = source_text(test_game_root)
    contract_sources = source_text(test_game_root / "Features", domain="Contract")
    parser_sources = source_text(
        test_game_root / "Features" / "Systems" / "Contract" / "Parsing"
    )
    contract_types_path = (
        test_game_root / "Features" / "Systems" / "Contract" / "ContractTypes.h"
    )
    contract_adapters_path = (
        test_game_root / "Features" / "Systems" / "Contract" / "ContractAdapters.h"
    )
    contract_data_path = (
        PLUGIN_ROOT / "test-game-cli" / "Content" / "Data" /
        "systems" / "contract.json"
    )
    runtime_data_path = (
        PLUGIN_ROOT / "test-game-cli" / "Content" / "Data" /
        "harness" / "runtime.json"
    )
    game_data_path = (
        PLUGIN_ROOT / "test-game-cli" / "Content" / "Data" /
        "harness" / "game.json"
    )
    runner_data_path = (
        PLUGIN_ROOT / "test-game-cli" / "Content" / "Data" /
        "harness" / "command-runner.json"
    )
    runner_aliases = json.loads(
        runner_data_path.read_text(encoding="utf-8")
    ).get("aliases", {})
    contract_aliases = contract.get("aliasRules") or {}
    contract_data = json.loads(contract_data_path.read_text(encoding="utf-8"))
    runtime_data = json.loads(runtime_data_path.read_text(encoding="utf-8"))
    game_data = json.loads(game_data_path.read_text(encoding="utf-8"))
    schema = contract_data.get("schema") or {}
    contract_types = contract_types_path.read_text(encoding="utf-8")
    contract_adapters = contract_adapters_path.read_text(encoding="utf-8")

    failures.extend(validate_schema_contract(
        contract,
        schema,
        parse_contract_schema_fields(contract_types),
        parse_schema_references(parser_sources),
    ))
    failures.extend(validate_schema_loader(contract_adapters))
    failures.extend(validate_runtime_command_groups(contract, runtime_data, game_data))
    failures.extend(validate_parser_shapes(parser_sources))

    for runtime_alias, runtime_value in sorted(runner_aliases.items()):
        contract_field = f"{runtime_alias}Alias"
        if contract_aliases.get(contract_field) != runtime_value:
            failures.append(
                f"UE runtime alias {runtime_alias}={runtime_value} does not match "
                f"API aliasRules.{contract_field}={contract_aliases.get(contract_field)}"
            )
    expected_transcript_fields = {
        "Id",
        "ScenarioId",
        "CommandGroup",
        "Command",
        "ExpectedRoutes",
        "Status",
        "Output",
        "Timestamp",
    }
    transcript_fields = parse_transcript_fields(all_sources)
    failures.extend(
        sorted_difference(expected_transcript_fields, transcript_fields, "UE transcript fields")
    )

    required_transcript_snippets = [
        "ScenarioId",
        "CommandGroup",
        "Command",
        "ExpectedRoutes",
        "Status",
        "Output",
    ]
    for snippet in required_transcript_snippets:
        if snippet not in all_sources:
            failures.append(f"UE transcript recording missing {snippet}")

    return failures


def main() -> int:
    contract = get_contract_data()

    failures = [
        *validate_contract_shape(contract),
        *validate_contract_matrix(contract),
        *validate_ue_sources(contract),
    ]

    if failures:
        print("UE API contract parity check failed:", file=sys.stderr)
        for failure in failures:
            print(f"- {failure}", file=sys.stderr)
        return 1

    scenario_count = len(contract.get("scenarios") or [])
    group_count = len(contract.get("requiredCommandGroups") or [])
    route_count = sum(
        1
        for scenario in contract.get("scenarios") or []
        for command in scenario.get("commands") or []
        for route in command.get("expectedRoutes") or []
        if route != "local only"
    )
    print(
        f"UE API contract parity OK - {scenario_count} scenarios, "
        f"{group_count} groups, {route_count} API routes."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
