#!/usr/bin/env python3
"""ECS checks engine.

The ECS naming/domain guards build on this engine. It owns only ECS topology,
domain naming, authored-data shape, and the ECS vocabulary. It does not own FP
control-flow rules or RTK role/dataflow rules.

The engine provides ECS-specific machinery: one canonical tokenizer plus path
helpers shared by the naming guards, and the ECS vocabulary derived from the
SDK-owned Core/ecs.hpp so the naming doctrine tracks the ECS core instead of a
hand-maintained copy.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from functools import lru_cache
import os
from pathlib import Path
import re


class Severity(str, Enum):
    CRITICAL = "critical"
    HIGH = "high"
    MEDIUM = "medium"
    LOW = "low"


SEVERITY_ORDER = {
    Severity.CRITICAL: 0,
    Severity.HIGH: 1,
    Severity.MEDIUM: 2,
    Severity.LOW: 3,
}

SARIF_LEVEL = {
    Severity.CRITICAL: "error",
    Severity.HIGH: "error",
    Severity.MEDIUM: "error",
    Severity.LOW: "error",
}


@dataclass(frozen=True)
class Rule:
    """A guard rule: stable id, severity, and the skill/doctrine it enforces."""

    id: str
    severity: Severity
    summary: str
    guidance: str
    skill: str
    roles: frozenset[str] = frozenset()  # empty => global / any scope


RULES: dict[str, Rule] = {}


def register(rule: Rule) -> Rule:
    if rule.id in RULES and RULES[rule.id] is not rule:
        raise ValueError(f"duplicate rule id: {rule.id}")
    RULES[rule.id] = rule
    return rule


@dataclass(frozen=True)
class Finding:
    path: Path
    line: int
    rule_id: str
    severity: Severity
    message: str
    column: int = 1

    @property
    def guidance(self) -> str:
        rule = RULES.get(self.rule_id)
        return rule.guidance if rule else ""


def line_number(text: str, index: int) -> int:
    return text.count("\n", 0, index) + 1


def column_number(text: str, index: int) -> int:
    return index - (text.rfind("\n", 0, index) + 1) + 1


# --- Formatting ------------------------------------------------------------

def _display_path(finding: Finding, project_root: Path) -> str:
    try:
        return finding.path.relative_to(project_root).as_posix()
    except ValueError:
        return finding.path.as_posix()


def sort_findings(findings: list[Finding]) -> list[Finding]:
    return sorted(
        findings,
        key=lambda f: (SEVERITY_ORDER[f.severity], f.path.as_posix(), f.line, f.rule_id),
    )


def finding_counts(findings: list[Finding]) -> dict[str, dict[str, int]]:
    counts = {sev: 0 for sev in Severity}
    rules: dict[str, int] = {}
    for finding in findings:
        counts[finding.severity] += 1
        rules[finding.rule_id] = rules.get(finding.rule_id, 0) + 1
    return {
        "severity": {
            severity.value: counts[severity]
            for severity in Severity
            if counts[severity]
        },
        "rules": dict(sorted(rules.items())),
    }


def format_summary(findings: list[Finding], guard_name: str) -> str:
    if not findings:
        return f"{guard_name} passed."
    counts = finding_counts(findings)
    severity = ", ".join(
        f"{count} {name}"
        for name, count in counts["severity"].items()
    )
    rules = ", ".join(
        f"{rule_id}={count}"
        for rule_id, count in counts["rules"].items()
    )
    return f"{guard_name} failed: {len(findings)} issue(s) ({severity}). Rules: {rules}."


def format_text(findings: list[Finding], project_root: Path, guard_name: str) -> str:
    lines: list[str] = [format_summary(findings, guard_name)]
    for finding in sort_findings(findings):
        display = _display_path(finding, project_root)
        lines.append(
            f"{display}:{finding.line}:{finding.column}: "
            f"[{finding.severity.value.upper()} {finding.rule_id}] {finding.message}"
        )
        if finding.guidance:
            lines.append(f"    fix: {finding.guidance}")
    return "\n".join(lines)


def format_json(findings: list[Finding], project_root: Path) -> str:
    import json

    payload = [
        {
            "path": _display_path(finding, project_root),
            "line": finding.line,
            "column": finding.column,
            "ruleId": finding.rule_id,
            "severity": finding.severity.value,
            "message": finding.message,
            "guidance": finding.guidance,
            "skill": RULES[finding.rule_id].skill if finding.rule_id in RULES else "",
        }
        for finding in sort_findings(findings)
    ]
    return json.dumps(
        {"findings": payload, "count": len(payload), "summary": finding_counts(findings)},
        indent=2,
    )


def format_sarif(findings: list[Finding], project_root: Path, guard_name: str) -> str:
    import json

    rule_ids = sorted({finding.rule_id for finding in findings if finding.rule_id in RULES})
    sarif_rules = [
        {
            "id": rule_id,
            "name": rule_id,
            "shortDescription": {"text": RULES[rule_id].summary},
            "fullDescription": {"text": RULES[rule_id].guidance},
            "helpUri": "",
            "properties": {"skill": RULES[rule_id].skill, "severity": RULES[rule_id].severity.value},
            "defaultConfiguration": {"level": SARIF_LEVEL[RULES[rule_id].severity]},
        }
        for rule_id in rule_ids
    ]
    results = [
        {
            "ruleId": finding.rule_id,
            "level": SARIF_LEVEL[finding.severity],
            "message": {"text": finding.message},
            "locations": [
                {
                    "physicalLocation": {
                        "artifactLocation": {"uri": _display_path(finding, project_root)},
                        "region": {"startLine": finding.line, "startColumn": finding.column},
                    }
                }
            ],
        }
        for finding in sort_findings(findings)
    ]
    sarif = {
        "$schema": "https://json.schemastore.org/sarif-2.1.0.json",
        "version": "2.1.0",
        "runs": [
            {
                "tool": {"driver": {"name": guard_name, "rules": sarif_rules}},
                "results": results,
            }
        ],
    }
    return json.dumps(sarif, indent=2)


def explain(rule_id: str | None) -> str:
    if rule_id and rule_id in RULES:
        rule = RULES[rule_id]
        return (
            f"{rule.id}  [{rule.severity.value.upper()}]\n"
            f"  what : {rule.summary}\n"
            f"  fix  : {rule.guidance}\n"
            f"  skill: {rule.skill}\n"
            f"  roles: {', '.join(sorted(rule.roles)) or 'any'}"
        )
    lines = ["Rules (id  severity  summary):"]
    for rule in sorted(RULES.values(), key=lambda r: (SEVERITY_ORDER[r.severity], r.id)):
        lines.append(f"  {rule.id:<16} {rule.severity.value:<8} {rule.summary}")
    return "\n".join(lines)


# --- Paths -----------------------------------------------------------------

SCRIPT_ROOT = Path(__file__).resolve().parent
DEFAULT_PROJECT_ROOT = SCRIPT_ROOT.parent
PROJECT_ROOT = Path(
    os.environ.get("FORBOCAI_ECS_PROJECT_ROOT", DEFAULT_PROJECT_ROOT)
).resolve()
ECS_CORE_ROOT = Path(
    os.environ.get(
        "FORBOCAI_ECS_CORE_ROOT",
        PROJECT_ROOT / "Source" / "ForbocAI_SDK" / "Public" / "Core",
    )
).resolve()
ECS_HPP = ECS_CORE_ROOT / "ecs.hpp"

IGNORED_PARTS = {
    ".git",
    "Binaries",
    "DerivedDataCache",
    "Intermediate",
    "Saved",
    "__pycache__",
}


def has_ignored_part(path: Path) -> bool:
    return any(part in IGNORED_PARTS for part in path.parts)


def rel(path: Path, root: Path = PROJECT_ROOT) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def iter_files(roots: list[Path], suffixes: set[str] | None = None) -> list[Path]:
    """Every file under the roots, ignoring build dirs; optional suffix filter."""
    files: list[Path] = []
    for spec in roots:
        root = spec if spec.is_absolute() else PROJECT_ROOT / spec
        if root.is_file():
            files.append(root)
        elif root.is_dir():
            files.extend(child for child in root.rglob("*") if child.is_file())
    return sorted(
        path
        for path in files
        if not has_ignored_part(path) and (suffixes is None or path.suffix in suffixes)
    )


# --- Canonical tokenizer ---------------------------------------------------
# One tokenizer for both naming guards, so path/domain token comparisons stay
# consistent instead of drifting between two local copies.

def camel_tokens(value: str) -> list[str]:
    if not value:
        return []
    chunks = re.findall(r"[A-Z]+(?=[A-Z][a-z]|[0-9]|\b)|[A-Z]?[a-z]+|[0-9]+", value)
    return chunks or [value]


def normalize(value: str) -> str:
    """Lowercase, non-alphanumerics to underscores (no plural folding)."""
    return re.sub(r"[^a-z0-9]+", "_", value.lower()).strip("_")


def normalize_token(value: str) -> str:
    """Normalize one token and fold a trailing plural to its singular."""
    normalized = value.replace("-", "_").lower()
    if normalized.endswith("ies") and len(normalized) > 3:
        return normalized[:-3] + "y"
    if normalized.endswith("s") and not normalized.endswith("ss") and len(normalized) > 1:
        return normalized[:-1]
    return normalized


def normalized_tokens(tokens: list[str] | tuple[str, ...]) -> list[str]:
    return [normalize_token(token) for token in tokens]


def folder_tokens(part: str) -> list[str]:
    tokens: list[str] = []
    for piece in re.split(r"[_\-\s]+", part):
        tokens.extend(camel_tokens(piece))
    return tokens


def pluralize(atom: str) -> str:
    if atom.endswith("y"):
        return atom[:-1] + "ies"
    if atom.endswith("s"):
        return atom
    return atom + "s"


def snake(camel: str) -> str:
    return "_".join(token.lower() for token in camel_tokens(camel))


# --- ECS vocabulary derived from ecs.hpp -----------------------------------
# The naming doctrine's ECS section vocabulary is sourced from the ECS core, not
# a hand-maintained Python copy: add a domain kind or registry section in
# ecs.hpp and the naming guards track it.

@lru_cache(maxsize=1)
def _ecs_source() -> str:
    roots = [ECS_HPP]
    ecs_dir = ECS_CORE_ROOT / "Ecs"
    if ecs_dir.exists():
        roots.extend(sorted(ecs_dir.rglob("*.hpp")))
        roots.extend(sorted(ecs_dir.rglob("*.h")))
    seen: set[Path] = set()
    chunks: list[str] = []
    for path in roots:
        resolved = path.resolve()
        if resolved in seen or not path.exists() or has_ignored_part(path):
            continue
        seen.add(resolved)
        chunks.append(path.read_text(encoding="utf-8", errors="replace"))
    return "\n".join(chunks)


def _struct_array_fields(struct_name: str) -> set[str]:
    match = re.search(
        r"struct\s+" + re.escape(struct_name) + r"\s*\{(.*?)\};",
        _ecs_source(),
        re.DOTALL,
    )
    if not match:
        return set()
    fields = re.findall(r"TArray<[^;]+>\s+([A-Za-z][A-Za-z0-9_]*)\s*;", match.group(1))
    return {snake(field) for field in fields}


@lru_cache(maxsize=1)
def ecs_domain_kinds() -> frozenset[str]:
    """The ECS domain kind enum values, lowercased (excluding Unknown)."""
    match = re.search(r"enum\s+class\s+(?:EDomainKind|EKind)\s*\{([^}]*)\}", _ecs_source())
    if not match:
        return frozenset()
    names = re.findall(r"[A-Za-z][A-Za-z0-9_]*", match.group(1))
    return frozenset(name.lower() for name in names if name != "Unknown")


@lru_cache(maxsize=1)
def ecs_section_keys() -> frozenset[str]:
    """The ECS registry section keys, derived from the ECS core.

    Node TArray section fields (component_schemas, capabilities,
    systems, resources, events) plus the Component/Entity runtime stores and the
    domain graph container. If the ECS core cannot be read the set is empty and
    section-mixing rules simply do not fire, rather than enforcing a stale copy.
    """
    sections = {
        field
        for struct_name in ("FDomainNode", "FNode")
        for field in _struct_array_fields(struct_name)
        if field != "children"
    }
    sections |= {pluralize(kind) for kind in ecs_domain_kinds() if kind in {"component", "entity"}}
    if re.search(r"\bstruct\s+(?:FDomainRegistry|FGraph)\b", _ecs_source()):
        sections |= {"domains", "domain_graph"}
    return frozenset(sections)
