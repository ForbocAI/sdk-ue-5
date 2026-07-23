#!/usr/bin/env python3
"""RTK boundary guard: one runner over every Redux role.

Role rules live in auto-discovered ``*_boundaries.py`` plugins (Actions,
Adapters, Listeners, Selectors, Slice, Thunks, Types, View). This runner owns
the cross-role structure: valid role leaves, import direction, the single store
boundary, dispatch/getState placement, pure-role purity, RTK Query lifecycle,
and the feature import graph. Every finding carries a stable rule id and a
severity aligned to the redux/rtk skills.

    check_redux.py [--format text|json|sarif] [--explain [RULE-ID]]

The production CLI scans the configured project root's feature/view roots.
FORBOCAI_REDUX_PROJECT_ROOT and FORBOCAI_REDUX_SCAN_ROOTS let the UE demo use
this SDK-owned checker without copying the rules back into the demo.
"""

from __future__ import annotations

import argparse
import importlib
import os
from pathlib import Path
import re
import sys

# The redux role plugins and their shared engine (features_boundaries) live in
# the sibling redux/ folder; put it on the path before importing either.
REDUX_DIR = Path(__file__).resolve().parent / "redux"
sys.path.insert(0, str(REDUX_DIR))
PROJECT_ROOT = Path(os.environ.get("FORBOCAI_REDUX_PROJECT_ROOT", Path(__file__).resolve().parents[1])).resolve()
from ue_targets import ue_targets


def default_scan_roots(project_root: Path) -> list[Path]:
    return [
        project_root / "Source" / "Features",
        project_root / "Source" / "Views",
        project_root / "Source" / "ForbocAI_SDK" / "Public" / "Features",
        project_root / "Source" / "ForbocAI_SDK" / "Private" / "Features",
        project_root / "test-game-cli" / "Source" / "ForbocAI_TestGame_CLI" / "Public" / "TestGame" / "Features",
        project_root / "test-game-cli" / "Source" / "ForbocAI_TestGame_CLI" / "Public" / "TestGame" / "Views",
    ]


def discovered_target_scan_roots(project_root: Path) -> list[tuple[str, Path, list[Path]]]:
    targets: list[tuple[str, Path, list[Path]]] = []
    for target in ue_targets():
        if target.kind == "sdk":
            roots = [
                target.root / "Source" / "ForbocAI_SDK" / "Public" / "Features",
                target.root / "Source" / "ForbocAI_SDK" / "Private" / "Features",
            ]
        elif target.kind == "sdk-cli":
            roots = [
                target.root / "Source" / "ForbocAI_TestGame_CLI" / "Public" / "TestGame" / "Features",
                target.root / "Source" / "ForbocAI_TestGame_CLI" / "Public" / "TestGame" / "Views",
            ]
        else:
            roots = [
                target.root / "Source" / "Features",
                target.root / "Source" / "Views",
            ]
        existing = [root for root in roots if root.exists()]
        if existing:
            targets.append((target.label, target.root, existing))
    if targets:
        return targets
    return [("project", project_root, [root for root in default_scan_roots(project_root) if root.exists()])]


def configured_scan_roots(project_root: Path) -> list[Path]:
    raw = os.environ.get("FORBOCAI_REDUX_SCAN_ROOTS", "")
    if not raw:
        return default_scan_roots(project_root)
    roots = []
    for value in raw.split(os.pathsep):
        if not value:
            continue
        candidate = Path(value)
        roots.append(candidate if candidate.is_absolute() else project_root / candidate)
    return roots


SCAN_ROOTS = configured_scan_roots(PROJECT_ROOT)

from features_boundaries import (
    Finding,
    ROLE_STEM_BY_ROLE,
    RULES,
    Rule,
    Severity,
    SourceUnit,
    build_unit,
    explain,
    find_import_cycles,
    findings_for,
    fingerprint_findings,
    format_json,
    format_sarif,
    format_summary,
    format_text,
    iter_source_files,
    line_number,
    register,
    role_for_include,
    role_for_stem_suffix,
)


# --- Cross-role structure rules -------------------------------------------

