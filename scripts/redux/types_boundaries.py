#!/usr/bin/env python3
"""Types boundary rules.

Types stay inert: state, payload, entity, and dispatch declarations only. They
do not own side effects, store access, or non-serializable behavior, and they
carry authoritative domain state -- not UI names, form drafts, or derived reads.
"""

from __future__ import annotations

import re

from features_boundaries import (
    Finding,
    Rule,
    Severity,
    SourceUnit,
    findings_for,
    register,
)


ROLE = "types"

NON_SERIALIZABLE_STATE = re.compile(
    r"\b(?:TFunction|TSharedPtr|TUniquePtr|TWeakObjectPtr|TObjectPtr|std::function)\s*<"
    r"|\b(?:U[A-Za-z0-9_]+|A[A-Za-z0-9_]+|UWorld)\s*\*"
)
# A member declaration, not a reference: the name is not preceded by a `.`/`->`
# member access, so `Left.LastPath` in an operator== body does not match.
SCRATCHPAD_STATE = re.compile(r"(?<![\w.>])Last[A-Z][A-Za-z0-9_]*\b")
VIEW_MODEL_STATE = re.compile(r"(?<![\w.>])(?:ViewModel|ViewDefaults)\b")
COMPONENT_NAMED_STATE = re.compile(
    r"\bstruct\s+F[A-Za-z0-9_]*(?:Screen|Widget|Panel|Modal|Dialog|Menu|Button)"
    r"(?:State|Model|Defaults|Data)?\b"
)
FORM_DRAFT_STATE = re.compile(
    r"\bstruct\s+F[A-Za-z0-9_]*(?:Draft|Editing|FieldInput)[A-Za-z0-9_]*\b"
    r"|(?<![\w.>])(?:Editing|Draft)[A-Za-z0-9_]*\s+[A-Za-z0-9_]+\s*(?:=|;)"
)
ROUTER_STATE = re.compile(
    r"(?<![\w.>])[A-Za-z0-9_]*(?:SearchParams|QueryParams|RouteParams|UrlState)\b"
)

NON_SERIALIZABLE = register(
    Rule(
        id="RTK-TYPES-001",
        severity=Severity.HIGH,
        summary="persistent Types hold non-serializable state (UE pointers, ownership, callbacks)",
        guidance=(
            "Keep Types serializable/inert; UE object pointers, shared ownership, "
            "and callbacks live at Adapter/Thunk/Listener/View boundaries. Never "
            "add suppressions; move the boundary shape to the right role."
        ),
        skill="evolve-and-diagnose-redux-apps-debug-redux-toolkit-apps: HIGH ignoring serializable-state violations",
        roles=frozenset({ROLE}),
    )
)

SCRATCHPAD = register(
    Rule(
        id="RTK-TYPES-002",
        severity=Severity.MEDIUM,
        summary="Last*/scratchpad field in authoritative state",
        guidance="Keep authoritative state minimal; Last*/scratchpad reads are derived or transient and belong in Selectors.",
        skill="model-redux-state-design-state-ownership: keep authoritative state minimal",
        roles=frozenset({ROLE}),
    )
)

DERIVED_TYPES = register(
    Rule(
        id="RTK-TYPES-003",
        severity=Severity.MEDIUM,
        summary="view model / derived defaults stored as Types shape",
        guidance=(
            "Store raw state and derive view models in Selectors. Never add "
            "suppressions; if the value is a boundary payload, move it out of "
            "persistent state and into the owning Adapter/Thunk/Listener/View role."
        ),
        skill="build-modern-redux-apps-redux-dataflow: MEDIUM storing derived values in state",
        roles=frozenset({ROLE}),
    )
)

COMPONENT_NAMED = register(
    Rule(
        id="RTK-TYPES-004",
        severity=Severity.HIGH,
        summary="state struct named after a UI component",
        guidance="Name state for the data/domain concept, not the current component tree (auth, not FLoginScreenState).",
        skill="model-redux-state-design-state-ownership: HIGH naming state after components",
        roles=frozenset({ROLE}),
    )
)

FORM_STATE = register(
    Rule(
        id="RTK-TYPES-005",
        severity=Severity.MEDIUM,
        summary="form/editing draft state modeled in Redux Types",
        guidance="Keep editable form/draft state local until the user commits it; dispatch the committed value.",
        skill="model-redux-state-design-state-ownership: MEDIUM putting form editing state in Redux",
        roles=frozenset({ROLE}),
    )
)

ROUTER_OWNED = register(
    Rule(
        id="RTK-TYPES-006",
        severity=Severity.MEDIUM,
        summary="router/URL state duplicated into Redux Types",
        guidance="The router/URL already owns this; pass it into selectors at the edge instead of syncing it into state.",
        skill="model-redux-state-design-state-ownership: HIGH synchronizing router or URL state into Redux",
        roles=frozenset({ROLE}),
    )
)


# Types under a translation boundary (Adapters/Json) legitimately hold
# projector callbacks and parse handles; serializability applies to state types.
BOUNDARY_FOLDERS = {"Adapters", "Json"}


def _is_boundary_type(unit: SourceUnit) -> bool:
    return any(part in BOUNDARY_FOLDERS for part in unit.path.parts)


def check(unit: SourceUnit) -> list[Finding]:
    findings: list[Finding] = []
    if not _is_boundary_type(unit):
        findings += findings_for(unit, unit.code, NON_SERIALIZABLE, NON_SERIALIZABLE_STATE)
    findings += findings_for(unit, unit.code, SCRATCHPAD, SCRATCHPAD_STATE)
    findings += findings_for(unit, unit.code, DERIVED_TYPES, VIEW_MODEL_STATE)
    findings += findings_for(unit, unit.code, COMPONENT_NAMED, COMPONENT_NAMED_STATE)
    findings += findings_for(unit, unit.code, FORM_STATE, FORM_DRAFT_STATE)
    findings += findings_for(unit, unit.code, ROUTER_OWNED, ROUTER_STATE)
    return findings
