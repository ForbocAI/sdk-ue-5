#!/usr/bin/env python3
"""Views boundary rules.

Views are the presentation boundary, not an RTK feature role. They render, bind
UE lifecycle callbacks, dispatch user/engine events through feature Actions, and
read already-derived data through feature Selectors. They may not import Types,
Slice, Thunks, Listeners, or Adapters, touch the store directly, own async or
reactive workflows, or subscribe to whole-state objects.

This module is a role plugin (``check``) for the boundaries runner.
"""

from __future__ import annotations

import re

from features_boundaries import (
    IMPORT_RE,
    Finding,
    Rule,
    Severity,
    SourceUnit,
    findings_for,
    line_number,
    register,
    role_for_include,
    role_for_path,
)


ROLE = "view"

ALLOWED_FEATURE_ROLES = {"actions", "selectors"}
FEATURE_INCLUDE = re.compile(r"^Features/")
STORE_INCLUDE = "Store.h"
STORE_ACCESS = re.compile(r"\bStore::GetStore\s*\(|\.dispatch\s*\(|\.getState\s*\(")
DIRECT_ASYNC_WORKFLOW = re.compile(
    r"\brtk::createAsyncThunk\s*<|\bcreateListenerMiddleware\s*<|\bStartListening\b"
)
RTK_QUERY_CACHE_PATCH = re.compile(r"\bupdateQueryData\s*\(|\bselectFromResult\s*\(")
WHOLE_STATE_SELECTOR = re.compile(r"\bSelect[A-Za-z0-9_]*State\s*\(")
BOUNDARY_IO = re.compile(
    r"\bIFileManager::|\bFPaths::|\bFParse::|\bFCommandLine::|\bFHttpModule\b|\bIHttpRequest\b"
)

VIEW_NAME = register(
    Rule(
        id="RTK-VIEW-001",
        severity=Severity.HIGH,
        summary="view source leaf must be folder-qualified `View`",
        guidance="Use the nearest unambiguous folder qualifier plus View, such as Source/Views/Player/Controller/ControllerView.cpp.",
        skill="build-modern-redux-apps-modern-redux: keep feature logic colocated behind stable leaves",
        roles=frozenset({ROLE}),
    )
)

VIEW_INCLUDE = register(
    Rule(
        id="RTK-VIEW-002",
        severity=Severity.HIGH,
        summary="view imports a feature role other than Actions/Selectors",
        guidance="Views may include only feature Actions or Selectors facades; move derivation to Selectors and effects to Thunks/Listeners.",
        skill="build-modern-redux-apps-modern-redux: keep React components on hooks/facades",
        roles=frozenset({ROLE}),
    )
)

VIEW_STORE_IMPORT = register(
    Rule(
        id="RTK-VIEW-003",
        severity=Severity.HIGH,
        summary="view imports Store.h",
        guidance="Only the single store boundary imports Store.h; use feature Actions and Selectors.",
        skill="build-modern-redux-apps-modern-redux: HIGH importing the store in components",
        roles=frozenset({ROLE}),
    )
)

VIEW_STORE_ACCESS = register(
    Rule(
        id="RTK-VIEW-004",
        severity=Severity.HIGH,
        summary="view accesses the store directly (dispatch/getState/GetStore)",
        guidance="Dispatch through Actions and read through Selectors; the view never touches the store.",
        skill="build-modern-redux-apps-modern-redux: HIGH importing the store in components",
        roles=frozenset({ROLE}),
    )
)

VIEW_ASYNC = register(
    Rule(
        id="RTK-VIEW-005",
        severity=Severity.HIGH,
        summary="view creates an RTK async/listener workflow",
        guidance="Move imperative work to Thunks and reactive work to Listeners; the view consumes the result.",
        skill="orchestrate-side-effects-handle-side-effects: keep side effects out of UI",
        roles=frozenset({ROLE}),
    )
)

VIEW_CACHE_PATCH = register(
    Rule(
        id="RTK-VIEW-006",
        severity=Severity.MEDIUM,
        summary="view patches RTK Query cache",
        guidance="RTK Query cache patches live in endpoint lifecycles, not presentation code.",
        skill="manage-server-data-adopt-rtk-query: HIGH patching cache from components",
        roles=frozenset({ROLE}),
    )
)

VIEW_WHOLE_STATE = register(
    Rule(
        id="RTK-VIEW-007",
        severity=Severity.HIGH,
        summary="view subscribes to a whole-state selector",
        guidance="Subscribe to narrow selector results, not whole slice/state objects, to keep rerenders tight.",
        skill="evolve-and-diagnose-redux-apps-debug-redux-toolkit-apps: HIGH selecting broad state",
        roles=frozenset({ROLE}),
    )
)

VIEW_IO = register(
    Rule(
        id="RTK-VIEW-008",
        severity=Severity.MEDIUM,
        summary="view performs file/command-line/network IO",
        guidance="File, command-line, and network IO live behind Adapters/Thunks/Listeners before a View consumes the result.",
        skill="orchestrate-side-effects-handle-side-effects: keep side effects out of UI",
        roles=frozenset({ROLE}),
    )
)


def _include_findings(unit: SourceUnit) -> list[Finding]:
    findings: list[Finding] = []
    for match in IMPORT_RE.finditer(unit.raw):
        include = match.group(1)
        line = line_number(unit.raw, match.start())
        if include == STORE_INCLUDE:
            findings.append(Finding(unit.path, line, VIEW_STORE_IMPORT.id, VIEW_STORE_IMPORT.severity, VIEW_STORE_IMPORT.summary))
        elif FEATURE_INCLUDE.match(include) and role_for_include(include) not in ALLOWED_FEATURE_ROLES:
            findings.append(
                Finding(unit.path, line, VIEW_INCLUDE.id, VIEW_INCLUDE.severity, f"disallowed feature include `{include}`")
            )
    return findings


def check(unit: SourceUnit) -> list[Finding]:
    findings: list[Finding] = []
    if role_for_path(unit.path) != ROLE:
        findings.append(Finding(unit.path, 1, VIEW_NAME.id, VIEW_NAME.severity, VIEW_NAME.summary))
    findings += _include_findings(unit)
    findings += findings_for(unit, unit.code, VIEW_STORE_ACCESS, STORE_ACCESS)
    findings += findings_for(unit, unit.code, VIEW_ASYNC, DIRECT_ASYNC_WORKFLOW)
    findings += findings_for(unit, unit.code, VIEW_CACHE_PATCH, RTK_QUERY_CACHE_PATCH)
    findings += findings_for(unit, unit.code, VIEW_WHOLE_STATE, WHOLE_STATE_SELECTOR)
    findings += findings_for(unit, unit.code, VIEW_IO, BOUNDARY_IO)
    return findings