STRUCT_LEAF = register(
    Rule(
        id="RTK-STRUCT-001",
        severity=Severity.HIGH,
        summary="feature leaf is not a folder-qualified RTK role name",
        guidance=(
            "Feature leaves must be the nearest unambiguous real domain atom "
            "plus the role, such as SkyThunks or TextureAdapters; keep broader "
            "domain words in folders. Do not use .inl fragments, Support/Setup "
            "leaves, RTK role folders like Slice/Thunks/Selectors, non-role "
            "implementation buckets like Reducers/Factories, or vague bucket "
            "atoms like Core/Runtime/Common. Put reducer transitions in Slice "
            "(skill: \"Use mutating logic inside slice reducers\"), derived reads "
            "in Selectors (skill: \"Derive values with selectors instead of "
            "storing duplicates\"), imperative effects in Thunks (skill: \"Use "
            "a thunk when you need one imperative async workflow\"), and reactive "
            "effects in Listeners. Never add suppression comments; fix the "
            "boundary by moving, splitting, or renaming the code."
        ),
        skill="model-redux-state-design-state-ownership: name by domain/role, not the component tree",
    )
)

ROOT_ROLE = register(
    Rule(
        id="RTK-STRUCT-002",
        severity=Severity.HIGH,
        summary="program-root source file declares an RTK role",
        guidance=(
            "Keep only the program store and public wiring/entry surfaces at the "
            "source root. Actions, Adapters, Api, Listeners, Selectors, Slice, "
            "Thunks, Types, and Views must live in their feature/view domain."
        ),
        skill="build-modern-redux-apps-modern-redux: keep app wiring at the root and feature logic in feature folders",
    )
)

IMPORT_DIRECTION = register(
    Rule(
        id="RTK-IMPORT-001",
        severity=Severity.HIGH,
        summary="import points against the RTK dataflow direction",
        guidance="Keep imports pointed action -> slice -> selector -> view, with thunks/listeners owning effects; a role must not import roles downstream of it.",
        skill="build-modern-redux-apps-redux-dataflow: event -> reducer -> selector -> render",
    )
)

IMPORT_STORE = register(
    Rule(
        id="RTK-IMPORT-002",
        severity=Severity.HIGH,
        summary="non-boundary file imports Store.h",
        guidance="Only the single store boundary imports Store.h; use Actions for dispatch facades, Selectors for reads, Thunks/Listeners for workflows.",
        skill="build-modern-redux-apps-modern-redux: HIGH importing the store in components",
    )
)

IMPORT_VIEW = register(
    Rule(
        id="RTK-IMPORT-003",
        severity=Severity.HIGH,
        summary="feature imports Views",
        guidance="Features never import Views; presentation depends on features, not the reverse.",
        skill="build-modern-redux-apps-modern-redux: keep app wiring and feature logic separate",
    )
)

STORE_GET = register(
    Rule(
        id="RTK-STORE-001",
        severity=Severity.HIGH,
        summary="non-boundary file calls Store::GetStore",
        guidance="Route through Actions, Selectors, Thunks, or Listeners; only the store boundary calls Store::GetStore.",
        skill="build-modern-redux-apps-modern-redux: HIGH importing the store in components",
    )
)

STORE_DISPATCH = register(
    Rule(
        id="RTK-STORE-002",
        severity=Severity.HIGH,
        summary="role dispatches when its role may not",
        guidance="Dispatch only from Actions (event facades), Thunks (imperative), or Listeners (reactive).",
        skill="orchestrate-side-effects-handle-side-effects: keep side effects out of reducers/UI",
    )
)

STORE_GET_STATE = register(
    Rule(
        id="RTK-STORE-003",
        severity=Severity.HIGH,
        summary="role calls getState when its role may not",
        guidance="Expose derived reads through Selectors; use getState only inside Thunks/Listeners.",
        skill="evolve-and-diagnose-redux-apps-debug-redux-toolkit-apps: derive through selectors",
    )
)

STORE_LEGACY = register(
    Rule(
        id="RTK-STORE-004",
        severity=Severity.HIGH,
        summary="legacy createStore/applyMiddleware boilerplate",
        guidance="Use configureStore; it keeps default middleware and dev checks instead of manual createStore/applyMiddleware.",
        skill="evolve-and-diagnose-redux-apps-migrate-to-modern-redux: replace the store setup first",
    )
)

