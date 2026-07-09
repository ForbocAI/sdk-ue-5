#!/usr/bin/env python3
"""Meta-test for the FP guards (composition, branchless, parameter discipline).

Each case asserts a rule fires on a bad snippet and stays silent on a clean one,
so a rule that silently stops matching is caught. This tests the linters, not
Source; it is never wired into run-tests. Run it directly:

    python3 Scripts/Checks/fp/fp_fixtures.py
"""

from __future__ import annotations

from pathlib import Path
import sys
import tempfile

_HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(_HERE.parent))
sys.path.insert(0, str(_HERE))

import branchless_source as branch
import function_composition as comp
import param_count as arity
import source_conformance as source


def _write(root: Path, rel: str, body: str) -> Path:
    path = root / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(body, encoding="utf-8")
    return path


def composition_rules(root: Path, rel: str, body: str) -> set[str]:
    path = _write(root, rel, body)
    paths = [path]
    return {
        finding.rule_id
        for finding in comp.scan_file(path, comp.struct_name_context(paths), comp.qualified_name_context(paths))
    }


def branchless_rules(root: Path, rel: str, body: str) -> set[str]:
    return {finding.rule_id for finding in branch.scan_file(_write(root, rel, body))}


def arity_buckets(root: Path, rel: str, body: str) -> set[str]:
    path = _write(root, rel, body)
    return {
        bucket
        for _name, params, _line, bucket in arity.scan_file(path)
        if params > arity.MAX_DATA_PARAMS
    }


def source_rules(root: Path, rel: str, body: str) -> set[str]:
    path = _write(root, rel, body)
    findings = []
    findings.extend(source._scan_pattern(path, source.CONTROL_RE, source.NO_IMPERATIVE_LOOPS, lambda m: m.group(1)))
    findings.extend(source._scan_pattern(path, source.IF_RE, source.IF_STATEMENT, lambda _m: "if"))
    findings.extend(source._scan_pattern(path, source.SWITCH_RE, source.SWITCH_STATEMENT, lambda _m: "switch"))
    findings.extend(source._scan_pattern(path, source.MUTABLE_RE, source.PUBLIC_MUTABLE, lambda _m: "mutable"))
    findings.extend(source._scan_pattern(path, source.DIRECT_HTTP_RE, source.COMMAND_DIRECT_HTTP, lambda _m: "http"))
    findings.extend(source._scan_pattern(path, source.MOCK_RE, source.MOCK_REFERENCE, lambda _m: "mock"))
    findings.extend(source._scan_pattern(path, source.ACCUMULATION_RE, source.MUTABLE_ACCUMULATION, lambda _m: "accumulate"))
    findings.extend(source._scan_pattern(path, source.NULLISH_RE, source.NULLISH_BRANCH, lambda _m: "nullish"))
    return {finding.rule_id for finding in findings}


WIDE_STRUCT = """
struct FConversationUITestAutomationSettings {
  TArray<FString> Labels;
  FString PlayerRole;
  FString PlayerText;
  FString ExpectedPlayerMessageText;
  float ExpectedPlayerMessageGreen;
  int32 ExpectedHistoryCount;
  int32 TaggedHistoryIndex;
};
"""
NARROW_STRUCT = "struct FRuntimeOptions { FString A; FString B; FString C; FString D; FString E; FString F; };"
WRAPPER_FAMILY = "struct FOther { int32 X; };\nstruct FThingFields { FString Name; };"


