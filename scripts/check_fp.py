#!/usr/bin/env python3
"""FP checks engine.

The fp/ guards (composition naming, parameter discipline, branchless control
flow) build on this engine. It owns only functional-programming conformance:
composition, expression-oriented control flow, immutable public state, and the
canonical functional substrate. It does not own ECS topology or RTK role rules.

The engine provides FP-specific machinery: a comment/string stripper, a
canonical tokenizer, and the ``func::`` helper vocabulary derived from the SDK's
Core/ue_fp.hpp so guidance and reinvented-helper detection track the functional
substrate instead of a hand-maintained copy.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from functools import lru_cache
import os
from pathlib import Path
import re


# --- Finding model (mirrored across the tri-core engines) ------------------

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
    Severity.MEDIUM: "warning",
    Severity.LOW: "note",
}


@dataclass(frozen=True)
class Rule:
    id: str
    severity: Severity
    summary: str
    guidance: str
    skill: str
    roles: frozenset[str] = frozenset()


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


def format_text(findings: list[Finding], project_root: Path, guard_name: str) -> str:
    lines: list[str] = []
    counts = {sev: 0 for sev in Severity}
    for finding in findings:
        counts[finding.severity] += 1
    summary = ", ".join(f"{counts[sev]} {sev.value}" for sev in Severity if counts[sev])
    lines.append(f"{guard_name} failed: {len(findings)} issue(s) ({summary}).")
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
    return json.dumps({"findings": payload, "count": len(payload)}, indent=2)


def format_sarif(findings: list[Finding], project_root: Path, guard_name: str) -> str:
    import json

    rule_ids = sorted({finding.rule_id for finding in findings if finding.rule_id in RULES})
    sarif_rules = [
        {
            "id": rule_id,
            "name": rule_id,
            "shortDescription": {"text": RULES[rule_id].summary},
            "fullDescription": {"text": RULES[rule_id].guidance},
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
        "runs": [{"tool": {"driver": {"name": guard_name, "rules": sarif_rules}}, "results": results}],
    }
    return json.dumps(sarif, indent=2)


def explain(rule_id: str | None) -> str:
    if rule_id and rule_id in RULES:
        rule = RULES[rule_id]
        return (
            f"{rule.id}  [{rule.severity.value.upper()}]\n"
            f"  what : {rule.summary}\n"
            f"  fix  : {rule.guidance}\n"
            f"  skill: {rule.skill}"
        )
    lines = ["Rules (id  severity  summary):"]
    for rule in sorted(RULES.values(), key=lambda r: (SEVERITY_ORDER[r.severity], r.id)):
        lines.append(f"  {rule.id:<16} {rule.severity.value:<8} {rule.summary}")
    return "\n".join(lines)


# --- Paths -----------------------------------------------------------------

def _path_from_env(name: str, fallback: Path) -> Path:
    value = os.environ.get(name)
    return Path(value).resolve() if value else fallback.resolve()


def _first_existing(paths: tuple[Path, ...]) -> Path:
    return next((path for path in paths if path.exists()), paths[0])


SCRIPT_ROOT = Path(__file__).resolve().parent
PROJECT_ROOT = _path_from_env("FORBOC_FP_PROJECT_ROOT", SCRIPT_ROOT.parent)
SOURCE_ROOT = _path_from_env("FORBOC_FP_SOURCE_ROOT", PROJECT_ROOT / "Source")
UE_FP_HPP = _path_from_env(
    "FORBOC_FP_UE_FP_HPP",
    _first_existing(
        (
            PROJECT_ROOT / "Source" / "ForbocAI_SDK" / "Public" / "Core" / "ue_fp.hpp",
            PROJECT_ROOT / "Plugins" / "ForbocAI_SDK" / "Source" / "ForbocAI_SDK" / "Public" / "Core" / "ue_fp.hpp",
            PROJECT_ROOT.parent / "sdk-ue-5" / "Source" / "ForbocAI_SDK" / "Public" / "Core" / "ue_fp.hpp",
        )
    ),
)
SOURCE_SUFFIXES = {".cpp", ".h", ".hpp"}
IGNORED_PARTS = {".git", "Binaries", "DerivedDataCache", "Intermediate", "Saved", "__pycache__"}


def has_ignored_part(path: Path) -> bool:
    return any(part in IGNORED_PARTS for part in path.parts)


def rel(path: Path, root: Path = PROJECT_ROOT) -> str:
    try:
        return path.relative_to(root).as_posix()
    except ValueError:
        return path.as_posix()


def iter_source_files(root: Path = SOURCE_ROOT) -> list[Path]:
    return sorted(
        path
        for path in root.rglob("*")
        if path.is_file() and path.suffix in SOURCE_SUFFIXES and not has_ignored_part(path)
    )


# --- Comment/string stripping ----------------------------------------------

def code_only(text: str) -> str:
    """Blank comments and string/char literals, preserving offsets/newlines."""
    out: list[str] = []
    index = 0
    length = len(text)
    state = "code"
    quote = ""
    while index < length:
        char = text[index]
        nxt = text[index + 1] if index + 1 < length else ""
        if state == "line_comment":
            out.append("\n" if char == "\n" else " ")
            state = "code" if char == "\n" else state
            index += 1
            continue
        if state == "block_comment":
            end = char == "*" and nxt == "/"
            out.append("  " if end else ("\n" if char == "\n" else " "))
            index += 2 if end else 1
            state = "code" if end else state
            continue
        if state in ("string", "char"):
            if char == "\\":
                out.append("  ")
                index += 2
                continue
            out.append("\n" if char == "\n" else " ")
            state = "code" if char == quote else state
            index += 1
            continue
        if char == "/" and nxt == "/":
            out.append("  ")
            state = "line_comment"
            index += 2
            continue
        if char == "/" and nxt == "*":
            out.append("  ")
            state = "block_comment"
            index += 2
            continue
        if char in ('"', "'"):
            out.append(" ")
            quote = char
            state = "string" if char == '"' else "char"
            index += 1
            continue
        out.append(char)
        index += 1
    return "".join(out)


# --- Canonical tokenizer ---------------------------------------------------

def camel_tokens(value: str) -> list[str]:
    if not value:
        return []
    return re.findall(r"[A-Z]+(?=[A-Z][a-z]|[0-9]|\b)|[A-Z]?[a-z]+|[0-9]+", value) or [value]


def folder_tokens(part: str) -> list[str]:
    tokens: list[str] = []
    for piece in re.split(r"[_\-\s]+", part):
        tokens.extend(camel_tokens(piece))
    return tokens


def normalize(token: str) -> str:
    value = token.replace("-", "_").lower()
    if value.endswith("ies") and len(value) > 3:
        return value[:-3] + "y"
    if value.endswith("s") and not value.endswith("ss") and len(value) > 1:
        return value[:-1]
    return value


# --- ue_fp.hpp helper vocabulary -------------------------------------------
# The FP substrate is the source of truth for which func:: helpers replace loops
# and branches, and for detecting hand-rolled reinventions. Add a helper to
# ue_fp.hpp and the guards track it; if ue_fp.hpp cannot be read the vocabulary
# is empty and vocabulary-driven rules simply do not fire.

_FP_NAME_RE = re.compile(r"\b([a-z][a-z0-9_]{2,})\s*\(")
_FP_PREFIXES = (
    "fold", "map", "for_each", "filter", "find", "traverse", "any", "all",
    "sequence", "lift", "match", "multi_match", "dispatch", "compose", "pipe",
    "converge", "juxt", "catalog", "zip", "index_range", "map_grid",
    "from_nullable", "require_just", "or_else", "unique", "contains", "concat",
    "append", "upsert", "update_map",
)
_LOOP_PREFIXES = (
    "fold", "map", "for_each", "filter", "find", "traverse", "any", "all",
    "sequence", "index_range", "zip", "catalog", "unique", "concat",
)
_ROUTE_PREFIXES = ("match", "multi_match", "dispatch", "when", "wildcard", "equals")


@lru_cache(maxsize=1)
def _ue_fp_source() -> str:
    return UE_FP_HPP.read_text(encoding="utf-8", errors="replace") if UE_FP_HPP.exists() else ""


@lru_cache(maxsize=1)
def fp_helper_names() -> frozenset[str]:
    """Every func:: composition helper name present in ue_fp.hpp."""
    names = set(_FP_NAME_RE.findall(_ue_fp_source()))
    return frozenset(name for name in names if name.startswith(_FP_PREFIXES))


def fp_helpers_for(kind: str, limit: int = 8) -> list[str]:
    """Helper names for a control-flow kind: 'loop' or 'route'."""
    prefixes = _ROUTE_PREFIXES if kind == "route" else _LOOP_PREFIXES
    return sorted(name for name in fp_helper_names() if name.startswith(prefixes))[:limit]