STORE_ARRAY_MW = register(
    Rule(
        id="RTK-STORE-005",
        severity=Severity.CRITICAL,
        summary="removed RTK 2 array-form middleware config",
        guidance="Configure middleware with the (getDefaultMiddleware) => getDefaultMiddleware().concat(...) callback; RTK 2 removed the array form.",
        skill="evolve-and-diagnose-redux-apps-migrate-to-modern-redux: CRITICAL carrying removed RTK 2 config forms forward",
    )
)

STORE_CONFIGURE = register(
    Rule(
        id="RTK-STORE-006",
        severity=Severity.MEDIUM,
        summary="store boundary is not visibly configureStore-style",
        guidance="Assemble the store with configureStore so default middleware and dev checks remain the baseline.",
        skill="build-modern-redux-apps-modern-redux: HIGH keeping createStore boilerplate as the default",
    )
)

STORE_CONFIGURE_OUTSIDE_ROOT = register(
    Rule(
        id="RTK-STORE-007",
        severity=Severity.CRITICAL,
        summary="configureStore is assembled outside the program root store",
        guidance=(
            "Each stateful program has one root *Store boundary. Route runtime, "
            "CLI, view, and test behavior through that root; test feature reducers "
            "as pure transitions when a separate store is unnecessary."
        ),
        skill="build-modern-redux-apps-modern-redux: create and provide one app store at the root",
    )
)

STORE_MULTIPLE_ROOTS = register(
    Rule(
        id="RTK-STORE-008",
        severity=Severity.CRITICAL,
        summary="program declares more than one logical root store",
        guidance=(
            "Keep one root *Store per stateful program. A header/source pair with "
            "the same stem is one boundary; distinct store stems are competing roots."
        ),
        skill="model-redux-state-design-state-ownership: keep one authority for each state domain",
    )
)

PURE_ROLE = register(
    Rule(
        id="RTK-PURE-001",
        severity=Severity.HIGH,
        summary="pure role (Slice/Selectors/Types) performs a side effect",
        guidance="Keep reducers/selectors/types pure; asset loads, spawns, timers, IO, and console commands live in Thunks/Listeners/Adapters.",
        skill="orchestrate-side-effects-handle-side-effects: CRITICAL running side effects inside reducers",
    )
)

QUERY_LIFECYCLE = register(
    Rule(
        id="RTK-QUERY-001",
        severity=Severity.MEDIUM,
        summary="createApi/injectEndpoints without tags or endpoint lifecycles",
        guidance="Declare providesTags/invalidatesTags or endpoint lifecycles so cache invalidation and optimistic updates stay coupled to requests.",
        skill="manage-server-data-adopt-rtk-query: use tags for cache invalidation",
    )
)

QUERY_MULTI_ROOT = register(
    Rule(
        id="RTK-QUERY-002",
        severity=Severity.CRITICAL,
        summary="more than one createApi root for the same backend",
        guidance="Keep one API slice per base URL and extend it with injectEndpoints; multiple roots break invalidation and duplicate middleware.",
        skill="manage-server-data-adopt-rtk-query: CRITICAL creating multiple API slices for one backend",
    )
)

GRAPH_CYCLE = register(
    Rule(
        id="RTK-GRAPH-001",
        severity=Severity.HIGH,
        summary="feature role import cycle",
        guidance="Break the include cycle; the feature import graph must stay a DAG in the action -> slice -> selector -> view direction.",
        skill="model-redux-state-design-state-ownership: re-size slices as access patterns change",
    )
)

SUPPRESSION_COMMENT = register(
    Rule(
        id="RTK-SUPPRESS-001",
        severity=Severity.HIGH,
        summary="suppression comment attempts to bypass the RTK guard",
        guidance=(
            "Never add suppression or allow comments such as rtk:suppress, "
            "boundary-allow, NOLINT, or ignore directives. Fix the boundary by "
            "moving, splitting, renaming, or changing the rule when the rule is "
            "wrong."
        ),
        skill="redux/rtk doctrine: fix boundaries instead of hiding violations",
    )
)


# --- Cross-role data -------------------------------------------------------

