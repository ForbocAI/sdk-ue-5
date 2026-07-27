from __future__ import annotations

import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest


SCRIPTS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(SCRIPTS))
SCRIPT_PATH = SCRIPTS / "generate_authored_values.py"
SPEC = importlib.util.spec_from_file_location("generate_authored_values", SCRIPT_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"Could not load {SCRIPT_PATH}")
GENERATOR = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = GENERATOR
SPEC.loader.exec_module(GENERATOR)


class AuthoredValueMigrationTests(unittest.TestCase):
    def test_migration_prunes_only_unreferenced_bindings(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            plan = GENERATOR.TargetPlan(
                label="fixture",
                root=root,
                manifest=root / "Content/Data/cpp-authored-values.json",
                shard_directory=root / "Content/Data/cpp-authored-values",
                output=root / "Source/Generated/AuthoredValuesTypes.h",
                legacy_outputs=(),
                include="Generated/AuthoredValuesTypes.h",
                prefix="FORBOCAI_FIXTURE_AUTHORED_",
            )
            used = {
                "binding": GENERATOR.value_binding(plan, "number", "7"),
                "kind": "number",
                "token": "7",
            }
            unused = {
                "binding": GENERATOR.value_binding(plan, "string", '"obsolete"'),
                "kind": "string",
                "token": '"obsolete"',
            }
            GENERATOR.write_contract(plan, [used, unused])
            source = root / "Source/Feature/FeatureTypes.h"
            source.parent.mkdir(parents=True)
            source.write_text(
                "#pragma once\n"
                '#include "Generated/AuthoredValuesTypes.h"\n'
                f"constexpr int Value = {used['binding']};\n",
                encoding="utf-8",
            )

            GENERATOR.migrate_plan(plan)

            self.assertEqual(GENERATOR.load_entries(plan), [used])
            self.assertIn(used["binding"], plan.output.read_text(encoding="utf-8"))
            self.assertNotIn(
                unused["binding"], plan.output.read_text(encoding="utf-8")
            )


if __name__ == "__main__":
    unittest.main()
