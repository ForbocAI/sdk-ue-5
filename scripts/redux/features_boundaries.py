#!/usr/bin/env python3
"""Shared engine for the RTK boundary guards.

This module is the whole engine the per-role guards build on:

* ``code_only`` blanks comments, string/char literals, and preprocessor lines so
  a rule never fires on a comment, a string, or an ``#include`` path.
* ``SourceUnit`` reads a file (and its sibling ``.cpp``) exactly once and hands a
  small model to every rule.
* ``Rule``/``register``/``RULES`` make each rule a first-class object with a
  stable id, a severity aligned to the redux/rtk skills, and a skill citation --
  which powers ``--explain`` and the JSON/SARIF emitters.
* Content-based role fingerprinting and the feature import graph let the guards
  reason about a file's *observed* role and cross-file structure, not just its
  name.

Adding a rule is data + a matcher; adding a role is a ``*_boundaries.py`` module
that the runner auto-discovers.
"""

from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from pathlib import Path
import re
from typing import Callable, Iterable


SOURCE_EXTENSIONS = {".h", ".hpp", ".cpp"}
IMPORT_RE = re.compile(r'^\s*#\s*include\s+"([^"]+)"', re.MULTILINE)


# --- Roles -----------------------------------------------------------------

ROLE_BY_STEM: dict[str, str] = {
    "Actions": "actions",
    "Adapters": "adapters",
    "Api": "api",
    "Listeners": "listeners",
    "Selectors": "selectors",
    "Slice": "slice",
    "Thunks": "thunks",
    "Types": "types",
    "View": "view",
}

ROLE_STEM_BY_ROLE = {role: stem for stem, role in ROLE_BY_STEM.items()}

ROLE_GUIDANCE: dict[str, str] = {
    "actions": "Actions describe events or expose narrow UE dispatch facades; reducers own the state transition.",
    "adapters": "Adapters translate authored JSON, UE objects, or external data at boundaries without dispatching.",
    "api": "Api owns RTK Query createApi/injectEndpoints boundaries for server data and cache lifecycles.",
    "listeners": "Listeners model reactive workflows that respond to future actions or state changes.",
    "selectors": "Selectors derive state and keep views from reading store internals directly.",
    "slice": "Slice owns createSlice assembly, initial state, and pure reducer transition functions.",
    "thunks": "Thunks model one imperative async workflow with dispatch/getState.",
    "types": "Types stay inert: state, payload, entity, and dispatch declarations only.",
    "view": "Views render and bind UE lifecycle, dispatching through Actions and reading through Selectors only.",
}

# The factory each role owns, keyed by role. A role that observes another role's
# owned factory in its text is either misplaced or mislabeled.
OWNED_FACTORY_BY_ROLE: dict[str, re.Pattern[str]] = {
    "actions": re.compile(r"\brtk::createAction\s*<"),
    "adapters": re.compile(r"\brtk::createEntityAdapter\s*<"),
    "listeners": re.compile(
        r"\bcreateListenerMiddleware\s*<|\bCreateRuntimeListenerMiddleware\b"
    ),
    "selectors": re.compile(r"\brtk::createSelector\s*<"),
    "slice": re.compile(r"\brtk::createSlice\s*<"),
    "thunks": re.compile(r"\brtk::createAsyncThunk\s*<"),
}

# Where an owned factory may ALSO legitimately appear (secondary homes).
FACTORY_SECONDARY_ROLES: dict[str, set[str]] = {
    "selectors": {"slice"},  # slice-local memoized selectors are fine
}


# --- Severity + findings ---------------------------------------------------

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
    """A boundary rule: stable id, severity, and the skill it enforces."""

    id: str
    severity: Severity
    summary: str
    guidance: str
    skill: str
    roles: frozenset[str] = frozenset()  # empty => global / any role


RULES: dict[str, Rule] = {}


def register(rule: Rule) -> Rule:
    if rule.id in RULES and RULES[rule.id] is not rule:
        raise ValueError(f"duplicate boundary rule id: {rule.id}")
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


# --- Code-only scanning ----------------------------------------------------

def blank_preprocessor(text: str) -> str:
    """Replace preprocessor directive lines with spaces, preserving offsets."""
    return "\n".join(
        (" " * len(line)) if line.lstrip().startswith("#") else line
        for line in text.split("\n")
    )


def code_only(text: str) -> str:
    """Blank comments, string/char literals, and preprocessor lines.

    Offsets and newlines are preserved so a match position still maps to the
    original line. Identifiers and operators survive; literal text, comments,
    and directives become spaces, so a rule cannot fire on them.
    """
    prepped = blank_preprocessor(text)
    out: list[str] = []
    index = 0
    length = len(prepped)
    state = "code"
    while index < length:
        char = prepped[index]
        nxt = prepped[index + 1] if index + 1 < length else ""
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
        if state == "string":
            if char == "\\":
                out.append("  ")
                index += 2
                continue
            out.append("\n" if char == "\n" else " ")
            state = "code" if char == '"' else state
            index += 1
            continue
        if state == "char":
            if char == "\\":
                out.append("  ")
                index += 2
                continue
            out.append(" ")
            state = "code" if char == "'" else state
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
        if char == '"':
            out.append(" ")
            state = "string"
            index += 1
            continue
        if char == "'":
            out.append(" ")
            state = "char"
            index += 1
            continue
        out.append(char)
        index += 1
    return "".join(out)