LEAF_SUFFIX_GUIDANCE: tuple[tuple[str, str], ...] = (
    (
        "Reducers",
        "Reducers is not an RTK role. Move reducer transitions/helpers into the sibling Slice; RTK skill: \"Use mutating logic inside slice reducers\" and \"Hand-written reducers are an escape hatch\".",
    ),
    (
        "Factories",
        "Factories is not an RTK role. Replace by responsibility: initial-state/createSlice assembly -> Slice; boundary translation -> Adapters; imperative effects -> Thunks (skill: \"Use a thunk when you need one imperative async workflow\"); reactive effects -> Listeners.",
    ),
    ("Dispatch", "Replace Dispatch files with Actions/Thunks/Listeners; keep one store boundary."),
    ("StateTypes", "Move domain words into folders and keep the folder-qualified Types leaf (State/StateTypes.h or the shortest collision-free suffix)."),
    ("PayloadTypes", "Move domain words into folders and keep the folder-qualified Types leaf (Payload/PayloadTypes.h or the shortest collision-free suffix)."),
)

FORBIDDEN_TARGET_ROLES: dict[str, set[str]] = {
    "types": {"actions", "selectors", "thunks", "listeners", "adapters", "slice"},
    "actions": {"selectors", "listeners"},
    "adapters": {"actions", "listeners"},
    "selectors": {"actions", "thunks", "listeners"},
    "slice": {"listeners"},
}

STORE_BOUNDARY_RELS = {
    "Features/Systems/SystemsActions.cpp",
    "Features/Systems/SystemsListeners.cpp",
    "Features/Systems/SystemsSelectors.cpp",
    "Features/Systems/SystemsThunks.cpp",
}

RAW_DISPATCH_ALLOW_ROLES = {"actions", "thunks", "listeners"}
RAW_GET_STATE_ALLOW_ROLES = {"thunks", "listeners"}
GET_STATE_ALLOW_RELS = {"Features/Systems/SystemsSelectors.cpp"}

DISPATCH_RE = re.compile(r"(?<!RuntimeDispatch::)\bDispatch\s*\(|\.dispatch\s*\(|Api\.dispatch\b")
GET_STATE_RE = re.compile(r"\.getState\s*\(|Api\.getState\b")
STORE_GET_RE = re.compile(r"\bStore::GetStore\s*\(")

PURE_ROLES = {"slice", "selectors", "types"}
SIDE_EFFECT_PATTERNS: tuple[tuple[re.Pattern[str], str], ...] = (
    (re.compile(r"\bLoadObject\s*<"), "loads Unreal assets"),
    (re.compile(r"\bSpawnActor\s*<"), "spawns actors"),
    (re.compile(r"\bGetTimerManager\s*\("), "schedules timers"),
    (re.compile(r"\bFScreenshotRequest::"), "requests screenshots"),
    (re.compile(r"\bFParse::|\bFCommandLine::"), "parses command lines"),
    (re.compile(r"\bIFileManager::|\bFPaths::"), "performs filesystem IO"),
    (re.compile(r"\bConsoleCommand\s*\("), "issues console commands"),
)

RTK_QUERY_PATTERN = re.compile(r"\bcreateApi\s*<|\binjectEndpoints\s*\(")
RTK_QUERY_LIFECYCLE = re.compile(r"\bprovidesTags\b|\binvalidatesTags\b|\bonQueryStarted\b|\bonCacheEntryAdded\b")
CREATE_API = re.compile(r"\bcreateApi\s*<")

STORE_LEGACY_RE = re.compile(r"\bcreateStore\s*\(|\bapplyMiddleware\s*\(")
STORE_ARRAY_MW_RE = re.compile(r"\bmiddleware\s*[:=]\s*\[")
CONFIGURE_STORE_RE = re.compile(r"\brtk::configureStore\s*<|(?<!create)configureStore\s*\(")
APPLICATION_CONFIGURE_STORE_RE = re.compile(r"\brtk::configureStore\s*<")
SUPPRESSION_RE = re.compile(
    r"\b(?:rtk:suppress|boundary-allow|NOLINT|eslint-disable|ts-ignore|@ts-ignore|noinspection)"
    r"|\bpragma\s+warning\s*\(\s*disable",
    re.IGNORECASE,
)


# --- Plugin discovery ------------------------------------------------------

