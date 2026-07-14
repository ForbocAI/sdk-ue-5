#!/usr/bin/env python3
"""Enforce the fp.hpp composition/naming cookbook, on the check_fp engine.

Derived from Core/fp.hpp's unary-composition cookbook (the megaphone rule).
It catches naming and call-shape violations that hide non-compositional bags:

* path/domain words repeated inside C++ identifiers;
* wrapper nouns such as Fixture/Bag/Manager/Helper/Utils;
* repeated typed factory families such as BindFloat/ComponentField/ReadMember;
* per-field registry entries instead of grouped declaration atoms;
* digit-suffixed helper/macro families such as READ_FIELD_1/FIELDS_32;
* hand-composed nested func::compose(...) chains that should be pipe3/pipe4/fold;
* wide scalar structs split into subdomain data recomposed by an adapter/fold.

Context is derived from paths and identifiers; there is no domain list, because
domains stay free to move. Findings carry stable ids/severities from the shared
engine.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from check_fp import (
    Finding,
    Rule,
    Severity,
    camel_tokens,
    explain,
    folder_tokens,
    format_json,
    format_sarif,
    format_text,
    iter_source_files,
    line_number,
    normalize,
    register,
    rel,
    PROJECT_ROOT,
    SOURCE_ROOT,
)


LAZY_NOUNS = {
    "Bag": "A Bag name hides unrelated data. Split by subdomain and compose at the adapter/fold boundary.",
    "Fixture": "A Fixture name turns test data into a bag. Move authored values to subdomain JSON and select the real values in the test.",
    "Helper": "A Helper name hides the actual role. Name the role boundary or move behavior into a reusable unary composer.",
    "Manager": "A Manager name hides ownership. Name the explicit data-flow boundary and the composition it owns.",
    "Util": "A Util name hides the role. Move neutral behavior down or name the exact composition boundary.",
    "Utils": "A Utils name hides the role. Move neutral behavior down or name the exact composition boundary.",
}

COOKBOOK_CALL_RULES: tuple[tuple[re.Pattern[str], str], ...] = (
    (re.compile(r"\b(?:SettingField|ComponentField)\s*\("),
     "Per-field wrapper calls are not compositional. Pass grouped field/path atoms to one reusable composer; the composer owns expansion, conversion, validation, and folds."),
    (re.compile(r"\bBind(?:Bool|Text|String|Float|Double|Int|Vector|Rotator|Color)\s*(?:<|\()"),
     "Repeated typed Bind* factories are the cookbook's wrong shape. Push typed reader selection into the lower composer and leave this layer with declaration data."),
    (re.compile(r"\b(?:ReadMember|ReadField|WriteField)\s*\("),
     "Do not hand-compose per-member readers/writers. Declare source paths/atoms as data and let one generic reader/projector composer expand them."),
    (re.compile(r"\b(?:Required|Sequence)\s*\("),
     "Do not nest Required/Sequence helper calls inside declarations unless the behavior is genuinely one-off. Use grouped declaration atoms and a reusable composer."),
    (re.compile(r"\bfield_names\s*\("),
     "field_names plus field_readers is noun/type plumbing. Replace it with one grouped declaration consumed by a reusable composer."),
    (re.compile(r"\bfield_readers\s*\("),
     "field_readers plus field_names is noun/type plumbing. Replace it with one grouped declaration consumed by a reusable composer."),
    (re.compile(r"\bRegisteredFields\s*=\s*\{\s*\{", re.DOTALL),
     "Hand-written per-field registry entries leak the repeated shape. A registry declares grouped atoms/paths/conversions and lets one composer expand them."),
    (re.compile(r"\bRegistered(?:Rules|Cases|ProjectionPasses|Groups|Domain)\s*\([^;]*\bRegistered(?:Fields|Rules|Cases|ProjectionPasses)\s*\(", re.DOTALL),
     "Do not nest RegisteredFields/RegisteredRules/RegisteredCases inside another declaration. The outer boundary owns expansion; call sites supply grouped atoms."),
    (re.compile(r"\b(?:READ_FIELD|GET_FIELD_READER|FIELDS)_?\d+\b"),
     "Digit-suffixed helper/macro families are arity plumbing. Use one named composition boundary plus grouped declaration data."),
    (re.compile(r"\bCompose[A-Z][A-Za-z0-9_]*\s*\("),
     "Repeated ComposeX-style call families are abstraction leaks. Move the repeated shape into a higher composer and feed it grouped declarations."),
    (re.compile(r"\bProjectPayload[A-Z][A-Za-z0-9_]*\s*\("),
     "Repeated ProjectPayloadY-style call families are abstraction leaks. Move entity/source/field selection into one projector composer fed by declarations."),
)

NESTED_COMPOSE_RE = re.compile(r"\bcompose\s*\([^;{}]*\bcompose\s*\(")

WRAPPER_FAMILY_RE = re.compile(
    r"^\s*(?!template\b)(?:struct|class)\s+"
    r"([A-Z][A-Za-z0-9_]*(?:Seeds|Declarations|Registry|Catalog|Fields|Rules|Cases))"
    r"\b[^{;]*\{",
    re.MULTILINE,
)
SCALAR_FIELD_RE = re.compile(
    r"^\s*(?:const\s+)?(?:"
    r"FString|FName|FText|float|double|int32|int64|uint32|uint64|bool|"
    r"FVector|FRotator|FColor|FLinearColor|TArray\s*<\s*FString\s*>"
    r")\s+[A-Za-z_][A-Za-z0-9_]*\s*(?:=.*)?;",
    re.MULTILINE,
)
STRUCT_RE = re.compile(r"\bstruct\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{", re.MULTILINE)
TYPE_DECL_RE = re.compile(r"\b(?:class|enum(?:\s+class)?)\s+([A-Za-z_][A-Za-z0-9_]*)\b", re.MULTILINE)
FUNCTION_DECL_RE = re.compile(
    r"^\s*(?:(?:inline|static|virtual|FORCEINLINE|constexpr)\s+)*"
    r"(?:[\w:<>,&*]+\s+)+([A-Za-z_][A-Za-z0-9_]*)"
    r"\s*\([^;\n{}]*\)\s*(?:const\s*)?(?:\{|;)",
    re.MULTILINE,
)
CONTROL_NAMES = {"if", "for", "while", "switch", "return", "sizeof", "TEXT"}
WIDE_SCALAR_FIELD_COUNT = 6
TEXT_ATOM_REPEAT_COUNT = 8
TEXT_LITERAL_RE = re.compile(r'TEXT\("')
EXTERNAL_UNREAL_TYPE_NAMES = {
    "FEvent",
}


LAZY_NOUN = register(Rule(
    "FP-COMP-001", Severity.HIGH, "identifier uses a lazy wrapper noun",
    "Name the exact role/domain boundary. Split vague bags/helpers into subdomain data and reusable composers.",
    "fp.hpp cookbook: name the composition boundary, not a bag/helper wrapper"))
REPEATED_PATH_TOKEN = register(Rule(
    "FP-COMP-002", Severity.MEDIUM, "identifier repeats a domain atom already in its path",
    "Keep the shortest collision-free name derived from the path/namespace atom ladder.",
    "fp.hpp cookbook: folder/namespace boundaries own domain words"))
COOKBOOK_CALL = register(Rule(
    "FP-COMP-003", Severity.HIGH, "call shape repeats field/member composition",
    "Pass grouped declaration atoms to one reusable composer; the composer owns conversion and folds.",
    "fp.hpp megaphone rule: grouped declaration atoms + one composer, not per-field wrappers"))
WRAPPER_FAMILY = register(Rule(
    "FP-COMP-004", Severity.MEDIUM, "struct/class is a noun wrapper family",
    "Put atoms/selectors/projectors/validators in grouped declarations and fold them with one composer.",
    "fp.hpp cookbook: registries declare atoms/paths as data, folded by one composer"))
TEXT_ATOM_REPETITION = register(Rule(
    "FP-COMP-005", Severity.MEDIUM, "file repeats platform text atoms",
    "Move authored atoms to grouped declarations or JSON and let generic helpers own TEXT conversion.",
    "fp.hpp cookbook: generic declaration helpers own TEXT(...) conversion"))
WIDE_SCALAR_STRUCT = register(Rule(
    "FP-COMP-006", Severity.MEDIUM, "wide scalar/list struct hides subdomains",
    "Split wide scalar/list records by subdomain and compose them through a reader/adapter fold.",
    "fp.hpp cookbook: split wide scalar bags, recompose through a fold"))
NESTED_COMPOSE = register(Rule(
    "FP-COMP-007", Severity.MEDIUM, "hand-composed nested func::compose(...) chain",
    "Assemble more than two functions with func::pipe3/pipe4 or a fold over a catalog, not nested compose(compose(...)).",
    "fp.hpp cookbook: do not hand-write compose(...) chains; feed a composer grouped declarations"))


@dataclass(frozen=True)
class QualifiedNameContext:
    expected_by_path_name: dict[tuple[Path, str], str]


@dataclass(frozen=True)
class StructNameContext:
    expected_by_key: dict[tuple[Path, int, str], str]


def _finding(path: Path, line: int, rule: Rule, message: str) -> Finding:
    return Finding(path, line, rule.id, rule.severity, message)


def strip_unreal_prefix(tokens: list[str]) -> list[str]:
    if tokens and tokens[0] in {"A", "E", "F", "I", "S", "T", "U"}:
        return tokens[1:]
    if (tokens and len(tokens[0]) > 1 and tokens[0][0] in {"A", "E", "F", "I", "S", "T", "U"}
            and tokens[0][1:].isupper()):
        return [tokens[0][1:]] + tokens[1:]
    return tokens


def split_unreal_name(name: str) -> tuple[str, list[str]]:
    simple = name.rsplit("::", 1)[-1].lstrip("~")
    if len(simple) > 1 and simple[0] in {"A", "E", "F", "I", "S", "T", "U"} and simple[1].isupper():
        return simple[0], camel_tokens(simple[1:])
    return "", camel_tokens(simple)


def atom_domain_parts(path: Path) -> tuple[str, ...]:
    try:
        rel_path = path.relative_to(SOURCE_ROOT)
    except ValueError:
        return path.parent.parts
    parts = rel_path.parent.parts
    for marker in ("Features", "Views", "Tests"):
        if marker in parts:
            return tuple(parts[parts.index(marker) + 1:])
    return tuple(parts)


def atom_groups(path: Path) -> tuple[tuple[str, ...], ...]:
    return tuple(tuple(folder_tokens(part)) for part in atom_domain_parts(path))


def atom_suffixes(path: Path) -> tuple[tuple[str, ...], ...]:
    groups = atom_groups(path)
    return tuple(
        tuple(token for group in groups[index:] for token in group)
        for index in range(len(groups) - 1, -1, -1)
    )


def atom_qualified_name_candidates(path: Path, name: str) -> tuple[str, ...]:
    unreal_prefix, tokens = split_unreal_name(name)
    for qualifier in sorted(atom_suffixes(path), key=len, reverse=True):
        if not qualifier or len(tokens) <= len(qualifier):
            continue
        if tuple(tokens[: len(qualifier)]) != qualifier:
            continue
        local_tokens = tokens[len(qualifier):]
        candidates = [unreal_prefix + "".join(local_tokens)]
        candidates.extend(
            unreal_prefix + "".join(list(candidate) + local_tokens)
            for candidate in atom_suffixes(path)
            if candidate
        )
        return tuple(dict.fromkeys(candidates))
    return ()


def _resolve_shortest(items: list[dict], collides) -> None:
    while True:
        changed = False
        counts: dict[str, int] = {}
        for item in items:
            candidate = item["candidates"][item["index"]]
            counts[candidate] = counts.get(candidate, 0) + 1
        for item in items:
            candidate = item["candidates"][item["index"]]
            if (counts[candidate] > 1 or collides(item, candidate)) and item["index"] < len(item["candidates"]) - 1:
                item["index"] += 1
                changed = True
        if not changed:
            return


def qualified_name_context(paths: list[Path]) -> QualifiedNameContext:
    all_names: set[str] = set(EXTERNAL_UNREAL_TYPE_NAMES)
    item_by_key: dict[tuple[Path, str], dict] = {}
    for path in paths:
        text = path.read_text(encoding="utf-8", errors="replace")
        for pattern in (STRUCT_RE, TYPE_DECL_RE):
            for match in pattern.finditer(text):
                simple = match.group(1).rsplit("::", 1)[-1].lstrip("~")
                if simple in CONTROL_NAMES:
                    continue
                all_names.add(simple)
                candidates = atom_qualified_name_candidates(path, simple)
                if candidates:
                    item_by_key.setdefault(
                        (path, simple),
                        {"path": path, "name": simple, "candidates": candidates, "index": 0},
                    )
    items = list(item_by_key.values())
    _resolve_shortest(items, lambda item, candidate: candidate in all_names and candidate != item["name"])
    return QualifiedNameContext(
        expected_by_path_name={(item["path"], item["name"]): item["candidates"][item["index"]] for item in items}
    )


def struct_name_context(paths: list[Path]) -> StructNameContext:
    items: list[dict] = []
    existing: dict[str, list[tuple[Path, int, str]]] = {}
    for path in paths:
        text = path.read_text(encoding="utf-8", errors="replace")
        for match in STRUCT_RE.finditer(text):
            name = match.group(1)
            key = (path, match.start(), name)
            existing.setdefault(name, []).append(key)
            candidates = atom_qualified_name_candidates(path, name)
            if candidates:
                items.append({"key": key, "name": name, "candidates": candidates, "index": 0})
    _resolve_shortest(
        items,
        lambda item, candidate: any(key != item["key"] for key in existing.get(candidate, [])),
    )
    return StructNameContext(
        expected_by_key={item["key"]: item["candidates"][item["index"]] for item in items}
    )


def lazy_noun_issues(path: Path, text: str, name: str, offset: int) -> list[Finding]:
    tokens = strip_unreal_prefix(camel_tokens(name.rsplit("::", 1)[-1].lstrip("~")))
    return [
        _finding(path, line_number(text, offset), LAZY_NOUN, LAZY_NOUNS[token])
        for token in tokens
        if token in LAZY_NOUNS
    ]


def repeated_path_token_issue(path: Path, text: str, name: str, offset: int,
                              context: QualifiedNameContext | None) -> Finding | None:
    simple = name.rsplit("::", 1)[-1].lstrip("~")
    expected = context.expected_by_path_name.get((path, simple)) if context is not None else None
    if expected is None or expected == simple:
        return None
    return _finding(
        path, line_number(text, offset), REPEATED_PATH_TOKEN,
        f"Identifier `{simple}` uses a broader domain atom qualifier than needed; expected shortest collision-free name `{expected}`.",
    )


def matching_brace_index(text: str, open_index: int) -> int | None:
    depth = 1
    index = open_index + 1
    while index < len(text):
        depth += 1 if text[index] == "{" else (-1 if text[index] == "}" else 0)
        if depth == 0:
            return index
        index += 1
    return None


def struct_body(text: str, match: re.Match[str]) -> str | None:
    close = matching_brace_index(text, match.end() - 1)
    return None if close is None else text[match.end(): close]


def wide_struct_issue(path: Path, text: str, match: re.Match[str], context: StructNameContext) -> Finding | None:
    name = match.group(1)
    body = struct_body(text, match)
    if body is None:
        return None
    scalar_fields = SCALAR_FIELD_RE.findall(body)
    if len(scalar_fields) <= WIDE_SCALAR_FIELD_COUNT:
        return None
    expected = context.expected_by_key.get((path, match.start(), name), name)
    qualifier = (
        f" Current name should shorten to `{expected}` if it remains in this domain; split records must use the same nearest-domain collision rule."
        if expected != name
        else " Split records must keep the shortest collision-free domain qualifier."
    )
    return _finding(
        path, line_number(text, match.start()), WIDE_SCALAR_STRUCT,
        f"Wide scalar struct `{name}` has {len(scalar_fields)} direct scalar/list fields. Split by subdomain and compose smaller records through a reader/adapter fold.{qualifier}",
    )


def cookbook_call_issues(path: Path, text: str) -> list[Finding]:
    findings = [
        _finding(path, line_number(text, match.start()), COOKBOOK_CALL, message)
        for pattern, message in COOKBOOK_CALL_RULES
        for match in pattern.finditer(text)
    ]
    findings += [
        _finding(path, line_number(text, match.start()), NESTED_COMPOSE, NESTED_COMPOSE.summary)
        for match in NESTED_COMPOSE_RE.finditer(text)
    ]
    return findings


def wrapper_family_issues(path: Path, text: str) -> list[Finding]:
    findings: list[Finding] = []
    for match in WRAPPER_FAMILY_RE.finditer(text):
        line_start = text.rfind("\n", 0, match.start()) + 1
        previous_start = text.rfind("\n", 0, line_start - 1) + 1
        if text[previous_start: line_start - 1].strip().startswith("template"):
            continue
        findings.append(_finding(
            path, line_number(text, match.start()), WRAPPER_FAMILY,
            f"`{match.group(1)}` looks like a noun wrapper family. If only nouns change, put atoms/selectors/projectors/validators in grouped declarations and fold them with one composer.",
        ))
    return findings


def authored_text_atom_count(text: str) -> int:
    count = 0
    for match in TEXT_LITERAL_RE.finditer(text):
        prefix = text[max(0, match.start() - 160): match.start()]
        if any(marker in prefix for marker in ("FString::Printf(", "checkf(", "UE_LOG(")):
            continue
        count += 1
    return count


def text_atom_repetition_issue(path: Path, text: str) -> Finding | None:
    count = authored_text_atom_count(text)
    if count < TEXT_ATOM_REPEAT_COUNT or "JSON_SETTINGS_ATOMS" in text or "JSON_REQUIRED_ATOMS" in text:
        return None
    return _finding(
        path, 1, TEXT_ATOM_REPETITION,
        f"File repeats TEXT(...) atoms {count} times. Generic declaration helpers own platform string conversion; move authored atoms to grouped declarations or JSON.",
    )


def scan_file(path: Path, context: StructNameContext | None = None,
              qualified_context: QualifiedNameContext | None = None) -> list[Finding]:
    text = path.read_text(encoding="utf-8", errors="replace")
    name_context = context if context is not None else struct_name_context([path])
    findings: list[Finding] = []
    seen: set[tuple[int, str, str]] = set()

    def add(finding: Finding) -> None:
        key = (finding.line, finding.rule_id, finding.message)
        if key not in seen:
            seen.add(key)
            findings.append(finding)

    for match in STRUCT_RE.finditer(text):
        name = match.group(1)
        for finding in lazy_noun_issues(path, text, name, match.start()):
            add(finding)
        repeated = repeated_path_token_issue(path, text, name, match.start(), qualified_context)
        if repeated is not None:
            add(repeated)
        wide = wide_struct_issue(path, text, match, name_context)
        if wide is not None:
            add(wide)

    for match in TYPE_DECL_RE.finditer(text):
        name = match.group(1)
        for finding in lazy_noun_issues(path, text, name, match.start()):
            add(finding)
        repeated = repeated_path_token_issue(path, text, name, match.start(), qualified_context)
        if repeated is not None:
            add(repeated)

    for match in FUNCTION_DECL_RE.finditer(text):
        if match.group(1) in CONTROL_NAMES:
            continue
        for finding in lazy_noun_issues(path, text, match.group(1), match.start()):
            add(finding)

    for finding in cookbook_call_issues(path, text):
        add(finding)
    for finding in wrapper_family_issues(path, text):
        add(finding)
    text_atom = text_atom_repetition_issue(path, text)
    if text_atom is not None:
        add(text_atom)

    return findings


def find_findings() -> list[Finding]:
    files = iter_source_files(SOURCE_ROOT)
    context = struct_name_context(files)
    qualified_context = qualified_name_context(files)
    findings: list[Finding] = []
    for path in files:
        findings.extend(scan_file(path, context, qualified_context))
    return findings


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--format", choices=("text", "json", "sarif"), default="text")
    parser.add_argument("--explain", nargs="?", const="", metavar="RULE-ID")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.explain is not None:
        print(explain(args.explain or None))
        return 0
    findings = find_findings()
    guard = "Function/data composition naming guard"
    if args.format == "json":
        print(format_json(findings, PROJECT_ROOT))
    elif args.format == "sarif":
        print(format_sarif(findings, PROJECT_ROOT, guard))
    elif findings:
        print(format_text(findings, PROJECT_ROOT, guard))
    else:
        print(f"{guard} passed.")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
