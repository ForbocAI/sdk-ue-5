from __future__ import annotations

import unittest
from pathlib import Path
import sys


SCRIPTS_ROOT = Path(__file__).resolve().parents[1]
if str(SCRIPTS_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_ROOT))

from parity.rtk.capabilities import build_capability_groups
from parity.rtk.types import ExportGroup


class RtkCapabilityTests(unittest.TestCase):
    def test_distinguishes_present_exports_from_actionable_mismatches(self) -> None:
        groups = (
            ExportGroup(
                "toolkit-runtime",
                "Toolkit runtime",
                ("configureStore", "createSlice"),
                ("node_modules/toolkit.js",),
            ),
        )

        capabilities = build_capability_groups(
            groups,
            {"configureStore": ("Source/Public/Core/rtk.hpp",)},
        )["toolkit-runtime"]

        self.assertEqual([item["result"] for item in capabilities], ["same", "mismatch"])
        self.assertIsNone(capabilities[0]["mismatch"])
        self.assertIn("createSlice", capabilities[1]["mismatch"]["message"])
        self.assertTrue(capabilities[1]["mismatch"]["remediation"])


if __name__ == "__main__":
    unittest.main()
