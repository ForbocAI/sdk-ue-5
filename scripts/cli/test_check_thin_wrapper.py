#!/usr/bin/env python3
"""Regression tests for the thin-wrapper guard."""

from __future__ import annotations

from pathlib import Path
import json
import sys
import tempfile
import unittest


sys.path.insert(0, str(Path(__file__).resolve().parent))

from check_thin_wrapper import collect_findings  # noqa: E402


TEST_DATA = json.loads(
    (
        Path(__file__).resolve().parents[2]
        / "Content"
        / "Data"
        / "tests"
        / "cli"
        / "thin-wrapper-guard.json"
    ).read_text(encoding="utf-8")
)


class ThinWrapperGuardTests(unittest.TestCase):
    def config(self) -> dict:
        fixture = TEST_DATA["fixture"]
        return {
            "requiredFiles": [fixture["requiredFile"]],
            "discovery": [],
            "sourceRules": [fixture["rule"]],
            "functionSize": fixture["functionSize"],
        }

    def run_guard(self, source: str):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / TEST_DATA["fixture"]["requiredFile"]).write_text(
                source, encoding="utf-8"
            )
            return collect_findings(root, self.config())[1]

    def test_source_cases(self):
        for case in TEST_DATA["sourceCases"]:
            with self.subTest(case["name"]):
                findings = self.run_guard(case["source"])
                self.assertEqual(
                    case["expectedRuleIds"],
                    [finding.rule_id for finding in findings],
                )
                if case.get("expectedMessageFragment"):
                    self.assertIn(case["expectedMessageFragment"], findings[0].message)

    def test_reports_missing_required_surface(self):
        with tempfile.TemporaryDirectory() as directory:
            findings = collect_findings(Path(directory), self.config())[1]
        self.assertEqual(
            TEST_DATA["missingRequiredExpectedRuleIds"],
            [finding.rule_id for finding in findings],
        )

    def test_function_binding_contract(self):
        case = TEST_DATA["bindingCase"]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            path = root / case["sourceFile"]
            path.write_text(case["source"], encoding="utf-8")
            config = self.config()
            config["requiredFiles"] = [case["sourceFile"]]
            config["sourceRules"] = []
            config["functionBindings"] = [
                {
                    key: value
                    for key, value in case.items()
                    if key
                    in {
                        "sourceFile",
                        "function",
                        "id",
                        "missingSummary",
                        "requiredSummary",
                        "forbiddenSummary",
                    }
                }
                | {
                    "requiredPatterns": case["passingRequiredPatterns"],
                    "forbiddenPatterns": case["passingForbiddenPatterns"],
                }
            ]
            self.assertEqual([], collect_findings(root, config)[1])
            config["functionBindings"][0]["requiredPatterns"] = case[
                "failingRequiredPatterns"
            ]
            config["functionBindings"][0]["forbiddenPatterns"] = case[
                "failingForbiddenPatterns"
            ]
            self.assertEqual(
                case["expectedFailureRuleIds"],
                [finding.rule_id for finding in collect_findings(root, config)[1]],
            )

    def test_role_dispatch_contract(self):
        case = TEST_DATA["roleCase"]
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / case["rolesFile"]).write_text(
                json.dumps(case["roles"]), encoding="utf-8"
            )
            (root / case["commandsFile"]).write_text(
                json.dumps(case["commands"]), encoding="utf-8"
            )
            source_path = root / case["sourceFile"]
            source_path.write_text(case["passingSource"], encoding="utf-8")
            config = self.config()
            config["requiredFiles"] = [case["sourceFile"]]
            config["sourceRules"] = []
            config["roleDispatchContracts"] = [
                {
                    key: value
                    for key, value in case.items()
                    if key
                    in {
                        "rolesFile",
                        "commandsFile",
                        "sourceFile",
                        "group",
                        "id",
                        "missingSummary",
                        "unmappedSummary",
                        "undispatchedSummary",
                    }
                }
            ]
            self.assertEqual([], collect_findings(root, config)[1])
            source_path.write_text(case["failingSource"], encoding="utf-8")
            self.assertEqual(
                case["expectedFailureRuleIds"],
                [finding.rule_id for finding in collect_findings(root, config)[1]],
            )


if __name__ == "__main__":
    unittest.main(verbosity=2)
