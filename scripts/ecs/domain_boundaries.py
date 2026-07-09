#!/usr/bin/env python3
"""Domain/subdomain ownership guard for file paths.

Boundaries are derived from the repository shape, not a managed list:

* source files put broad domain tokens in folders and keep the shortest needed
  folder-qualified role leaf;
* view files put broad subject tokens in folders and keep the shortest needed
  folder-qualified View leaf;
* data files fold repeated prefix groups (and ECS section/kind prefixes) into
  folders;
* source/view folders are real domain atoms, never role buckets
  (Slice/Thunks/Selectors/etc.), implementation buckets
  (Factories/Reducers), or vague buckets (Core/Runtime/Common/Shared/etc.);
* source/view splits stay in checked role/domain leaves, never .inl/.inc/.ipp
  fragments;
* a folder never repeats its parent.

This guard is report-only. It computes the constructive target path for every
violation and tells the developer the move to make -- it never renames files or
rewrites references itself. The production CLI scans the configured project
root's Source/Features, Source/Views, and Content/Data; path arguments are
intentionally unsupported.
"""

from __future__ import annotations

import argparse
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
import sys

# The ECS engine (check_ecs) lives one level up in scripts/; put it on the path.
sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from check_ecs import (
    Finding,
    Rule,
    Severity,
    camel_tokens,
    ecs_domain_kinds,
    ecs_section_keys,
    explain,
    folder_tokens,
    format_json,
    format_sarif,
    format_summary,
    format_text,
    iter_files,
    normalize_token,
    normalized_tokens,
    register,
    rel,
    PROJECT_ROOT,
)
from ue_targets import ue_targets


SOURCE_FEATURES_ROOT = PROJECT_ROOT / "Source" / "Features"
SOURCE_VIEWS_ROOT = PROJECT_ROOT / "Source" / "Views"
CONTENT_DATA_ROOT = PROJECT_ROOT / "Content" / "Data"


def _existing(paths: list[Path]) -> tuple[Path, ...]:
    return tuple(path for path in paths if path.exists())


def _target_feature_roots() -> tuple[Path, ...]:
    roots: list[Path] = []
    for target in ue_targets():
        if target.label == "sdk":
            roots.extend(
                [
                    target.root / "Source" / "ForbocAI_SDK" / "Public" / "Features",
                    target.root / "Source" / "ForbocAI_SDK" / "Private" / "Features",
                ]
            )
        elif target.label == "sdk-cli":
            roots.append(target.root / "Source" / "ForbocAI_TestGame_CLI" / "Public" / "TestGame" / "Features")
        else:
            roots.append(target.root / "Source" / "Features")
    return _existing(roots) or _existing([SOURCE_FEATURES_ROOT])


def _target_view_roots() -> tuple[Path, ...]:
    roots: list[Path] = []
    for target in ue_targets():
        if target.label == "sdk-cli":
            roots.append(target.root / "Source" / "ForbocAI_TestGame_CLI" / "Public" / "TestGame" / "Views")
        elif target.label == "demo":
            roots.append(target.root / "Source" / "Views")
    return _existing(roots) or _existing([SOURCE_VIEWS_ROOT])


def _target_data_roots() -> tuple[Path, ...]:
    return _existing([target.root / "Content" / "Data" for target in ue_targets()]) or _existing([CONTENT_DATA_ROOT])


SOURCE_FEATURES_ROOTS = _target_feature_roots()
SOURCE_VIEWS_ROOTS = _target_view_roots()
CONTENT_DATA_ROOTS = _target_data_roots()


def _root_for(path: Path, roots: tuple[Path, ...]) -> Path | None:
    return next((root for root in roots if path.is_relative_to(root)), None)

SOURCE_SUFFIXES = {".cpp", ".h", ".hpp"}
SOURCE_FRAGMENT_SUFFIXES = {".inl", ".inc", ".ipp"}
CANONICAL_SOURCE_ROLES = (
    ("Actions",),
    ("Adapters",),
    ("Listeners",),
    ("Selectors",),
    ("Slice",),
    ("Thunks",),
    ("Types",),
)
ROLE_FOLDER_ATOMS = frozenset({
    "action",
    "adapter",
    "listener",
    "selector",
    "slice",
    "thunk",
    "type",
    "view",
})
IMPLEMENTATION_BUCKET_ATOMS = frozenset({
    "factory",
    "reducer",
})
VAGUE_FOLDER_ATOMS = frozenset({
    "base",
    "common",
    "core",
    "default",
    "fragment",
    "general",
    "generic",
    "helper",
    "implementation",
    "impl",
    "internal",
    "macro",
    "misc",
    "miscellaneous",
    "runtime",
    "setup",
    "shared",
    "support",
    "util",
    "utility",
})
DATA_PREFIX_MIN_COUNT = 4
VIEW_QUALIFIER_MIN_COUNT = 3

