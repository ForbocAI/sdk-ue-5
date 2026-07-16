from __future__ import annotations

import importlib.util
from pathlib import Path
import sys
import unittest


SCRIPT_PATH = Path(__file__).resolve().parents[1] / "check-rtk-parity.py"
SPEC = importlib.util.spec_from_file_location("check_rtk_parity", SCRIPT_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"Could not load {SCRIPT_PATH}")
PARITY = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = PARITY
SPEC.loader.exec_module(PARITY)


class MapPortabilityTests(unittest.TestCase):
    def test_generated_section_uses_repository_relative_identities(self) -> None:
        ts_root = Path("/workspace/Forboc.AI/sdk")
        ue_root = Path("/workspace/Forboc.AI/sdk-ue-5")

        section, missing = PARITY.build_section(
            "2.11.2",
            ts_root,
            ue_root,
            (ue_root / "Source/ForbocAI_SDK/Public/Core/rtk.hpp",),
            (PARITY.ExportGroup("Toolkit runtime", ("configureStore",)),),
            ("configureStore",),
        )

        self.assertEqual(missing, 0)
        self.assertNotIn("/workspace/", section)
        self.assertIn("TS repository `sdk`", section)
        self.assertIn(
            "`Source/ForbocAI_SDK/Public/Core/rtk.hpp` in UE repository `sdk-ue-5`",
            section,
        )


if __name__ == "__main__":
    unittest.main()
