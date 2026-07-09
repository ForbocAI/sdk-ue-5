#!/usr/bin/env python3
"""Selectors boundary rules.

Selectors expose derived reads from state. They stay pure, avoid
dispatch/effects, and keep Views away from store internals.
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


ROLE = "selectors"

SELECTOR_EXPORT = re.compile(r"\bSelector\s*<|\bselectAll\s*\(|\bselectById\s*\(")
SELECTOR_FACTORY = re.compile(r"\brtk::createSelector\s*<|\bgetSelectors\s*\(")
WHOLE_STATE_RETURN = re.compile(r"\breturn\s+State\s*;")
# A memoized selector result-function that constructs a fresh container inline
# defeats memoization: every call returns a new reference.
UNSTABLE_RESULT = re.compile(
    r"\brtk::createSelector\s*<[\s\S]{0,400}?\breturn\s+(?:TArray|TMap|TSet)\s*<[^>]*>\s*\{"
)

SELECTOR_WITHOUT_FACTORY = register(
    Rule(
        id="RTK-SELECTOR-001",
        severity=Severity.LOW,
        summary="Selectors expose memoized/adapter selectors without createSelector or getSelectors",
        guidance="Memoized/adapter selectors use createSelector or adapter getSelectors; simple pure selectors stay direct reads.",
        skill="model-redux-state-build-slices-and-selectors: define selectors in the slice/selectors",
        roles=frozenset({ROLE}),
    )
)

WHOLE_STATE = register(
    Rule(
        id="RTK-SELECTOR-002",
        severity=Severity.HIGH,
        summary="selector returns the whole slice/state instead of a narrow derived read",
        guidance="Return the narrow value the view renders; returning whole state widens subscriptions and rerenders.",
        skill="evolve-and-diagnose-redux-apps-debug-redux-toolkit-apps: HIGH selecting broad state",
        roles=frozenset({ROLE}),
    )
)

UNSTABLE = register(
    Rule(
        id="RTK-SELECTOR-003",
        severity=Severity.MEDIUM,
        summary="memoized selector builds a fresh container each call, defeating memoization",
        guidance="Return the derived data by reference; constructing a new TArray/TMap/TSet inside createSelector produces a new reference every call.",
        skill="evolve-and-diagnose-redux-apps-debug-redux-toolkit-apps: MEDIUM returning unstable objects from selection logic",
        roles=frozenset({ROLE}),
    )
)


def check(unit: SourceUnit) -> list[Finding]:
    findings = presence_finding(unit, SELECTOR_EXPORT, SELECTOR_FACTORY, SELECTOR_WITHOUT_FACTORY)
    findings += findings_for(unit, unit.code, WHOLE_STATE, WHOLE_STATE_RETURN)
    findings += findings_for(unit, unit.role_code, UNSTABLE, UNSTABLE_RESULT)
    return findings
