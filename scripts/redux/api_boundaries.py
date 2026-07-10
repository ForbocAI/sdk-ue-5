#!/usr/bin/env python3
"""API boundary rules.

API files own RTK Query surfaces: createApi roots, injected endpoints, endpoint
tags, and lifecycle hooks. Feature thunks may initiate API work, but request
construction and cache lifecycle metadata live here.
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


ROLE = "api"

API_EXPORT = re.compile(r"\brtk::Api\s*<|\bApiEndpoint\s*<|\binjectEndpoints\s*\(")
API_FACTORY = re.compile(r"\bcreateApi\s*<|\binjectEndpoints\s*\(")
RAW_TRANSPORT = re.compile(r"\b(?:FHttpModule|IHttpRequest|IHttpResponse)\b")

API_WITHOUT_FACTORY = register(
    Rule(
        id="RTK-API-001",
        severity=Severity.MEDIUM,
        summary="Api file exposes RTK Query shapes without createApi/injectEndpoints",
        guidance="Build server-data surfaces with createApi or injectEndpoints; leave feature thunks as initiators only.",
        skill="manage-server-data-adopt-rtk-query: keep one API slice per base URL and extend it",
        roles=frozenset({ROLE}),
    )
)

RAW_HTTP = register(
    Rule(
        id="RTK-API-002",
        severity=Severity.HIGH,
        summary="Api file uses raw UE HTTP transport instead of RTK Query baseQuery",
        guidance="Use fetchBaseQuery and endpoint definitions inside Api files; raw transport belongs in the generic RTK Query baseQuery implementation.",
        skill="manage-server-data-adopt-rtk-query: RTK Query owns server-data request lifecycles",
        roles=frozenset({ROLE}),
    )
)


def check(unit: SourceUnit) -> list[Finding]:
    findings = presence_finding(unit, API_EXPORT, API_FACTORY, API_WITHOUT_FACTORY)
    findings += findings_for(unit, unit.code, RAW_HTTP, RAW_TRANSPORT)
    return findings