def discover_role_plugins() -> dict[str, object]:
    plugins: dict[str, object] = {}
    for file in sorted(REDUX_DIR.glob("*_boundaries.py")):
        module = importlib.import_module(file.stem)
        role = getattr(module, "ROLE", None)
        check = getattr(module, "check", None)
        if role and callable(check):
            plugins[role] = module
    return plugins


# --- Per-file checks -------------------------------------------------------

def _rel_key(path: Path) -> str | None:
    marker = "Source/Features/"
    text = path.as_posix()
    return "Features/" + text.split(marker, 1)[1] if marker in text else None


def leaf_guidance(stem: str) -> str:
    for suffix, guidance in LEAF_SUFFIX_GUIDANCE:
        if stem.endswith(suffix):
            return guidance
    return STRUCT_LEAF.guidance


def _camel_name(value: str) -> str:
    return "".join(
        token[:1].upper() + token[1:]
        for token in re.split(r"[^A-Za-z0-9]+", value)
        if token
    )


def _parts_after_marker(path: Path, marker: str) -> tuple[str, ...]:
    parts = path.parts
    if marker not in parts:
        return ()
    index = parts.index(marker)
    return tuple(parts[index + 1 : -1])


def _domain_parts(unit: SourceUnit) -> tuple[str, ...]:
    return _parts_after_marker(unit.path, "Features") or _parts_after_marker(unit.path, "Views")


def _domain_qualifiers(unit: SourceUnit) -> tuple[str, ...]:
    parts = _domain_parts(unit)
    return tuple(
        "".join(_camel_name(part) for part in parts[index:])
        for index in range(len(parts) - 1, -1, -1)
    )


def _nearest_leaf_stem(unit: SourceUnit) -> str | None:
    if unit.declared_role is None:
        return None
    qualifiers = _domain_qualifiers(unit)
    if not qualifiers:
        return None
    return qualifiers[0] + ROLE_STEM_BY_ROLE[unit.declared_role]


def _layered_leaf_stems(unit: SourceUnit) -> tuple[str, ...]:
    if unit.declared_role is None:
        return ()
    return tuple(
        qualifier + ROLE_STEM_BY_ROLE[unit.declared_role]
        for qualifier in _domain_qualifiers(unit)
    )


def check_feature_leaf_names(units: list[SourceUnit]) -> list[Finding]:
    role_units = [
        unit for unit in units
        if (_parts_after_marker(unit.path, "Features") or _parts_after_marker(unit.path, "Views"))
        and unit.declared_role is not None
    ]
    nearest_counts: dict[tuple[str, str], int] = {}
    for unit in role_units:
        nearest = _nearest_leaf_stem(unit)
        if nearest:
            key = (unit.declared_role or "", nearest)
            nearest_counts[key] = nearest_counts.get(key, 0) + 1

    findings: list[Finding] = []
    for unit in role_units:
        nearest = _nearest_leaf_stem(unit)
        if nearest is None or unit.stem == nearest:
            continue
        layered = _layered_leaf_stems(unit)
        conflict = nearest_counts.get((unit.declared_role or "", nearest), 0) > 1
        if conflict and unit.stem in layered[1:]:
            continue
        findings.append(
            Finding(
                unit.path,
                1,
                STRUCT_LEAF.id,
                STRUCT_LEAF.severity,
                f"feature leaf `{unit.stem}` must be `{nearest}`; only layer parent domains when `{nearest}` conflicts",
            )
        )
    return findings


def check_imports(unit: SourceUnit, role: str) -> list[Finding]:
    findings: list[Finding] = []
    rel = _rel_key(unit.path)
    for match in re.finditer(r'^\s*#\s*include\s+"([^"]+)"', unit.raw, re.MULTILINE):
        include = match.group(1)
        line = line_number(unit.raw, match.start())
        if include.startswith("Views/") or "/Views/" in include:
            findings.append(Finding(unit.path, line, IMPORT_VIEW.id, IMPORT_VIEW.severity, f"feature imports Views: `{include}`"))
        target = role_for_include(include)
        if target and target in FORBIDDEN_TARGET_ROLES.get(role, set()):
            findings.append(Finding(unit.path, line, IMPORT_DIRECTION.id, IMPORT_DIRECTION.severity, f"{role} must not import {target}: `{include}`"))
        if Path(include).name.endswith("Store.h") and rel not in STORE_BOUNDARY_RELS:
            findings.append(Finding(unit.path, line, IMPORT_STORE.id, IMPORT_STORE.severity, IMPORT_STORE.summary))
    return findings