def line_number(text: str, index: int) -> int:
    return text.count("\n", 0, index) + 1


def column_number(text: str, index: int) -> int:
    return index - (text.rfind("\n", 0, index) + 1) + 1


# --- Source model ----------------------------------------------------------

@dataclass(frozen=True)
class SourceUnit:
    """A file read once: raw text, code-only text, includes, and its sibling."""

    path: Path
    root: Path
    stem: str
    declared_role: str | None
    raw: str
    code: str
    includes: tuple[str, ...]
    sibling_code: str
    role_code: str  # code + sibling code-only, for cross-.h/.cpp role rules

    @property
    def rel(self) -> Path:
        try:
            return self.path.relative_to(self.root)
        except ValueError:
            return self.path


def _sibling_raw(path: Path) -> str:
    if path.suffix not in {".h", ".hpp"}:
        return ""
    candidates = [path.with_suffix(".cpp")]
    role = role_for_path(path)
    if role is not None:
        role_stem = ROLE_STEM_BY_ROLE[role]
        qualifiers = {
            qualifier
            for marker in ("Features", "Views")
            for qualifier in _folder_suffix_qualifiers(path, marker)
            if qualifier
        }
        candidates.extend(path.with_name(qualifier + role_stem + ".cpp") for qualifier in qualifiers)
    seen: set[Path] = set()
    for sibling in candidates:
        if sibling in seen:
            continue
        seen.add(sibling)
        if sibling.exists():
            return sibling.read_text(encoding="utf-8", errors="replace")
    return ""


def build_unit(path: Path, root: Path) -> SourceUnit:
    raw = path.read_text(encoding="utf-8", errors="replace")
    code = code_only(raw)
    sibling_raw = _sibling_raw(path)
    sibling_code = code_only(sibling_raw) if sibling_raw else ""
    includes = tuple(match.group(1) for match in IMPORT_RE.finditer(raw))
    role_code = code if not sibling_code else code + "\n" + sibling_code
    return SourceUnit(
        path=path,
        root=root,
        stem=path.stem,
        declared_role=role_for_path(path),
        raw=raw,
        code=code,
        includes=includes,
        sibling_code=sibling_code,
        role_code=role_code,
    )


def role_for_stem(stem: str) -> str | None:
    return ROLE_BY_STEM.get(stem)


def role_for_stem_suffix(stem: str) -> str | None:
    for role_stem, role in sorted(ROLE_BY_STEM.items(), key=lambda item: len(item[0]), reverse=True):
        if stem.endswith(role_stem):
            return role
    return None


def _camel_name(value: str) -> str:
    return "".join(
        token[:1].upper() + token[1:]
        for token in re.split(r"[^A-Za-z0-9]+", value)
        if token
    )


def _folder_parts_after_marker(path: Path, marker: str) -> tuple[str, ...]:
    parts = path.parts
    if marker not in parts:
        return ()
    index = parts.index(marker)
    return tuple(parts[index + 1 : -1])


def _is_under_marker(path: Path, marker: str) -> bool:
    return bool(_folder_parts_after_marker(path, marker))


def _folder_suffix_qualifiers(path: Path, marker: str) -> tuple[str, ...]:
    parts = _folder_parts_after_marker(path, marker)
    return tuple(
        "".join(_camel_name(part) for part in parts[index:])
        for index in range(len(parts) - 1, -1, -1)
    )


def _qualified_role_for_path(path: Path) -> str | None:
    stem = path.stem
    markers = ("Features", "Views")
    qualifiers = {
        qualifier
        for marker in markers
        for qualifier in _folder_suffix_qualifiers(path, marker)
        if qualifier
    }
    for role_stem, role in ROLE_BY_STEM.items():
        if any(stem == qualifier + role_stem for qualifier in qualifiers):
            return role
    return None


def role_for_path(path: Path) -> str | None:
    if _is_under_marker(path, "Features") or _is_under_marker(path, "Views"):
        return _qualified_role_for_path(path) or role_for_stem_suffix(path.stem)
    return role_for_stem(path.stem)


def role_for_include(include: str) -> str | None:
    parts = Path(include).parts
    if "Features" not in parts:
        return None
    return role_for_path(Path(*parts[parts.index("Features") :]))


def feature_relative_include(include: str) -> Path | None:
    parts = Path(include).parts
    if "Features" not in parts:
        return None
    return Path(*parts[parts.index("Features") + 1 :])


def iter_source_files(roots: Iterable[Path]) -> Iterable[Path]:
    for root in roots:
        path = root.resolve()
        if path.is_file() and path.suffix in SOURCE_EXTENSIONS:
            yield path
        elif path.is_dir():
            yield from sorted(
                child
                for child in path.rglob("*")
                if child.is_file() and child.suffix in SOURCE_EXTENSIONS
            )