_SELF_MOVE_NOTE = (
    " Rename/move the file to that path and update its references yourself; this "
    "guard reports the target and never edits files. Never add suppression "
    "comments to bypass the guard."
)

SRC_REPEAT = register(
    Rule(
        id="DOMAIN-SRC-001",
        severity=Severity.HIGH,
        summary="source filename repeats domain/subdomain tokens already in its folders",
        guidance=(
            "Move broad domain/subdomain words into folders and keep the "
            "shortest collision-free real-domain qualifier plus the role "
            "(Rendering/Sky/SkyThunks.h). Drill into descriptive subdomains; "
            "do not use role folders, vague buckets, digits, or .inl fragments."
        ) + _SELF_MOVE_NOTE,
        skill="ecs.hpp doctrine: domain words live in folders; the leaf is a small role name",
    )
)

SOURCE_FRAGMENT = register(
    Rule(
        id="DOMAIN-SRC-002",
        severity=Severity.HIGH,
        summary="source fragment bypasses checked domain leaf structure",
        guidance=(
            "Never split Source into .inl/.inc/.ipp or other non-role "
            "implementation fragments. Drill into real descriptive subdomain "
            "folders and keep code in checked .h/.hpp/.cpp role/domain leaves."
        ) + _SELF_MOVE_NOTE,
        skill="ecs.hpp doctrine: implementation splits stay visible as domain role leaves",
    )
)

VIEW_REPEAT = register(
    Rule(
        id="DOMAIN-VIEW-001",
        severity=Severity.HIGH,
        summary="view filename repeats subject tokens shared across views",
        guidance=(
            "Move broad subject words into folders and keep the shortest "
            "collision-free real-domain qualifier plus View "
            "(Player/Controller/ControllerView.cpp). Drill into descriptive "
            "view subdomains; do not use .inl fragments, Support/Setup leaves, "
            "role folders, vague buckets, or digits."
        ) + _SELF_MOVE_NOTE,
        skill="ecs.hpp doctrine: subject in folders, View leaf",
    )
)

DATA_REPEAT = register(
    Rule(
        id="DOMAIN-DATA-001",
        severity=Severity.HIGH,
        summary="data filename repeats folderable prefix tokens",
        guidance="Fold the repeated prefix into a folder (french_gulch/roads/residential.json)." + _SELF_MOVE_NOTE,
        skill="check_data_naming doctrine: catalog/instance/subdomain/leaf",
    )
)

DATA_ECS_PREFIX = register(
    Rule(
        id="DOMAIN-DATA-002",
        severity=Severity.MEDIUM,
        summary="flat data filename leads with an ECS section/kind that should be its folder",
        guidance="Put the file under a folder named for the ECS registry section or domain kind (components/..., systems/...) so authored data mirrors the ecs.hpp registry." + _SELF_MOVE_NOTE,
        skill="ecs.hpp: EDomainKind + FDomainRegistry sections",
    )
)

FOLDER_REDUNDANT = register(
    Rule(
        id="DOMAIN-FOLDER-001",
        severity=Severity.MEDIUM,
        summary="folder segment repeats its parent segment",
        guidance="Collapse the redundant nesting; each path segment should add a new domain word, not repeat its parent." + _SELF_MOVE_NOTE,
        skill="ecs.hpp doctrine: every path segment is a distinct domain step",
    )
)

FOLDER_ROLE_BUCKET = register(
    Rule(
        id="DOMAIN-FOLDER-002",
        severity=Severity.HIGH,
        summary="folder segment is an implementation role bucket, not a domain",
        guidance=(
            "Do not create folders named for roles such as Actions, Adapters, "
            "Listeners, Selectors, Slice, Thunks, Types, or View. "
            "Choose the real domain/subdomain atom, then keep the role as the "
            "file leaf suffix."
        ) + _SELF_MOVE_NOTE,
        skill="ecs.hpp doctrine: folders are domains; role names belong on leaves",
    )
)