def check_store_access(unit: SourceUnit, role: str) -> list[Finding]:
    findings: list[Finding] = []
    rel = _rel_key(unit.path)
    for match in STORE_GET_RE.finditer(unit.code):
        if rel not in STORE_BOUNDARY_RELS:
            findings.append(Finding(unit.path, line_number(unit.code, match.start()), STORE_GET.id, STORE_GET.severity, STORE_GET.summary))
    if role not in RAW_DISPATCH_ALLOW_ROLES:
        for match in DISPATCH_RE.finditer(unit.code):
            findings.append(Finding(unit.path, line_number(unit.code, match.start()), STORE_DISPATCH.id, STORE_DISPATCH.severity, f"{role} files must not dispatch"))
    if role not in RAW_GET_STATE_ALLOW_ROLES and rel not in GET_STATE_ALLOW_RELS:
        for match in GET_STATE_RE.finditer(unit.code):
            findings.append(Finding(unit.path, line_number(unit.code, match.start()), STORE_GET_STATE.id, STORE_GET_STATE.severity, f"{role} files must not call getState"))
    return findings


def check_pure_roles(unit: SourceUnit, role: str) -> list[Finding]:
    if role not in PURE_ROLES:
        return []
    findings: list[Finding] = []
    for pattern, what in SIDE_EFFECT_PATTERNS:
        for match in pattern.finditer(unit.role_code):
            findings.append(Finding(unit.path, line_number(unit.role_code, match.start()), PURE_ROLE.id, PURE_ROLE.severity, f"{role} {what}"))
    return findings


def check_rtk_query(unit: SourceUnit) -> list[Finding]:
    if not RTK_QUERY_PATTERN.search(unit.code) or RTK_QUERY_LIFECYCLE.search(unit.code):
        return []
    match = RTK_QUERY_PATTERN.search(unit.code)
    return [Finding(unit.path, line_number(unit.code, match.start()), QUERY_LIFECYCLE.id, QUERY_LIFECYCLE.severity, QUERY_LIFECYCLE.summary)]


def check_suppressions(unit: SourceUnit) -> list[Finding]:
    return [
        Finding(
            unit.path,
            line_number(unit.raw, match.start()),
            SUPPRESSION_COMMENT.id,
            SUPPRESSION_COMMENT.severity,
            SUPPRESSION_COMMENT.summary,
        )
        for match in SUPPRESSION_RE.finditer(unit.raw)
    ]


def check_unit(unit: SourceUnit, plugins: dict[str, object]) -> list[Finding]:
    findings = check_suppressions(unit)
    role = unit.declared_role
    if role is None:
        return findings + [Finding(unit.path, 1, STRUCT_LEAF.id, STRUCT_LEAF.severity, leaf_guidance(unit.stem))]

    findings += fingerprint_findings(unit)
    plugin = plugins.get(role)

    if role == "view":
        findings += plugin.check(unit) if plugin else []
        return findings

    findings += check_imports(unit, role)
    findings += check_store_access(unit, role)
    findings += check_pure_roles(unit, role)
    findings += check_rtk_query(unit)
    findings += plugin.check(unit) if plugin else []
    return findings


# --- Global checks ---------------------------------------------------------

SOURCE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
NON_PROGRAM_STORE_DIRS = {"Core", "Features", "Views", "Tests", "Intermediate"}


def discover_store_paths(project_root: Path) -> list[Path]:
    source_root = project_root / "Source"
    if not source_root.is_dir():
        return []
    return sorted(
        path
        for path in source_root.rglob("*")
        if path.is_file()
        and path.suffix.lower() in SOURCE_SUFFIXES
        and path.stem.lower().endswith("store")
        and not NON_PROGRAM_STORE_DIRS.intersection(
            path.relative_to(source_root).parts[:-1]
        )
    )