# --- Rule matching helpers -------------------------------------------------

def findings_for(
    unit: SourceUnit,
    text: str,
    rule: Rule,
    pattern: re.Pattern[str],
    *,
    once: bool = False,
) -> list[Finding]:
    """Emit a Finding per match of ``pattern`` in ``text`` (code-only)."""
    findings: list[Finding] = []
    for match in pattern.finditer(text):
        findings.append(
            Finding(
                unit.path,
                line_number(text, match.start()),
                rule.id,
                rule.severity,
                rule.summary,
                column_number(text, match.start()),
            )
        )
        if once:
            break
    return findings


def presence_finding(
    unit: SourceUnit,
    export_pattern: re.Pattern[str],
    factory_pattern: re.Pattern[str],
    rule: Rule,
) -> list[Finding]:
    """Flag files that expose a role export without the owning RTK factory."""
    if export_pattern.search(unit.role_code) and not factory_pattern.search(unit.role_code):
        return [Finding(unit.path, 1, rule.id, rule.severity, rule.summary)]
    return []


# --- Content-based role fingerprint ---------------------------------------

FINGERPRINT = register(
    Rule(
        id="RTK-ROLE-001",
        severity=Severity.HIGH,
        summary="file assembles an RTK factory that its role name does not own",
        guidance=(
            "Name the leaf for what it builds: createSlice -> Slice, "
            "createAsyncThunk -> Thunks, createAction -> Actions, "
            "createEntityAdapter -> Adapters, createListenerMiddleware -> "
            "Listeners. The leaf must be folder-qualified, such as BotsSlice."
        ),
        skill="model-redux-state-design-state-ownership: naming state after the wrong role",
    )
)


def fingerprint_findings(unit: SourceUnit) -> list[Finding]:
    """Reconcile the declared (filename) role with the observed (factory) role."""
    declared = unit.declared_role
    if declared is None or declared in {"types", "view"}:
        return []
    findings: list[Finding] = []
    for owner_role, pattern in OWNED_FACTORY_BY_ROLE.items():
        if owner_role == declared:
            continue
        if declared in FACTORY_SECONDARY_ROLES.get(owner_role, set()):
            continue
        match = pattern.search(unit.code)
        if match:
            findings.append(
                Finding(
                    unit.path,
                    line_number(unit.code, match.start()),
                    FINGERPRINT.id,
                    FINGERPRINT.severity,
                    f"{unit.stem} is named `{declared}` but assembles a `{owner_role}` factory",
                    column_number(unit.code, match.start()),
                )
            )
    return findings


# --- Import graph ----------------------------------------------------------

def role_edges(units: list[SourceUnit]) -> list[tuple[Path, str, Path]]:
    """(importer_path, target_include, imported_feature_path-ish) role edges."""
    edges: list[tuple[Path, str, Path]] = []
    for unit in units:
        for include in unit.includes:
            target = role_for_include(include)
            if target:
                edges.append((unit.path, target, Path(include)))
    return edges


def find_import_cycles(units: list[SourceUnit]) -> list[list[str]]:
    """Detect include cycles among feature role files (by include target)."""
    graph: dict[str, set[str]] = {}
    for unit in units:
        rel_include = _feature_include_key(unit)
        if rel_include is None:
            continue
        targets = {
            target
            for include in unit.includes
            for target in [_include_key(include)]
            if target is not None
        }
        graph.setdefault(rel_include, set()).update(targets)

    cycles: list[list[str]] = []
    visiting: list[str] = []
    visited: set[str] = set()

    def walk(node: str) -> None:
        if node in visiting:
            start = visiting.index(node)
            cycles.append(visiting[start:] + [node])
            return
        if node in visited:
            return
        visiting.append(node)
        for target in sorted(graph.get(node, set())):
            walk(target)
        visiting.pop()
        visited.add(node)

    for node in sorted(graph):
        walk(node)
    # De-duplicate rotations.
    unique: list[list[str]] = []
    seen: set[frozenset[str]] = set()
    for cycle in cycles:
        key = frozenset(cycle)
        if key not in seen:
            seen.add(key)
            unique.append(cycle)
    return unique


def _include_key(include: str) -> str | None:
    parts = Path(include).parts
    if "Features" not in parts:
        return None
    return Path(*parts[parts.index("Features") :]).as_posix()


def _feature_include_key(unit: SourceUnit) -> str | None:
    parts = unit.path.parts
    if "Features" not in parts:
        return None
    return Path(*parts[parts.index("Features") :]).as_posix()


# --- Output formatters -----------------------------------------------------

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
    lines = ["Boundary rules (id  severity  summary):"]
    for rule in sorted(RULES.values(), key=lambda r: (SEVERITY_ORDER[r.severity], r.id)):
        lines.append(f"  {rule.id:<16} {rule.severity.value:<8} {rule.summary}")
    return "\n".join(lines)
