"""Regression tests for source-derived API-contract parity."""

from __future__ import annotations

import unittest

from api_contract import (
    nested_object_keys,
    parse_contract_schema_fields,
    parse_schema_references,
    validate_parser_shapes,
    validate_runtime_command_groups,
    validate_schema_contract,
)


class ApiContractParityTests(unittest.TestCase):
    def test_discovers_nested_wire_fields_without_a_field_allowlist(self) -> None:
        contract = {
            "version": "v1",
            "scenarios": [{"id": "one", "commands": [{"group": "status"}]}],
        }
        self.assertEqual(
            nested_object_keys(contract),
            {"version", "scenarios", "id", "commands", "group"},
        )

    def test_compares_authored_schema_to_source_discovery(self) -> None:
        contract = {"version": "v1", "scenarios": [{"id": "one"}]}
        schema = {"version": "version", "scenarios": "scenarios", "id": "id"}
        declarations = parse_contract_schema_fields(
            "#define FORBOCAI_CONTRACT_SCHEMA_FIELDS(X) \\\n"
            "  X(version) \\\n"
            "  X(scenarios) \\\n"
            "  X(id)\n\n"
            "struct FContractSchemaData {};"
        )
        references = parse_schema_references(
            "Schema.version; ContractData().Schema.scenarios; Schema.id;"
        )
        self.assertEqual(
            validate_schema_contract(contract, schema, declarations, references),
            [],
        )

    def test_requires_exact_runtime_groups_plus_authored_bootstrap(self) -> None:
        contract = {"requiredCommandGroups": ["status", "memory_list"]}
        runtime = {
            "commandGroups": {
                "status": True,
                "memory_list": True,
                "contract": True,
            }
        }
        harness_groups = {"contract"}
        self.assertEqual(
            validate_runtime_command_groups(contract, runtime, harness_groups),
            [],
        )
        self.assertIn(
            "UE runtime command groups: missing memory_list",
            validate_runtime_command_groups(
                contract,
                {"commandGroups": {"status": True, "contract": True}},
                harness_groups,
            ),
        )

    def test_requires_direct_typed_parser_shapes(self) -> None:
        source = """
TParseResult<FCommandSpec> ParseCommand(const Value &Value) {
  Command.ExpectedRoutes = Routes;
  return GameRuntimeData().commandGroups.all.Contains(Command.Group);
}
TParseResult<FScenarioStep> ParseScenario(const Value &Value) { return {}; }
"""
        self.assertEqual(validate_parser_shapes(source), [])


if __name__ == "__main__":
    unittest.main()
