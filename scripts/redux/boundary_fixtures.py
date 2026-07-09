#!/usr/bin/env python3
"""Meta-test for the boundary guards.

Each fixture asserts a rule fires on a bad snippet (and that a clean snippet
stays silent), so a rule that silently stops matching is caught. This tests the
LINTER, not Source: it is never wired into run-tests and is not a substitute for
scanning Source (that is what check_redux.py does). Run it directly:

    python3 Scripts/Checks/boundary_fixtures.py
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import sys
import tempfile

# This fixture lives in redux/ next to the engine it tests. Put this folder on
# the path for features_boundaries and the parent Checks/ dir for check_redux.
_HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(_HERE.parent))
sys.path.insert(0, str(_HERE))

import check_redux
from features_boundaries import build_unit


@dataclass
class Case:
    name: str
    rel: str  # path under the temp project, e.g. "Source/Features/Demo/DemoTypes.h"
    body: str
    expect: set[str]  # rule ids that MUST fire
    forbid: set[str] = frozenset()  # rule ids that must NOT fire


CASES: list[Case] = [
    Case("nonserializable-types", "Source/Features/Demo/State/StateTypes.h",
         "struct FDemo { AActor *Owner; };", {"RTK-TYPES-001"}),
    Case("adapter-types-exempt", "Source/Features/Demo/Adapters/AdaptersTypes.h",
         "struct FDemo { TFunction<int(int)> Project; };", set(), {"RTK-TYPES-001"}),
    Case("component-named-state", "Source/Features/Demo/State/StateTypes.h",
         "struct FLoginScreenState { int X; };", {"RTK-TYPES-004"}),
    Case("scratchpad-decl-once", "Source/Features/Demo/State/StateTypes.h",
         "struct FDemo { int LastValue; };\nbool eq(FDemo L, FDemo R){ return L.LastValue == R.LastValue; }",
         {"RTK-TYPES-002"}),
    Case("setter-action", "Source/Features/Demo/DemoActions.h",
         "void SetPlayerHealth(int v);", {"RTK-ACTION-002"}),
    Case("switch-reducer", "Source/Features/Demo/DemoSlice.h",
         "int r(int s, Act action){ switch(action.type){ default: return s; } }", {"RTK-SLICE-002"}),
    Case("createreducer-ok", "Source/Features/Demo/DemoSlice.h",
         "auto s = rtk::createReducer<FState>(builder);", set(), {"RTK-SLICE-002"}),
    Case("rtk1-extra-reducers", "Source/Features/Demo/DemoSlice.h",
         "auto x = [Thunk.fulfilled]{};", {"RTK-SLICE-005"}),
    Case("thunk-no-condition", "Source/Features/Demo/DemoThunks.h",
         "auto t = rtk::createAsyncThunk<P>(prefix, fn);", {"RTK-THUNK-004"}),
    Case("thunk-polling", "Source/Features/Demo/DemoThunks.h",
         "auto t = rtk::createAsyncThunk<P>(p, [](auto Api){ while(true){ Api.getState(); } }, cfg_with_Condition);",
         {"RTK-THUNK-002"}),
    Case("selector-whole-state", "Source/Features/Demo/DemoSelectors.h",
         "FState pick(){ return State; }", {"RTK-SELECTOR-002"}),
    Case("qualified-selector-whole-state", "Source/Features/Demo/DemoSelectors.h",
         "FState pick(){ return State; }", {"RTK-SELECTOR-002"}, {"RTK-STRUCT-001"}),
    Case("listener-appended", "Source/Features/Demo/DemoListeners.h",
         "auto m = createListenerMiddleware<S>(); auto s = base.concat(m);", {"RTK-LISTENER-002"}),
    Case("fingerprint-mismatch", "Source/Features/Demo/DemoSelectors.h",
         "auto s = rtk::createSlice<FState>(name, r);", {"RTK-ROLE-001"}),
    Case("view-store-access", "Source/Views/Demo/DemoView.cpp",
         "void f(){ Store::GetStore().dispatch(a); }", {"RTK-VIEW-004"}),
    Case("qualified-view-store-access", "Source/Views/Demo/DemoView.cpp",
         "void f(){ Store::GetStore().dispatch(a); }", {"RTK-VIEW-004"}, {"RTK-STRUCT-001", "RTK-VIEW-001"}),
    Case("view-whole-state", "Source/Views/Demo/DemoView.cpp",
         "void f(){ auto s = SelectRuntimeState(); }", {"RTK-VIEW-007"}),
    Case("bare-role-leaf", "Source/Features/Demo/Actions.h",
         "int x;", {"RTK-STRUCT-001"}),
    Case("bare-view-leaf", "Source/Views/Demo/View.cpp",
         "int x;", {"RTK-STRUCT-001"}),
    Case("bad-leaf-name", "Source/Features/Demo/Reducers.h",
         "int x;", {"RTK-STRUCT-001"}),
    Case("clean-types", "Source/Features/Demo/State/StateTypes.h",
         "struct FDemo { int Health; FString Name; };", set(),
         {"RTK-TYPES-001", "RTK-TYPES-002", "RTK-TYPES-004"}),
]


def run_case(root: Path, plugins: dict, case: Case) -> list[str]:
    path = root / case.rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(case.body, encoding="utf-8")
    unit = build_unit(path, root)
    return [finding.rule_id for finding in check_redux.check_unit(unit, plugins)]


def main() -> int:
    plugins = check_redux.discover_role_plugins()
    failures: list[str] = []
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp)
        for case in CASES:
            fired = set(run_case(root, plugins, case))
            missing = case.expect - fired
            unexpected = case.forbid & fired
            if missing:
                failures.append(f"{case.name}: expected {sorted(missing)} to fire, got {sorted(fired)}")
            if unexpected:
                failures.append(f"{case.name}: {sorted(unexpected)} fired but is forbidden")

    if failures:
        print(f"Boundary fixtures FAILED: {len(failures)} case(s).")
        for failure in failures:
            print(f"  - {failure}")
        return 1
    print(f"Boundary fixtures passed: {len(CASES)} rule cases.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
