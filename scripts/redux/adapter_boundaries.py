#!/usr/bin/env python3
"""Adapters boundary rules.

Adapters translate authored JSON, UE objects, external data, and entity-adapter
state. They do not dispatch, listen, or read the store.
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


ROLE = "adapters"

ENTITY_ADAPTER_EXPORT = re.compile(r"\bEntityAdapter\s*<|\bgetSelectors\s*\(")
CREATE_ENTITY_ADAPTER = re.compile(r"\brtk::createEntityAdapter\s*<")
# createEntityAdapter over a struct keyed by something other than `id` needs an
# explicit selectId, mirroring the skill mistake in TS.
ADAPTER_WITHOUT_SELECT_ID = re.compile(
    r"\brtk::createEntityAdapter\s*<[^>]*>\s*\(\s*\)"
)

ENTITY_WITHOUT_FACTORY = register(
    Rule(
        id="RTK-ADAPTER-001",
        severity=Severity.MEDIUM,
        summary="Adapters expose entity adapters but do not build them with rtk::createEntityAdapter",
        guidance="Build exported entity adapters with rtk::createEntityAdapter; non-entity adapters stay plain translation helpers.",
        skill="model-redux-state-build-slices-and-selectors: entity adapters standardize normalized collections",
        roles=frozenset({ROLE}),
    )
)

ADAPTER_DEFAULT_ID = register(
    Rule(
        id="RTK-ADAPTER-002",
        severity=Severity.MEDIUM,
        summary="createEntityAdapter with no selectId assumes an `id` field",
        guidance="Collections keyed by another field must pass selectId; the adapter defaults to entity.id.",
        skill="model-redux-state-build-slices-and-selectors: HIGH assuming entity.id exists for every collection",
        roles=frozenset({ROLE}),
    )
)


def check(unit: SourceUnit) -> list[Finding]:
    findings = presence_finding(unit, ENTITY_ADAPTER_EXPORT, CREATE_ENTITY_ADAPTER, ENTITY_WITHOUT_FACTORY)
    findings += findings_for(unit, unit.role_code, ADAPTER_DEFAULT_ID, ADAPTER_WITHOUT_SELECT_ID)
    return findings
