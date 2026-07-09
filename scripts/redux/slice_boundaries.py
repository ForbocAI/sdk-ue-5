#!/usr/bin/env python3
"""Slice boundary rules.

Slice owns createSlice assembly, initial state wiring, and pure reducer
transitions. Reducer helpers may live here, but side effects do not.
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


ROLE = "slice"

SLICE_EXPORT = re.compile(r"\brtk::Slice\s*<|\bGetSlice\s*\(")
CREATE_SLICE = re.compile(r"\brtk::createSlice\s*<")
# A switch on action.type is the hand-written anti-pattern. rtk::createReducer's
# builder form is modern RTK (root-reducer composition) and is NOT flagged.
HAND_WRITTEN_REDUCER = re.compile(
    r"\bswitch\s*\([^)]*(?:Action|action)\s*\.\s*(?:type|Type)[^)]*\)"
)
BLIND_PAYLOAD_REPLACE = re.compile(r"\breturn\s+(?:Action|action)\.PayloadValue\s*;")
VIEW_MODEL_STATE = re.compile(r"(?<![\w.>])(?:ViewModel|ViewDefaults)\b")
RTK1_EXTRA_REDUCERS = re.compile(
    r"\bextraReducers\s*[:=]\s*\{"
    r"|\[\s*[A-Za-z_][\w:]*\.(?:fulfilled|pending|rejected)(?:\.type)?\s*\]"
)

SLICE_WITHOUT_FACTORY = register(
    Rule(
        id="RTK-SLICE-001",
        severity=Severity.MEDIUM,
        summary="Slice exposes GetSlice/rtk::Slice without assembling it via rtk::createSlice",
        guidance="Assemble the slice with rtk::createSlice in this file or its sibling Slice.cpp; reducer helpers may live here without pretending to be store slices.",
        skill="build-modern-redux-apps-modern-redux: createSlice is the default",
        roles=frozenset({ROLE}),
    )
)

HAND_WRITTEN = register(
    Rule(
        id="RTK-SLICE-002",
        severity=Severity.HIGH,
        summary="switch-on-action.type transition instead of createSlice assembly",
        guidance="Move the transition into createSlice's reducers; a switch on action.type is the hand-written escape hatch, not the default.",
        skill="model-redux-state-build-slices-and-selectors: HIGH hand-written switch reducers as the default",
        roles=frozenset({ROLE}),
    )
)

BLIND_PAYLOAD = register(
    Rule(
        id="RTK-SLICE-003",
        severity=Severity.HIGH,
        summary="reducer blindly replaces state from the payload",
        guidance="Set the fields the reducer owns; do not treat the payload as a trusted whole-state patch.",
        skill="model-redux-state-design-state-ownership: HIGH blindly spreading payloads into state",
        roles=frozenset({ROLE}),
    )
)

DERIVED_SLICE_STATE = register(
    Rule(
        id="RTK-SLICE-004",
        severity=Severity.MEDIUM,
        summary="view model / derived defaults stored as slice shape",
        guidance="Derive view models in Selectors; keep raw authoritative state in the slice and derive the view shape.",
        skill="build-modern-redux-apps-redux-dataflow: MEDIUM storing derived values in state",
        roles=frozenset({ROLE}),
    )
)

RTK1_OBJECT_SYNTAX = register(
    Rule(
        id="RTK-SLICE-005",
        severity=Severity.HIGH,
        summary="RTK 1.x object-syntax extraReducers / lifecycle map keys",
        guidance="Use the builder callback form (builder.addCase(thunk.fulfilled, ...)); RTK 2 removed the object/map form.",
        skill="model-redux-state-build-slices-and-selectors: HIGH RTK 1.x object syntax for extraReducers",
        roles=frozenset({ROLE}),
    )
)


def check(unit: SourceUnit) -> list[Finding]:
    findings = presence_finding(unit, SLICE_EXPORT, CREATE_SLICE, SLICE_WITHOUT_FACTORY)
    findings += findings_for(unit, unit.role_code, HAND_WRITTEN, HAND_WRITTEN_REDUCER)
    findings += findings_for(unit, unit.role_code, BLIND_PAYLOAD, BLIND_PAYLOAD_REPLACE)
    findings += findings_for(unit, unit.code, DERIVED_SLICE_STATE, VIEW_MODEL_STATE)
    findings += findings_for(unit, unit.role_code, RTK1_OBJECT_SYNTAX, RTK1_EXTRA_REDUCERS)
    return findings
