#!/usr/bin/env python3
"""Actions boundary rules.

Actions expose event-style action creators/facades. They may dispatch through
the one store boundary, but they do not derive state or watch future actions.
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


ROLE = "actions"

ACTION_CREATOR_EXPORT = re.compile(r"\brtk::ActionCreator\s*<|\bTypePrefix\s*\(")
CREATE_ACTION = re.compile(r"\brtk::createAction\s*<")
SETTER_STYLE_ACTION = re.compile(r"\b(?:Set|Replace|Put|Assign)[A-Z][A-Za-z0-9_]*\s*\(")

CREATOR_WITHOUT_FACTORY = register(
    Rule(
        id="RTK-ACTION-001",
        severity=Severity.MEDIUM,
        summary="Actions expose action creators but do not build them with rtk::createAction",
        guidance="Build exported creators with rtk::createAction so actions stay event-style metadata; a bare facade is not an action creator.",
        skill="build-modern-redux-apps-redux-dataflow: dispatch events, not setters",
        roles=frozenset({ROLE}),
    )
)

SETTER_STYLE = register(
    Rule(
        id="RTK-ACTION-002",
        severity=Severity.HIGH,
        summary="setter/replacer-style action name instead of an event name",
        guidance="Name actions for what happened (postPublished, profileSaved), not Set*/Replace*; dispatch the event and let Slice own the transition.",
        skill="build-modern-redux-apps-redux-dataflow: HIGH using setter-style actions instead of event-style actions",
        roles=frozenset({ROLE}),
    )
)


def check(unit: SourceUnit) -> list[Finding]:
    findings = presence_finding(unit, ACTION_CREATOR_EXPORT, CREATE_ACTION, CREATOR_WITHOUT_FACTORY)
    findings += findings_for(unit, unit.code, SETTER_STYLE, SETTER_STYLE_ACTION)
    return findings