FOLDER_IMPLEMENTATION_BUCKET = register(
    Rule(
        id="DOMAIN-FOLDER-004",
        severity=Severity.HIGH,
        summary="folder segment is an implementation bucket, not a domain",
        guidance=(
            "Do not create Factories/Reducers folders or leaves. They are "
            "implementation labels, not ECS domain atoms. Drill into the "
            "concrete domain/subdomain the code owns and keep the leaf named "
            "for that local domain responsibility."
        ) + _SELF_MOVE_NOTE,
        skill="ecs.hpp doctrine: folders are concrete domain atoms, not implementation buckets",
    )
)

FOLDER_VAGUE_BUCKET = register(
    Rule(
        id="DOMAIN-FOLDER-003",
        severity=Severity.HIGH,
        summary="folder segment is a vague bucket instead of a domain atom",
        guidance=(
            "Do not use vague folder atoms such as Core, Runtime, Common, "
            "Shared, Support, Setup, Util, or numeric/version buckets. Name the "
            "specific domain/subdomain the code owns."
        ) + _SELF_MOVE_NOTE,
        skill="ecs.hpp doctrine: every folder is a concrete domain step",
    )
)


def _move(path: Path, expected: Path, rule: Rule) -> Finding:
    return Finding(path, 1, rule.id, rule.severity, f"{rule.summary}; expected `{rel(expected)}`")


# --- Source leaves ---------------------------------------------------------

@dataclass(frozen=True)
class RoleNameContext:
    expected_stem_by_path: dict[Path, str]

def _split_source_role(tokens: list[str]) -> tuple[list[str], list[str]]:
    for role in CANONICAL_SOURCE_ROLES:
        if len(tokens) >= len(role) and tuple(tokens[-len(role):]) == role:
            return tokens[: -len(role)], list(role)
    return tokens, []


def _source_ancestor_tokens(path: Path) -> set[str]:
    root = _root_for(path, SOURCE_FEATURES_ROOTS)
    if root is None:
        return set()
    tokens: list[str] = []
    for part in path.relative_to(root).parent.parts:
        tokens.extend(folder_tokens(part))
    return set(normalized_tokens(tokens))


def _source_stems(path: Path, role_tokens: list[str]) -> list[str]:
    root = _root_for(path, SOURCE_FEATURES_ROOTS)
    if root is None:
        return []
    parts = path.relative_to(root).parent.parts
    return [
        "".join(
            token
            for part in parts[index:]
            for token in camel_tokens(part)
        )
        + "".join(role_tokens)
        for index in range(len(parts) - 1, -1, -1)
    ]


def _role_name_context(
    paths: list[Path],
    root: Path,
    role_suffixes: tuple[tuple[str, ...], ...],
    stem_candidates,
) -> RoleNameContext:
    items: list[dict] = []
    for path in paths:
        if path.suffix not in SOURCE_SUFFIXES or not path.is_relative_to(root):
            continue
        tokens = camel_tokens(path.stem)
        for role_tokens in role_suffixes:
            if len(tokens) >= len(role_tokens) and tuple(tokens[-len(role_tokens):]) == role_tokens:
                candidates = stem_candidates(path, list(role_tokens))
                if candidates:
                    items.append({"path": path, "candidates": candidates, "index": 0})
                break

    while True:
        groups: dict[tuple[str, str], list[dict]] = {}
        for item in items:
            key = (item["path"].suffix, item["candidates"][item["index"]])
            groups.setdefault(key, []).append(item)

        changed = False
        for group in groups.values():
            if len(group) <= 1:
                continue
            for item in group:
                if item["index"] < len(item["candidates"]) - 1:
                    item["index"] += 1
                    changed = True
        if not changed:
            break

    return RoleNameContext(
        expected_stem_by_path={item["path"]: item["candidates"][item["index"]] for item in items}
    )


def source_role_context(paths: list[Path]) -> RoleNameContext:
    contexts = [
        _role_name_context(paths, root, CANONICAL_SOURCE_ROLES, _source_stems)
        for root in SOURCE_FEATURES_ROOTS
    ]
    merged: dict[Path, str] = {}
    for context in contexts:
        merged.update(context.expected_stem_by_path)
    return RoleNameContext(merged)