def discover_program_entry_roots(project_root: Path) -> list[Path]:
    roots = sorted({path.parent for path in discover_store_paths(project_root)})
    source_root = project_root / "Source"
    return roots or ([source_root] if source_root.is_dir() else [])


def is_module_entry(path: Path) -> bool:
    sibling_sources = [
        candidate
        for candidate in path.parent.glob(f"{path.stem}.*")
        if candidate.is_file() and candidate.suffix.lower() in SOURCE_SUFFIXES
    ]
    code = "\n".join(
        candidate.read_text(encoding="utf-8", errors="replace")
        for candidate in sibling_sources
    )
    return bool(re.search(r"\bIMPLEMENT_(?:PRIMARY_GAME_)?MODULE\s*\(", code))


def check_root_role_boundaries(project_root: Path) -> list[Finding]:
    findings: list[Finding] = []
    for root in discover_program_entry_roots(project_root):
        for path in sorted(root.iterdir()):
            if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
                continue
            if path.stem.lower().endswith("store"):
                continue
            if is_module_entry(path):
                continue
            declared_role = role_for_stem_suffix(path.stem)
            detail = (
                f"declares {declared_role} role"
                if declared_role is not None
                else "is not a store or module entry"
            )
            findings.append(
                Finding(
                    path,
                    1,
                    ROOT_ROLE.id,
                    ROOT_ROLE.severity,
                    f"root source file {detail}; move behavior under Features or presentation under Views",
                )
            )
    return findings

def check_store_boundary(project_root: Path) -> list[Finding]:
    findings: list[Finding] = []
    store_paths = discover_store_paths(project_root)
    if not store_paths:
        source_root = project_root / "Source"
        return [
            Finding(
                source_root if source_root.exists() else project_root,
                1,
                STORE_CONFIGURE.id,
                STORE_CONFIGURE.severity,
                "stateful program has no root store boundary",
            )
        ]
    store_path_set = {path.resolve() for path in store_paths}
    source_root = project_root / "Source"
    if source_root.is_dir():
        for path in sorted(source_root.rglob("*")):
            if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
                continue
            if {"Binaries", "Intermediate"}.intersection(path.parts):
                continue
            text = path.read_text(encoding="utf-8", errors="replace")
            if path.resolve() in store_path_set:
                continue
            for match in APPLICATION_CONFIGURE_STORE_RE.finditer(text):
                findings.append(
                    Finding(
                        path,
                        line_number(text, match.start()),
                        STORE_CONFIGURE_OUTSIDE_ROOT.id,
                        STORE_CONFIGURE_OUTSIDE_ROOT.severity,
                        STORE_CONFIGURE_OUTSIDE_ROOT.summary,
                    )
                )
    for path in store_paths:
        text = path.read_text(encoding="utf-8", errors="replace")
        for match in STORE_LEGACY_RE.finditer(text):
            findings.append(Finding(path, line_number(text, match.start()), STORE_LEGACY.id, STORE_LEGACY.severity, STORE_LEGACY.summary))
        for match in STORE_ARRAY_MW_RE.finditer(text):
            findings.append(Finding(path, line_number(text, match.start()), STORE_ARRAY_MW.id, STORE_ARRAY_MW.severity, STORE_ARRAY_MW.summary))
    logical_stores = sorted({path.stem.lower() for path in store_paths})
    if len(logical_stores) > 1:
        findings.append(
            Finding(
                store_paths[0],
                1,
                STORE_MULTIPLE_ROOTS.id,
                STORE_MULTIPLE_ROOTS.severity,
                "competing root stores: " + ", ".join(logical_stores),
            )
        )
    combined = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in store_paths
    )
    if not CONFIGURE_STORE_RE.search(combined):
        findings.append(Finding(store_paths[0], 1, STORE_CONFIGURE.id, STORE_CONFIGURE.severity, STORE_CONFIGURE.summary))
    return findings


def check_multiple_api_roots(units: list[SourceUnit]) -> list[Finding]:
    roots = [unit for unit in units if CREATE_API.search(unit.code)]
    if len(roots) < 2:
        return []
    return [
        Finding(unit.path, line_number(unit.code, CREATE_API.search(unit.code).start()), QUERY_MULTI_ROOT.id, QUERY_MULTI_ROOT.severity, QUERY_MULTI_ROOT.summary)
        for unit in roots
    ]


