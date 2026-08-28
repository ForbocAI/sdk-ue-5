from __future__ import annotations

from collections.abc import Iterable, Sequence
from typing import Any


Capability = dict[str, Any]
Sources = dict[str, dict[str, Any]]


def endpoint_evidence(available: bool, evidence: Iterable[str]) -> dict[str, Any]:
    return {
        "available": available,
        "evidence": sorted(set(evidence)),
    }


def mismatch(message: str, remediation: str) -> dict[str, str]:
    return {"message": message, "remediation": remediation}


def capability(
    capability_id: str,
    category: str,
    classification: str,
    result: str,
    typescript: dict[str, Any],
    unreal: dict[str, Any],
    mismatch_evidence: dict[str, str] | None,
) -> Capability:
    return {
        "id": capability_id,
        "category": category,
        "classification": classification,
        "result": result,
        "typescript": typescript,
        "unreal": unreal,
        "mismatch": mismatch_evidence,
    }


def summarize(capabilities: Sequence[Capability]) -> dict[str, Any]:
    results = [str(item["result"]) for item in capabilities]
    mismatch_count = results.count("mismatch")
    return {
        "status": "failed" if mismatch_count else "passed",
        "capabilityCount": len(capabilities),
        "sameCount": results.count("same"),
        "expectedMechanicCount": results.count("expected-mechanic"),
        "mismatchCount": mismatch_count,
    }


def combine_summaries(summaries: Sequence[dict[str, Any]]) -> dict[str, Any]:
    keys = (
        "capabilityCount",
        "sameCount",
        "expectedMechanicCount",
        "mismatchCount",
    )
    totals = {key: sum(int(summary[key]) for summary in summaries) for key in keys}
    return {
        "status": "failed" if totals["mismatchCount"] else "passed",
        **totals,
    }