def source_finding(path: Path, context: RoleNameContext | None = None) -> Finding | None:
    if path.suffix not in SOURCE_SUFFIXES or _root_for(path, SOURCE_FEATURES_ROOTS) is None:
        return None
    tokens = camel_tokens(path.stem)
    prefix_tokens, role_tokens = _split_source_role(tokens)
    if not role_tokens:
        return None
    expected_stem = (
        context.expected_stem_by_path.get(path)
        if context is not None
        else _source_stems(path, role_tokens)[0]
    )
    if path.stem == expected_stem:
        return None
    expected = path.parent / (expected_stem + path.suffix)
    return None if expected == path else _move(path, expected, SRC_REPEAT)


# --- View subjects ---------------------------------------------------------

def _view_subject_tokens(path: Path) -> list[str]:
    tokens = camel_tokens(path.stem)
    return tokens[:-1] if tokens and tokens[-1] == "View" else []


def _view_stems(path: Path) -> list[str]:
    root = _root_for(path, SOURCE_VIEWS_ROOTS)
    if root is None or path.parent == root:
        return []
    parts = path.relative_to(root).parent.parts
    return [
        "".join(
            token
            for part in parts[index:]
            for token in camel_tokens(part)
        )
        + "View"
        for index in range(len(parts) - 1, -1, -1)
    ]


def view_role_context(paths: list[Path]) -> RoleNameContext:
    contexts = [
        _role_name_context(
            paths,
            root,
            (("View",),),
            lambda path, _role_tokens: _view_stems(path),
        )
        for root in SOURCE_VIEWS_ROOTS
    ]
    merged: dict[Path, str] = {}
    for context in contexts:
        merged.update(context.expected_stem_by_path)
    return RoleNameContext(merged)


def view_qualifier_tokens(paths: list[Path]) -> set[str]:
    counts: Counter[str] = Counter()
    seen: set[str] = set()
    for path in paths:
        if path.suffix not in SOURCE_SUFFIXES or _root_for(path, SOURCE_VIEWS_ROOTS) is None:
            continue
        if path.stem in seen:
            continue
        seen.add(path.stem)
        for token in _view_subject_tokens(path):
            counts[normalize_token(token)] += 1
    return {token for token, count in counts.items() if count >= VIEW_QUALIFIER_MIN_COUNT}


def view_finding(
    path: Path,
    qualifiers: set[str],
    context: RoleNameContext | None = None,
) -> Finding | None:
    root = _root_for(path, SOURCE_VIEWS_ROOTS)
    if path.suffix not in SOURCE_SUFFIXES or root is None:
        return None
    expected_stem = (
        context.expected_stem_by_path.get(path)
        if context is not None
        else (_view_stems(path)[0] if _view_stems(path) else None)
    )
    if expected_stem is not None:
        if path.stem == expected_stem:
            return None
        expected = path.parent / (expected_stem + path.suffix)
        return None if expected == path else _move(path, expected, VIEW_REPEAT)
    subject = [t for t in _view_subject_tokens(path) if normalize_token(t) not in qualifiers]
    if not subject:
        return None
    expected_parent = root
    for token in subject:
        expected_parent /= token
    expected = expected_parent / ("View" + path.suffix)
    return None if expected == path else _move(path, expected, VIEW_REPEAT)


# --- Data filenames --------------------------------------------------------

@dataclass(frozen=True)
class DataContext:
    prefix_counts: Counter[tuple[str, ...]]
    compound_roots: frozenset[tuple[Path, tuple[str, str]]]


def _data_root_for(path: Path) -> Path | None:
    data_root = _root_for(path, CONTENT_DATA_ROOTS)
    if data_root is None:
        return None
    if path.parent == data_root:
        return data_root
    source_root = data_root / "Source"
    return source_root if path.parent == source_root else None


def _data_names(paths: list[Path]) -> list[tuple[Path, Path, tuple[str, ...]]]:
    names = []
    for path in paths:
        data_root = _root_for(path, CONTENT_DATA_ROOTS)
        if path.suffix != ".json" or data_root is None:
            continue
        root = _data_root_for(path)
        if root is None:
            continue
        tokens = tuple(token for token in path.stem.split("_") if token)
        if len(tokens) > 1:
            names.append((path, root, tokens))
    return names


def _data_prefixes(tokens: tuple[str, ...]) -> list[tuple[str, ...]]:
    return [tokens[:index] for index in range(1, len(tokens))]


