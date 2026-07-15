from __future__ import annotations

import importlib.util
from pathlib import Path
import sys
import tempfile
import unittest


SCRIPT_PATH = Path(__file__).resolve().parents[1] / "check-sdk-parity.py"
SPEC = importlib.util.spec_from_file_location("check_sdk_parity", SCRIPT_PATH)
if SPEC is None or SPEC.loader is None:
    raise RuntimeError(f"Could not load {SCRIPT_PATH}")
PARITY = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = PARITY
SPEC.loader.exec_module(PARITY)


class ParityProgramDiscoveryTests(unittest.TestCase):
    def test_groups_discovered_packages_without_exact_package_tables(self) -> None:
        ts_roots = (
            Path("packages/runtime/src"),
            Path("packages/runtime-browser/src"),
            Path("packages/test-game-runtime/src"),
        )
        ue_roots = (
            Path("Source/RuntimeSDK"),
            Path("test-game-cli/Source/RuntimeTestGame"),
        )

        programs = PARITY.build_parity_programs(ts_roots, ue_roots)

        self.assertEqual([program.label.split(":", 1)[0] for program in programs], ["Sdk", "Test Game"])
        self.assertEqual(programs[0].ts_source_roots, (Path("packages/runtime/src"),))
        self.assertEqual(programs[1].ts_source_roots, (Path("packages/test-game-runtime/src"),))
        self.assertNotIn(Path("packages/runtime-browser/src"), programs[0].ts_source_roots)

    def test_browser_programs_remain_unmapped(self) -> None:
        roots = (
            Path("packages/runtime/src"),
            Path("packages/runtime-browser/src"),
        )
        programs = PARITY.build_parity_programs(roots, (Path("Source/RuntimeSDK"),))

        self.assertEqual(
            PARITY.unmapped_ts_roots(roots, programs),
            (Path("packages/runtime-browser/src"),),
        )


class MatrixDiscoveryTests(unittest.TestCase):
    def test_finds_node_keys_from_authored_data_schema(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ordinary = root / "ordinary.json"
            catalog = root / "catalog.json"
            ordinary.write_text('{"unrelated": true}\n', encoding="utf-8")
            catalog.write_text(
                """{
  "surfaces": {"node": {}, "browser": {}},
  "commands": {
    "status": {"surfaces": ["node", "browser"]},
    "npc process": {"surfaces": ["node"]},
    "browser only": {"surfaces": ["browser"]}
  }
}
""",
                encoding="utf-8",
            )

            source, keys = PARITY.find_matrix_source(
                (ordinary, catalog),
                PARITY.extract_ts_node_keys,
                "TS CLI authored-data catalog",
            )

            self.assertEqual(source, catalog)
            self.assertEqual(keys, ["status", "npc process"])

    def test_rejects_ambiguous_catalog_owners(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            sources = []
            for name in ("first.json", "second.json"):
                source = root / name
                source.write_text(
                    """{
  "surfaces": {"node": {}},
  "commands": {"status": {"surfaces": ["node"]}}
}
""",
                    encoding="utf-8",
                )
                sources.append(source)

            with self.assertRaisesRegex(ValueError, "Expected exactly one"):
                PARITY.find_matrix_source(
                    sources,
                    PARITY.extract_ts_node_keys,
                    "TS CLI authored-data catalog",
                )


class PathNormalizationTests(unittest.TestCase):
    def test_preserves_ui_path_atoms_without_unreal_type_prefix_rules(self) -> None:
        self.assertEqual(
            PARITY.normalize_path_signature("Features/Systems/Terminal/UI/UISlice.h"),
            "features/systems/terminal/ui/uislice",
        )


class CppSymbolDiscoveryTests(unittest.TestCase):
    def test_finds_reference_returning_functions_without_namespace_symbols(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "FeatureSelectors.h"
            source.write_text(
                """\
#pragma once

namespace FeatureSelectors {

inline const FString &selectFeatureError(const FFeatureState &State) {
  return State.Error;
}

template <typename RootState>
inline const FFeatureState &selectFeatureState(const RootState &State) {
  return State.Feature;
}

} // namespace FeatureSelectors
""",
                encoding="utf-8",
            )

            symbols = PARITY.extract_ue_symbols(source, root)

            self.assertEqual(
                [(symbol.name, symbol.kind) for symbol in symbols],
                [
                    ("selectFeatureError", "function"),
                    ("selectFeatureState", "function"),
                ],
            )


if __name__ == "__main__":
    unittest.main()
