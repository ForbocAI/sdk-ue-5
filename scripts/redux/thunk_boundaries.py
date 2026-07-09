#!/usr/bin/env python3
"""Thunks boundary rules.

Thunks own one imperative async workflow with dispatch/getState. Long-lived
reaction logic belongs in Listeners, and cache-owned request/response data
moves behind RTK Query.
"""

from __future__ import annotations

import re

from features_boundaries import (
    Finding,
    Rule,
    Severity,
    SourceUnit,
    findings_for,
    presence_finding,
    register,
)


ROLE = "thunks"

THUNK_EXPORT = re.compile(r"\brtk::ThunkAction\s*<|\bAsyncThunk\b|\bApi\.dispatch\b|\bApi\.getState\b")
THUNK_FACTORY = re.compile(r"\brtk::createAsyncThunk\s*<|\brtk::ThunkAction\s*<")
CREATE_ASYNC_THUNK = re.compile(r"\brtk::createAsyncThunk\s*<")
THUNK_CONDITION = re.compile(r"\b[Cc]ondition\b")
POLLING_GET_STATE = re.compile(r"\b(?:while|for)\s*\([^)]*\)[\s\S]{0,800}?\b(?:Api\.getState|\.getState)\b")
SERVER_CACHE_WORKFLOW = re.compile(r"\b(?:FHttpModule|IHttpRequest|IHttpResponse|fetch\s*\(|Fetch[A-Z]|Http[A-Z])")

THUNK_WITHOUT_FACTORY = register(
    Rule(
        id="RTK-THUNK-001",
        severity=Severity.MEDIUM,
        summary="Thunks expose an async workflow without createAsyncThunk / typed ThunkAction",
        guidance="Use rtk::createAsyncThunk or a typed ThunkAction and keep imperative effects out of Slice/View code.",
        skill="orchestrate-side-effects-handle-side-effects: createAsyncThunk for imperative workflows",
        roles=frozenset({ROLE}),
    )
)

THUNK_POLLING = register(
    Rule(
        id="RTK-THUNK-002",
        severity=Severity.HIGH,
        summary="thunk polls future state in a loop over getState",
        guidance="Move long-lived reactive waiting into a Listener predicate/effect; thunks are one imperative pass, not a poll loop.",
        skill="orchestrate-side-effects-handle-side-effects: HIGH using thunks to watch future state changes",
        roles=frozenset({ROLE}),
    )
)

THUNK_SERVER_CACHE = register(
    Rule(
        id="RTK-THUNK-003",
        severity=Severity.MEDIUM,
        summary="hand-written request/response server-cache workflow in a thunk",
        guidance="Move request/response server cache behind an RTK Query createApi/injectEndpoints boundary instead of a hand-written thunk cache.",
        skill="orchestrate-side-effects-handle-side-effects: use RTK Query for server cache by default",
        roles=frozenset({ROLE}),
    )
)

THUNK_NO_CONDITION = register(
    Rule(
        id="RTK-THUNK-004",
        severity=Severity.MEDIUM,
        summary="createAsyncThunk has no condition guard against duplicate/stale dispatches",
        guidance="Add a condition(arg, {getState}) guard so StrictMode double-invokes and stale requests are skipped at the thunk level.",
        skill="evolve-and-diagnose-redux-apps-debug-redux-toolkit-apps: HIGH dispatching fetch thunks without a thunk-level guard",
        roles=frozenset({ROLE}),
    )
)


def check(unit: SourceUnit) -> list[Finding]:
    findings = presence_finding(unit, THUNK_EXPORT, THUNK_FACTORY, THUNK_WITHOUT_FACTORY)
    findings += findings_for(unit, unit.role_code, THUNK_POLLING, POLLING_GET_STATE)
    findings += findings_for(unit, unit.role_code, THUNK_SERVER_CACHE, SERVER_CACHE_WORKFLOW)
    if CREATE_ASYNC_THUNK.search(unit.role_code) and not THUNK_CONDITION.search(unit.role_code):
        findings.append(Finding(unit.path, 1, THUNK_NO_CONDITION.id, THUNK_NO_CONDITION.severity, THUNK_NO_CONDITION.summary))
    return findings