def check_import_graph(units: list[SourceUnit]) -> list[Finding]:
    cycles = find_import_cycles(units)
    return [
        Finding(units[0].path if units else Path("Source/Features"), 1, GRAPH_CYCLE.id, GRAPH_CYCLE.severity, "import cycle: " + " -> ".join(cycle))
        for cycle in cycles
    ]


# --- Runner ----------------------------------------------------------------

def collect_findings(project_root: Path, scan_roots: list[Path]) -> list[Finding]:
    plugins = discover_role_plugins()

    units = [build_unit(path, project_root) for path in iter_source_files(scan_roots)]

    findings: list[Finding] = []
    for unit in units:
        findings += check_unit(unit, plugins)

    findings += check_feature_leaf_names(units)
    findings += check_root_role_boundaries(project_root)
    findings += check_store_boundary(project_root)
    findings += check_multiple_api_roots(units)
    findings += check_import_graph(units)
    return findings


def run(guard_name: str = "RTK boundary guard", fmt: str = "text") -> int:
    project_root = PROJECT_ROOT
    findings = collect_findings(project_root, SCAN_ROOTS)

    if fmt == "json":
        print(format_json(findings, project_root))
        if findings:
            print(format_summary(findings, guard_name), file=sys.stderr)
    elif fmt == "sarif":
        print(format_sarif(findings, project_root, guard_name))
        if findings:
            print(format_summary(findings, guard_name), file=sys.stderr)
    elif findings:
        print(format_text(findings, project_root, guard_name))
    else:
        print(f"{guard_name} passed.")

    return 1 if findings else 0


def run_discovered(fmt: str = "text") -> int:
    target_results: list[tuple[str, Path, list[Finding]]] = []
    for label, root, scan_roots in discovered_target_scan_roots(PROJECT_ROOT):
        target_results.append((label, root, collect_findings(root, scan_roots)))

    findings = [finding for _, _, target_findings in target_results for finding in target_findings]
    guard = "RTK boundary guard"
    if fmt == "json":
        print(format_json(findings, PROJECT_ROOT))
        if findings:
            print(format_summary(findings, guard), file=sys.stderr)
    elif fmt == "sarif":
        print(format_sarif(findings, PROJECT_ROOT, guard))
        if findings:
            print(format_summary(findings, guard), file=sys.stderr)
    elif findings:
        for label, root, target_findings in target_results:
            if not target_findings:
                print(f"{guard} passed for {label} ({root}).")
                continue
            print(format_text(target_findings, root, f"{guard} [{label}]"))
    else:
        labels = ", ".join(label for label, _, _ in target_results)
        print(f"{guard} passed for {labels}.")

    return 1 if findings else 0


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--format", choices=("text", "json", "sarif"), default="text")
    parser.add_argument("--explain", nargs="?", const="", metavar="RULE-ID", help="Print a rule (or the whole catalog) and exit.")
    parser.add_argument("--project-root", type=Path, help="Project root to scan. Defaults to FORBOCAI_REDUX_PROJECT_ROOT or the SDK root.")
    parser.add_argument(
        "--scan-root",
        action="append",
        type=Path,
        default=[],
        help="Source root to scan, relative to project root unless absolute. May be supplied multiple times.",
    )
    return parser.parse_args()


def main() -> int:
    global PROJECT_ROOT, SCAN_ROOTS
    args = parse_args()
    if args.project_root:
        PROJECT_ROOT = args.project_root.resolve()
    if args.scan_root:
        SCAN_ROOTS = [
            scan_root if scan_root.is_absolute() else PROJECT_ROOT / scan_root
            for scan_root in args.scan_root
        ]
    else:
        SCAN_ROOTS = configured_scan_roots(PROJECT_ROOT)
    discover_role_plugins()  # populate RULES for --explain
    if args.explain is not None:
        print(explain(args.explain or None))
        return 0
    if not args.project_root and not args.scan_root and not os.environ.get("FORBOCAI_REDUX_SCAN_ROOTS"):
        return run_discovered(fmt=args.format)
    return run(fmt=args.format)


if __name__ == "__main__":
    sys.exit(main())
