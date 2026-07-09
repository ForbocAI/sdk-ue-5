#!/usr/bin/env python3
"""Listeners boundary rules.

Listeners own reactive workflows that respond to later actions or state
changes. They are the place for long-lived reaction logic, not reducers/views.
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


ROLE = "listeners"

LISTENER_EXPORT = re.compile(r"\bListenerMiddleware\b|\bstartListening\b|\bStartListening\b")
CREATE_LISTENER = re.compile(r"\bcreateListenerMiddleware\s*<|\bCreateRuntimeListenerMiddleware\b")
APPENDED_LISTENER = re.compile(r"\bcreateListenerMiddleware[\s\S]{0,300}?\bconcat\s*\(")

LISTENER_WITHOUT_FACTORY = register(
    Rule(
        id="RTK-LISTENER-001",
        severity=Severity.MEDIUM,
        summary="Listeners expose listener middleware but do not build it with createListenerMiddleware",
        guidance="Create exported listener middleware with createListenerMiddleware and keep reactive workflows out of reducers/views.",
        skill="orchestrate-side-effects-handle-side-effects: listeners fit reactive workflows",
        roles=frozenset({ROLE}),
    )
)

LISTENER_APPENDED = register(
    Rule(
        id="RTK-LISTENER-002",
        severity=Severity.HIGH,
        summary="listener middleware appended after (not prepended before) the default checks",
        guidance="prepend() the listener middleware; listener add/remove actions carry functions and must run before the serializability check.",
        skill="orchestrate-side-effects-handle-side-effects: HIGH appending listener middleware after the default checks",
        roles=frozenset({ROLE}),
    )
)


def check(unit: SourceUnit) -> list[Finding]:
    findings = presence_finding(unit, LISTENER_EXPORT, CREATE_LISTENER, LISTENER_WITHOUT_FACTORY)
    findings += findings_for(unit, unit.role_code, LISTENER_APPENDED, APPENDED_LISTENER)
    return findings
