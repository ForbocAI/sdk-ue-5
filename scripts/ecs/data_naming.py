#!/usr/bin/env python3
"""ECS-shaped authored-data naming guard.

JSON paths read like ECS domain paths: broad catalog, concrete instance,
subdomain, then a small local leaf. Violations are derived from path and object
shape, and the ECS section vocabulary is sourced from the SDK-owned Core/ecs.hpp
(via check_ecs) rather than a hand-maintained copy, so the doctrine tracks the
ECS core.

Report-only: it names the constructive target shape and never edits files.
The production CLI scans the configured project root's Content/Data; path
arguments are intentionally unsupported.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

# The ECS engine (check_ecs) lives one level up in scripts/; put it on the path.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from check_ecs import (
    Finding,
    Rule,
    Severity,
    ecs_section_keys,
    explain,
    format_json,
    format_sarif,
    format_text,
    has_ignored_part,
    iter_files,
    normalize,
    pluralize,
    register,
    rel,
    PROJECT_ROOT,
)
from ue_targets import ue_targets


CONTENT_DATA_ROOT = PROJECT_ROOT / "Content" / "Data"
CONTENT_DATA_ROOTS = tuple(
    path for path in (target.root / "Content" / "Data" for target in ue_targets()) if path.exists()
) or tuple(path for path in (CONTENT_DATA_ROOT,) if path.exists())
LARGE_JSON_LINE_COUNT = 500
WIDE_SECTION_COUNT = 4
INSTANCE_CATALOG_ATOM = "level"


CATALOG_ORDER = Rule(
    id="DATA-NAME-001",
    severity=Severity.HIGH,
    summary="ECS data path puts the concrete location before its catalog",
    guidance="Order the path catalog-then-instance (levels/french_gulch/...), matching the ecs.hpp domain-path order.",
    skill="ecs.hpp doctrine: broad catalog, concrete instance, subdomain, leaf",
)
COMPOUND_LEAF = Rule(
    id="DATA-NAME-002",
    severity=Severity.MEDIUM,
    summary="JSON filename repeats folderable ECS path tokens",
    guidance="Fold the repeated leading token into a folder so the leaf is a small local name.",
    skill="ecs.hpp doctrine: small local leaf under domain folders",
)
ROOT_REWRAP = Rule(
    id="DATA-NAME-003",
    severity=Severity.MEDIUM,
    summary="JSON root key re-wraps a domain already named by the path",
    guidance="Drop the redundant root wrapper; the path already names the domain. Split local sections into leaf files.",
    skill="check_data_naming doctrine: the path is the domain, not a repeated root key",
)
WIDE_SECTIONS = Rule(
    id="DATA-NAME-004",
    severity=Severity.HIGH,
    summary="JSON mixes multiple ECS registry sections in one file",
    guidance="Keep each ECS registry section (components/systems/resources/events/...) in its own folder-local file and compose them through one registry fold.",
    skill="ecs.hpp: FDomainRegistry sections are composed by a fold, not authored in one bag",
)
LOCATION_BAG = Rule(
    id="DATA-NAME-005",
    severity=Severity.MEDIUM,
    summary="location-scoped file holds a wide reusable-definition bag",
    guidance="Split reusable definitions into shared catalogs; keep the location leaf to placement/override rows.",
    skill="ecs.hpp doctrine: reusable definitions in catalogs, instances hold overrides",
)
LARGE_SPLIT = Rule(
    id="DATA-NAME-006",
    severity=Severity.MEDIUM,
    summary="large JSON file bundles split-ready data sections",
    guidance="Move the split-ready sections into folder-local files and compose them through ECS domain folds.",
    skill="ecs.hpp doctrine: fold folder-local leaves instead of one wide file",
)
UNPARSEABLE = Rule(
    id="DATA-NAME-007",
    severity=Severity.HIGH,
    summary="authored JSON does not parse",
    guidance="Fix the JSON syntax; an unparseable data file is silently skipped by adapters and every other data rule.",
    skill="authored data must load through the runtime settings adapters",
)
SECTION_FOLDER = Rule(
    id="DATA-NAME-008",
    severity=Severity.MEDIUM,
    summary="file under an ECS-section folder declares a different ECS section",
    guidance="Move each section's rows under the folder named for that section so authored data mirrors the ecs.hpp registry layout.",
    skill="ecs.hpp: EDomainKind + FDomainRegistry section ownership",
)
DEEP_WRAP = Rule(
    id="DATA-NAME-009",
    severity=Severity.LOW,
    summary="JSON is wrapped in a chain of single-key objects",
    guidance="Collapse the redundant single-key nesting; each object level should add real structure.",
    skill="ecs.hpp doctrine: the path carries domain depth, not nested wrappers",
)

for _rule in (
    CATALOG_ORDER, COMPOUND_LEAF, ROOT_REWRAP, WIDE_SECTIONS, LOCATION_BAG,
    LARGE_SPLIT, UNPARSEABLE, SECTION_FOLDER, DEEP_WRAP,
):
    register(_rule)


def _finding(path: Path, rule: Rule, detail: str) -> Finding:
    return Finding(path, 1, rule.id, rule.severity, f"{rule.summary}. {detail}")


# --- Path shape ------------------------------------------------------------

def content_data_root_for(path: Path) -> Path | None:
    return next((root for root in CONTENT_DATA_ROOTS if path.is_relative_to(root)), None)


def path_tokens(path: Path) -> list[str]:
    root = content_data_root_for(path)
    if root is None:
        return []
    tokens: list[str] = []
    for part in path.relative_to(root).with_suffix("").parts:
        tokens.extend(token for token in normalize(part).split("_") if token)
    return tokens


def infer_location_roots(files: list[Path]) -> frozenset[str]:
    roots: set[str] = set()
    for path in files:
        root = content_data_root_for(path)
        if root is None:
            continue
        parts = path.relative_to(root).parts
        if len(parts) >= 3 and parts[1] == INSTANCE_CATALOG_ATOM:
            roots.add(parts[0])
    return frozenset(roots)


def catalog_instance_expected(path: Path, location_roots: frozenset[str]) -> Path | None:
    root = content_data_root_for(path)
    if root is None:
        return None
    parts = path.relative_to(root).parts
    if len(parts) < 2:
        return None
    instance, *rest = parts
    if instance not in location_roots:
        return None
    if rest and rest[0] == INSTANCE_CATALOG_ATOM:
        rest = rest[1:]
    if not rest:
        return None
    return root / pluralize(INSTANCE_CATALOG_ATOM) / instance / Path(*rest)


def leaf_split(path: Path) -> Path | None:
    tokens = [token for token in path.stem.split("_") if token]
    if len(tokens) <= 1:
        return None
    if path.parent.name == tokens[0]:
        return path.parent / ("_".join(tokens[1:]) + path.suffix)
    return path.parent / tokens[0] / ("_".join(tokens[1:]) + path.suffix)


def local_split_base(path: Path, location_roots: frozenset[str], root_key: str | None = None) -> Path:
    ordered = catalog_instance_expected(path, location_roots) or path
    base = ordered.with_suffix("")
    if root_key is not None and normalize(base.name) != normalize(root_key):
        return base / root_key
    return base


def split_examples(base: Path, keys: list[str]) -> str:
    return ", ".join(rel(base / f"{key}.json") for key in keys[:4])


def is_json_path(value: object) -> bool:
    return isinstance(value, str) and value.endswith(".json")


def is_json_path_manifest(value: object) -> bool:
    if isinstance(value, dict) and value:
        return all(is_json_path_manifest(child) for child in value.values())
    if isinstance(value, list) and value:
        return all(is_json_path(child) for child in value)
    return is_json_path(value)


def line_count(path: Path) -> int:
    return path.read_text(encoding="utf-8", errors="replace").count("\n") + 1


# --- Rules -----------------------------------------------------------------

def catalog_order_finding(path: Path, location_roots: frozenset[str]) -> Finding | None:
    expected = catalog_instance_expected(path, location_roots)
    if expected is None or expected == path:
        return None
    return _finding(path, CATALOG_ORDER, f"Expected `{rel(expected)}`.")


def compound_leaf_finding(path: Path, location_roots: frozenset[str]) -> Finding | None:
    if content_data_root_for(path) is None:
        return None
    ordered = catalog_instance_expected(path, location_roots) or path
    expected = leaf_split(ordered)
    if expected is None or expected == path:
        return None
    return _finding(path, COMPOUND_LEAF, f"Expected `{rel(expected)}`.")


def root_rewrap_finding(path: Path, value: object, location_roots: frozenset[str]) -> Finding | None:
    if content_data_root_for(path) is None:
        return None
    if not isinstance(value, dict) or len(value) != 1 or is_json_path_manifest(value):
        return None
    root_key, root_value = next(iter(value.items()))
    root_name = normalize(root_key)
    if root_name != normalize(path.stem) and root_name not in path_tokens(path):
        return None
    base = local_split_base(path, location_roots, root_key)
    if isinstance(root_value, dict):
        section_keys = [k for k, v in root_value.items() if isinstance(v, (dict, list))]
        if not section_keys:
            return None
        detail = f"Split local sections into leaf files such as {split_examples(base, section_keys)}."
    elif isinstance(root_value, list):
        detail = f"Put the array at the leaf or split repeated groups under {rel(base)}/."
    else:
        return None
    return _finding(path, ROOT_REWRAP, f"Root `{root_key}` repeats the path. {detail}")


def wide_sections_finding(path: Path, value: object, location_roots: frozenset[str]) -> Finding | None:
    if content_data_root_for(path) is None:
        return None
    if not isinstance(value, dict) or is_json_path_manifest(value):
        return None
    sections = ecs_section_keys()
    ecs_keys = [k for k, v in value.items() if normalize(k) in sections and isinstance(v, (dict, list))]
    if len(ecs_keys) < 2:
        return None
    base = local_split_base(path, location_roots)
    return _finding(path, WIDE_SECTIONS, f"Sections ({', '.join(ecs_keys)}) -> {split_examples(base, ecs_keys)}.")


def location_bag_finding(path: Path, value: object, location_roots: frozenset[str]) -> Finding | None:
    root = content_data_root_for(path)
    if root is None:
        return None
    parts = path.relative_to(root).parts
    if len(parts) < 2 or parts[0] not in location_roots:
        return None
    if not isinstance(value, dict) or len(value) != 1:
        return None
    rows = next(iter(value.values()))
    if not isinstance(rows, list) or not any(isinstance(row, dict) for row in rows):
        return None
    return _finding(path, LOCATION_BAG, "Split reusable definitions into shared catalogs; keep this leaf to placement/override rows.")


def large_split_finding(path: Path, value: object, location_roots: frozenset[str]) -> Finding | None:
    if content_data_root_for(path) is None or line_count(path) < LARGE_JSON_LINE_COUNT:
        return None
    if not isinstance(value, dict) or is_json_path_manifest(value):
        return None
    if len(value) == 1:
        root_key, root_value = next(iter(value.items()))
        if isinstance(root_value, dict):
            array_keys = [k for k, v in root_value.items() if isinstance(v, list)]
            if array_keys:
                examples = split_examples(local_split_base(path, location_roots, root_key), array_keys)
                return _finding(path, LARGE_SPLIT, f"Root `{root_key}` arrays ({', '.join(array_keys)}) -> {examples}.")
    sections = [k for k, v in value.items() if isinstance(v, (dict, list))]
    if len(sections) >= WIDE_SECTION_COUNT:
        examples = split_examples(local_split_base(path, location_roots), sections)
        return _finding(path, LARGE_SPLIT, f"{len(sections)} sections -> {examples}.")
    return None


def section_folder_finding(path: Path, value: object) -> Finding | None:
    """A file inside a folder named for an ECS section must not declare a
    different ECS section at its top level."""
    if content_data_root_for(path) is None or not isinstance(value, dict):
        return None
    if is_json_path_manifest(value):
        return None
    sections = ecs_section_keys()
    folder = normalize(path.parent.name)
    if folder not in sections:
        return None
    mismatched = [k for k, v in value.items() if normalize(k) in sections and normalize(k) != folder and isinstance(v, (dict, list))]
    if not mismatched:
        return None
    return _finding(path, SECTION_FOLDER, f"Under `{path.parent.name}/` but declares {', '.join(mismatched)}; move each under its own section folder.")


def deep_wrap_finding(path: Path, value: object) -> Finding | None:
    if not isinstance(value, dict) or len(value) != 1 or is_json_path_manifest(value):
        return None
    depth = 0
    node: object = value
    while isinstance(node, dict) and len(node) == 1:
        depth += 1
        node = next(iter(node.values()))
    # A `{domain: {section: content}}` settings shape is fine; only flag a chain
    # deep enough (3+) to be genuinely redundant nesting.
    if depth < 3:
        return None
    return _finding(path, DEEP_WRAP, f"{depth} single-key wrapper levels; collapse the redundant nesting.")


# --- Runner ----------------------------------------------------------------

def load_json(path: Path) -> tuple[object, str | None]:
    try:
        return json.loads(path.read_text(encoding="utf-8")), None
    except (json.JSONDecodeError, UnicodeDecodeError) as error:
        return None, str(error)


def find_findings() -> list[Finding]:
    files = iter_files(list(CONTENT_DATA_ROOTS), {".json"})
    location_roots = infer_location_roots(files)
    findings: list[Finding] = []
    for path in files:
        value, error = load_json(path)
        if error is not None:
            findings.append(_finding(path, UNPARSEABLE, error.splitlines()[0]))
            continue
        for finding in (
            catalog_order_finding(path, location_roots),
            compound_leaf_finding(path, location_roots),
            root_rewrap_finding(path, value, location_roots),
            wide_sections_finding(path, value, location_roots),
            location_bag_finding(path, value, location_roots),
            large_split_finding(path, value, location_roots),
            section_folder_finding(path, value),
            deep_wrap_finding(path, value),
        ):
            if finding is not None:
                findings.append(finding)
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
    guard = "Authored-data naming guard"
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