def data_context(paths: list[Path]) -> DataContext:
    names = _data_names(paths)
    first_token_files = {
        (root, path.stem)
        for path in paths
        if path.suffix == ".json"
        for root in [_data_root_for(path)]
        if root is not None and path.parent == root
    }
    pair_counts: Counter[tuple[Path, tuple[str, str]]] = Counter()
    for _path, root, tokens in names:
        if len(tokens) >= 2:
            pair_counts[(root, (tokens[0], tokens[1]))] += 1
    compound_roots = frozenset(
        pair
        for pair, count in pair_counts.items()
        if count >= DATA_PREFIX_MIN_COUNT and (pair[0], pair[1][0]) not in first_token_files
    )
    prefix_counts: Counter[tuple[str, ...]] = Counter()
    for _path, root, tokens in names:
        prefix_counts.update(_data_prefixes(_normalized_data_tokens(root, tokens, compound_roots)))
    return DataContext(prefix_counts=prefix_counts, compound_roots=compound_roots)


def _normalized_data_tokens(
    root: Path, tokens: tuple[str, ...], compound_roots: frozenset[tuple[Path, tuple[str, str]]]
) -> tuple[str, ...]:
    if len(tokens) >= 2 and (root, (tokens[0], tokens[1])) in compound_roots:
        return ("_".join(tokens[:2]), *tokens[2:])
    return tokens


def _best_data_prefix(tokens: tuple[str, ...], context: DataContext) -> tuple[str, ...]:
    candidates = [
        prefix for prefix in _data_prefixes(tokens)
        if context.prefix_counts[prefix] >= DATA_PREFIX_MIN_COUNT
    ]
    if candidates:
        return max(candidates, key=len)
    return tokens[:1] if len(tokens) <= 2 else tokens[:-2]


def data_ecs_finding(path: Path) -> Finding | None:
    """A flat data file whose leading token is an ECS section/kind belongs in
    that folder regardless of how many siblings share the prefix."""
    if path.suffix != ".json":
        return None
    root = _data_root_for(path)
    if root is None:
        return None
    tokens = tuple(token for token in path.stem.split("_") if token)
    if len(tokens) <= 1:
        return None
    ecs_words = ecs_section_keys() | {normalize_token(k) for k in ecs_domain_kinds()}
    if normalize_token(tokens[0]) not in ecs_words and tokens[0] not in ecs_words:
        return None
    expected = root / tokens[0] / ("_".join(tokens[1:]) + ".json")
    return None if expected == path else _move(path, expected, DATA_ECS_PREFIX)


def data_freq_finding(path: Path, context: DataContext) -> Finding | None:
    if path.suffix != ".json":
        return None
    root = _data_root_for(path)
    if root is None:
        return None
    tokens = tuple(token for token in path.stem.split("_") if token)
    if len(tokens) <= 1:
        return None
    normalized = _normalized_data_tokens(root, tokens, context.compound_roots)
    prefix = _best_data_prefix(normalized, context)
    leaf = normalized[len(prefix):]
    if not prefix or not leaf:
        return None
    expected = root / Path(*prefix) / ("_".join(leaf) + ".json")
    return None if expected == path else _move(path, expected, DATA_REPEAT)


# --- Redundant folders -----------------------------------------------------

def folder_redundancy_findings(paths: list[Path]) -> list[Finding]:
    seen: set[Path] = set()
    findings: list[Finding] = []
    for path in paths:
        parts = path.parent.parts
        for index in range(1, len(parts)):
            if normalize_token(parts[index]) == normalize_token(parts[index - 1]):
                folder = Path(*parts[: index + 1])
                if folder in seen:
                    continue
                seen.add(folder)
                collapsed = Path(*parts[:index], *parts[index + 1:])
                findings.append(
                    Finding(
                        folder,
                        1,
                        FOLDER_REDUNDANT.id,
                        FOLDER_REDUNDANT.severity,
                        f"{FOLDER_REDUNDANT.summary} (`{parts[index]}`); expected `{rel(collapsed)}`",
                    )
                )
    return findings


def _source_or_view_root(path: Path) -> Path | None:
    return _root_for(path, SOURCE_FEATURES_ROOTS) or _root_for(path, SOURCE_VIEWS_ROOTS)


def _source_or_view_folder_parts(path: Path) -> tuple[Path, tuple[str, ...]] | None:
    root = _source_or_view_root(path)
    if root is None:
        return None
    return root, path.relative_to(root).parent.parts