def main() -> int:
    failures: list[str] = []
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        types = "Source/Features/Demo/State/StateTypes.h"

        comp_cases: list[tuple[str, set[str], str, set[str]]] = [
            ("comp-wide-struct", composition_rules(root, "Source/Features/Components/Data/Settings/UI/UITypes.h", WIDE_STRUCT), types, {"FP-COMP-006"}),
            ("comp-narrow-ok", composition_rules(root, types, NARROW_STRUCT), types, set()),
            ("comp-lazy-noun", composition_rules(root, types, "struct FThingManager { int32 Count; };"), types, {"FP-COMP-001"}),
            ("comp-wrapper-family", composition_rules(root, "Source/Features/Demo/Data/Registry/RegistryAdapters.h", WRAPPER_FAMILY), types, {"FP-COMP-004"}),
            ("comp-nested-compose", composition_rules(root, "Source/Features/Demo/Adapters/DemoAdapters.cpp", "auto f = compose(a, compose(b, c));"), types, {"FP-COMP-007"}),
            ("comp-clean", composition_rules(root, types, "struct FRuntimeState { int32 Value; };"), types, set()),
        ]
        for name, fired, _rel, want in comp_cases:
            if want == set():
                if fired & {"FP-COMP-001", "FP-COMP-004", "FP-COMP-006", "FP-COMP-007"}:
                    failures.append(f"{name}: unexpected composition rule fired: {sorted(fired)}")
            elif not want <= fired:
                failures.append(f"{name}: expected {sorted(want)}, got {sorted(fired)}")

        branch_cases = [
            ("branch-if", branchless_rules(root, "Source/Features/Demo/Slice.cpp", "int r(int s){ if (s) return s; return 0; }"), {"FP-BRANCH-001"}),
            ("branch-for", branchless_rules(root, "Source/Features/Demo/Slice.cpp", "void r(){ for (int i=0;i<3;++i) g(i); }"), {"FP-BRANCH-001"}),
            ("branch-ternary-chain", branchless_rules(root, "Source/Features/Demo/Slice.cpp", "int r(int s){ return s==0 ? 1 : s==1 ? 2 : s==2 ? 3 : 4; }"), {"FP-BRANCH-002"}),
            ("branch-goto", branchless_rules(root, "Source/Features/Demo/Slice.cpp", "void r(){ goto end; end: return; }"), {"FP-BRANCH-003"}),
            ("branch-clean", branchless_rules(root, "Source/Features/Demo/Slice.cpp", "int r(int s){ return s == 0 ? 1 : 2; }"), set()),
        ]
        for name, fired, want in branch_cases:
            if want == set():
                if fired:
                    failures.append(f"{name}: unexpected branchless rule fired: {sorted(fired)}")
            elif not want <= fired:
                failures.append(f"{name}: expected {sorted(want)}, got {sorted(fired)}")

        arity_cases = [
            ("arity-actionable", arity_buckets(root, "Source/Features/Demo/DemoAdapters.cpp", "int Foo(int a, int b, int c) { return a; }"), "actionable"),
            ("arity-reducer", arity_buckets(root, "Source/Features/Demo/DemoAdapters.cpp", "FState ReduceX(FState state, int a, int b) { return state; }"), "reducer"),
            ("arity-fn-arg", arity_buckets(root, "Source/Features/Demo/DemoAdapters.cpp", "int Bar(std::function<int()> fn, int a, int b) { return a; }"), "fn-arg"),
        ]
        for name, buckets, want in arity_cases:
            if want not in buckets:
                failures.append(f"{name}: expected bucket {want!r}, got {sorted(buckets)}")

        source_cases = [
            ("source-loop", source_rules(root, "Source/ForbocAI_SDK/Private/Demo.cpp", "void r(){ while (Ready()) Step(); }"), {"FP-SOURCE-001"}),
            ("source-if", source_rules(root, "Source/ForbocAI_SDK/Private/Demo.cpp", "void r(){ if (Ready()) Step(); }"), {"FP-SOURCE-006"}),
            ("source-switch", source_rules(root, "Source/ForbocAI_SDK/Private/Demo.cpp", "void r(){ switch (Mode) { default: break; } }"), {"FP-SOURCE-007"}),
            ("source-mutable", source_rules(root, "Source/ForbocAI_SDK/Public/Demo.h", "struct FDemo { mutable int32 Value; };"), {"FP-SOURCE-003"}),
            ("source-http", source_rules(root, "Source/ForbocAI_SDK/Private/CLI/Demo.cpp", "auto Request = FHttpModule::Get().CreateRequest();"), {"FP-SOURCE-005"}),
            ("source-mock", source_rules(root, "Source/ForbocAI_SDK/Public/Demo.h", "struct FMockRuntimeState {};"), {"FP-SOURCE-008"}),
            ("source-accumulation", source_rules(root, "Source/ForbocAI_SDK/Private/Demo.cpp", "void r(){ Items.Add(Value); Names.push(Value); }"), {"FP-SOURCE-009"}),
            ("source-nullish", source_rules(root, "Source/ForbocAI_SDK/Private/Demo.cpp", "void r(){ if (Ptr != nullptr) Step(); if (value === null) step(); }"), {"FP-SOURCE-010"}),
            ("source-clean", source_rules(root, "Source/ForbocAI_SDK/Public/Demo.h", "struct FRuntimeState { int32 Value; };"), set()),
        ]
        for name, fired, want in source_cases:
            if want == set():
                if fired:
                    failures.append(f"{name}: unexpected source rule fired: {sorted(fired)}")
            elif not want <= fired:
                failures.append(f"{name}: expected {sorted(want)}, got {sorted(fired)}")

        if "Either" not in source.missing_ue_surface_names("struct Maybe {};"):
            failures.append("surface-ue: expected Either to be reported missing")
        if "nothing" not in source.missing_ts_surface_names("export const just = () => undefined;\n"):
            failures.append("surface-ts: expected nothing to be reported missing")

    total = len(comp_cases) + len(branch_cases) + len(arity_cases) + len(source_cases) + 2
    if failures:
        print(f"FP fixtures FAILED: {len(failures)} case(s).")
        for failure in failures:
            print(f"  - {failure}")
        return 1
    print(f"FP fixtures passed: {total} checks.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
