#!/usr/bin/env python3
"""Regression tests for the thin-wrapper guard."""

from __future__ import annotations

from pathlib import Path
import sys
import tempfile
import unittest


sys.path.insert(0, str(Path(__file__).resolve().parent))

from check_thin_wrapper import collect_findings  # noqa: E402


class ThinWrapperGuardTests(unittest.TestCase):
    def config(self, maximum: int = 5) -> dict:
        return {
            "requiredFiles": ["surface.cpp"],
            "discovery": [],
            "sourceRules": [
                {
                    "id": "CLI-THIN-HTTP",
                    "summary": "direct HTTP",
                    "pattern": "\\bFHttpModule\\b",
                }
            ],
            "functionSize": {
                "id": "CLI-THIN-SIZE",
                "summary": "too large",
                "maxLines": maximum,
            },
        }

    def run_guard(self, source: str, maximum: int = 5):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "surface.cpp").write_text(source, encoding="utf-8")
            return collect_findings(root, self.config(maximum))[1]

    def test_reports_real_function_line_count(self):
        findings = self.run_guard(
            "int Oversized() {\n  int Value = 1;\n  Value += 1;\n  Value += 1;\n  Value += 1;\n  return Value;\n}\n"
        )
        self.assertEqual(["CLI-THIN-SIZE"], [finding.rule_id for finding in findings])
        self.assertIn("Oversized is 7 lines", findings[0].message)

    def test_ignores_rule_patterns_inside_comments(self):
        findings = self.run_guard("int Thin() {\n  // FHttpModule\n  return 0;\n}\n")
        self.assertEqual([], findings)

    def test_reports_executable_rule_patterns(self):
        findings = self.run_guard("int Thin() {\n  return FHttpModule();\n}\n")
        self.assertEqual(["CLI-THIN-HTTP"], [finding.rule_id for finding in findings])

    def test_reports_missing_required_surface(self):
        with tempfile.TemporaryDirectory() as directory:
            findings = collect_findings(Path(directory), self.config())[1]
        self.assertEqual(["CLI-THIN-000"], [finding.rule_id for finding in findings])


if __name__ == "__main__":
    unittest.main(verbosity=2)