def _folder_token_atoms(part: str) -> list[str]:
    return normalized_tokens(folder_tokens(part))


def _is_numeric_or_version_atom(atom: str) -> bool:
    return atom.isdigit() or (atom.startswith("v") and atom[1:].isdigit())


def source_fragment_findings(paths: list[Path]) -> list[Finding]:
    findings: list[Finding] = []
    for path in paths:
        if path.suffix not in SOURCE_FRAGMENT_SUFFIXES or _source_or_view_root(path) is None:
            continue
        findings.append(
            Finding(
                path,
                1,
                SOURCE_FRAGMENT.id,
                SOURCE_FRAGMENT.severity,
                f"{SOURCE_FRAGMENT.summary}; suffix `{path.suffix}` hides an implementation split",
            )
        )
    return findings


def folder_domain_findings(paths: list[Path]) -> list[Finding]:
    seen: set[tuple[Path, str]] = set()
    findings: list[Finding] = []
    for path in paths:
        scoped = _source_or_view_folder_parts(path)
        if scoped is None:
            continue
        root, parts = scoped
        for index, part in enumerate(parts):
            atoms = _folder_token_atoms(part)
            if not atoms:
                continue
            folder = root / Path(*parts[: index + 1])
            if any(atom in ROLE_FOLDER_ATOMS for atom in atoms):
                key = (folder, FOLDER_ROLE_BUCKET.id)
                if key not in seen:
                    seen.add(key)
                    findings.append(
                        Finding(
                            folder,
                            1,
                            FOLDER_ROLE_BUCKET.id,
                            FOLDER_ROLE_BUCKET.severity,
                            f"{FOLDER_ROLE_BUCKET.summary} (`{part}`); use a real domain folder and keep the role on the file leaf",
                        )
                    )
            if any(atom in IMPLEMENTATION_BUCKET_ATOMS for atom in atoms):
                key = (folder, FOLDER_IMPLEMENTATION_BUCKET.id)
                if key not in seen:
                    seen.add(key)
                    findings.append(
                        Finding(
                            folder,
                            1,
                            FOLDER_IMPLEMENTATION_BUCKET.id,
                            FOLDER_IMPLEMENTATION_BUCKET.severity,
                            f"{FOLDER_IMPLEMENTATION_BUCKET.summary} (`{part}`); choose the concrete owned domain/subdomain atom and a real RTK role leaf",
                        )
                    )
            if any(atom in VAGUE_FOLDER_ATOMS or _is_numeric_or_version_atom(atom) for atom in atoms):
                key = (folder, FOLDER_VAGUE_BUCKET.id)
                if key not in seen:
                    seen.add(key)
                    findings.append(
                        Finding(
                            folder,
                            1,
                            FOLDER_VAGUE_BUCKET.id,
                            FOLDER_VAGUE_BUCKET.severity,
                            f"{FOLDER_VAGUE_BUCKET.summary} (`{part}`); choose the concrete owned domain/subdomain atom",
                        )
                    )
    return findings


# --- Runner ----------------------------------------------------------------

def find_findings() -> list[Finding]:
    candidates = iter_files([*SOURCE_FEATURES_ROOTS, *SOURCE_VIEWS_ROOTS, *CONTENT_DATA_ROOTS])
    source_context = source_role_context(candidates)
    view_context = view_role_context(candidates)
    qualifiers = view_qualifier_tokens(candidates)
    context = data_context(candidates)
    findings: list[Finding] = []
    for path in candidates:
        finding = (
            view_finding(path, qualifiers, view_context)
            or source_finding(path, source_context)
            or data_ecs_finding(path)
            or data_freq_finding(path, context)
        )
        if finding is not None:
            findings.append(finding)
    findings += folder_redundancy_findings(candidates)
    findings += source_fragment_findings(candidates)
    findings += folder_domain_findings(candidates)
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
    guard = "Domain boundary guard"
    if args.format == "json":
        print(format_json(findings, PROJECT_ROOT))
        if findings:
            print(format_summary(findings, guard), file=sys.stderr)
    elif args.format == "sarif":
        print(format_sarif(findings, PROJECT_ROOT, guard))
        if findings:
            print(format_summary(findings, guard), file=sys.stderr)
    elif findings:
        print(format_text(findings, PROJECT_ROOT, guard))
    else:
        print(f"{guard} passed.")
    return 1 if findings else 0


if __name__ == "__main__":
    raise SystemExit(main())
