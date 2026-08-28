from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
import sys


SCRIPTS_ROOT = Path(__file__).resolve().parents[1]
if str(SCRIPTS_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_ROOT))

from parity.report.sources import repository_identity
from parity.sdk.capabilities import build_cli_capabilities
from parity.sdk.commands import load_ts_cli_contract, load_ue_cli_contract, node_command_keys
from parity.sdk.discovery import build_parity_programs, unmapped_ts_roots
from parity.sdk.normalization import normalize_path_signature, ue_suffix_index
from parity.sdk.symbols import extract_ue_symbols


class ParityProgramDiscoveryTests(unittest.TestCase):
    def test_groups_packages_by_sdk_and_micro_game_without_mirror_tables(self) -> None:
        ts_roots = (
            Path("packages/runtime/src"),
            Path("packages/runtime-browser/src"),
            Path("packages/micro-game-runtime/src"),
        )
        ue_roots = (
            Path("Source/RuntimeSDK"),
            Path("micro-game-cli/Source/RuntimeMicroGame"),
        )

        programs = build_parity_programs(ts_roots, ue_roots)

        self.assertEqual(
            [program.label.split(":", 1)[0] for program in programs],
            ["Sdk", "Micro-game"],
        )
        self.assertEqual(programs[0].ts_source_roots, (Path("packages/runtime/src"),))
        self.assertEqual(
            programs[1].ts_source_roots,
            (Path("packages/micro-game-runtime/src"),),
        )

    def test_browser_programs_remain_explicit_runtime_mechanics(self) -> None:
        roots = (Path("packages/runtime/src"), Path("packages/runtime-browser/src"))
        programs = build_parity_programs(roots, (Path("Source/RuntimeSDK"),))

        self.assertEqual(
            unmapped_ts_roots(roots, programs),
            (Path("packages/runtime-browser/src"),),
        )


class CliContractTests(unittest.TestCase):
    def test_real_split_contracts_produce_only_same_capabilities(self) -> None:
        ue_root = Path(__file__).resolve().parents[2]
        ts_root = ue_root.parent / "sdk"
        ts = load_ts_cli_contract(ts_root)
        ue = load_ue_cli_contract(ue_root)

        groups = build_cli_capabilities(ts, ue)
        results = [item["result"] for group in groups.values() for item in group]

        self.assertEqual(node_command_keys(ts), node_command_keys(ue))
        self.assertNotIn("mismatch", results)


class PathNormalizationTests(unittest.TestCase):
    def test_preserves_ui_path_atoms_without_unreal_type_prefix_rules(self) -> None:
        self.assertEqual(
            normalize_path_signature("Features/Systems/Terminal/UI/UISlice.h"),
            "features/systems/terminal/ui/uislice",
        )

    def test_indexes_cpp_ancestor_qualified_roles(self) -> None:
        source = "Source/Runtime/Public/Entities/CLI/Ghost/CLIGhostSelectors.h"

        index = ue_suffix_index([source])

        self.assertEqual(index["entities/cli/ghost/ghostselectors"], [source])


class CppSymbolDiscoveryTests(unittest.TestCase):
    def test_finds_reference_returning_functions_without_namespace_symbols(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "FeatureSelectors.h"
            source.write_text(
                """#pragma once
namespace FeatureSelectors {
inline const FString &selectFeatureError(const FFeatureState &State) {
  return State.Error;
}
template <typename RootState>
inline const FFeatureState &selectFeatureState(const RootState &State) {
  return State.Feature;
}
}
""",
                encoding="utf-8",
            )

            symbols = extract_ue_symbols(source, root)

            self.assertEqual(
                [(symbol.name, symbol.kind) for symbol in symbols],
                [("selectFeatureError", "function"), ("selectFeatureState", "function")],
            )


class SourceIdentityTests(unittest.TestCase):
    def test_repository_identity_omits_host_path(self) -> None:
        self.assertEqual(repository_identity(Path("/workspace/Forboc.AI/sdk")), "sdk")


if __name__ == "__main__":
    unittest.main()
