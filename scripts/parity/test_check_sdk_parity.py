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
    def test_finds_matrix_by_declaration_content(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            ordinary = root / "ordinary.ts"
            matrix = root / "commands.ts"
            ordinary.write_text("export const unrelated = true;\n", encoding="utf-8")
            matrix.write_text(
                "export const NODE_CLI_COMMAND_KEYS = ['status', 'npc process'] as const;\n",
                encoding="utf-8",
            )

            source, keys = PARITY.find_matrix_source(
                (ordinary, matrix),
                PARITY.extract_ts_node_keys,
                "TS CLI command matrix",
            )

            self.assertEqual(source, matrix)
            self.assertEqual(keys, ["status", "npc process"])

    def test_rejects_ambiguous_matrix_owners(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            sources = []
            for name in ("first.ts", "second.ts"):
                source = root / name
                source.write_text(
                    "export const NODE_CLI_COMMAND_KEYS = ['status'] as const;\n",
                    encoding="utf-8",
                )
                sources.append(source)

            with self.assertRaisesRegex(ValueError, "Expected exactly one"):
                PARITY.find_matrix_source(
                    sources,
                    PARITY.extract_ts_node_keys,
                    "TS CLI command matrix",
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
