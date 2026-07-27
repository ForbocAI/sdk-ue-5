#!/usr/bin/env python3
"""Generate target-specific C++ authored-value macros from canonical JSON."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import re
import sys
from typing import Any

from ue_targets import UeTarget, ue_targets


SCHEMA_VERSION = "1.0.0"
MANIFEST_RELATIVE_PATH = Path("Content/Data/cpp-authored-values.json")
SHARD_RELATIVE_DIRECTORY = Path("Content/Data/cpp-authored-values")
SHARD_SIZE = 180
SOURCE_SUFFIXES = frozenset({".h", ".hpp", ".cpp"})


@dataclass(frozen=True)
class TargetPlan:
    label: str
    root: Path
    manifest: Path
    shard_directory: Path
    output: Path
    legacy_outputs: tuple[Path, ...]
    include: str
    prefix: str


def identifier(value: str) -> str:
    """Return an uppercase identifier atom without standalone numeric tokens."""
    normalized = re.sub(r"[^A-Za-z0-9]+", "", value).upper()
    return normalized or "TARGET"


def target_plan(target: UeTarget) -> TargetPlan:
    """Resolve canonical data, generated output, include, and macro ownership."""
    if target.kind == "sdk":
        output_relative = Path(
            "Source/ForbocAI_SDK/Public/Components/AuthoredValues/"
            "AuthoredValuesTypes.h"
        )
        legacy_relative = Path(
            "Source/ForbocAI_SDK/Public/Components/Generated/AuthoredValues.h"
        )
        include = "Components/AuthoredValues/AuthoredValuesTypes.h"
    elif target.kind == "sdk-cli":
        output_relative = Path(
            "Source/ForbocAI_MicroGame_CLI/Public/MicroGame/Features/"
            "Components/AuthoredValues/AuthoredValuesTypes.h"
        )
        legacy_relative = Path(
            "Source/ForbocAI_MicroGame_CLI/Public/MicroGame/Features/"
            "Components/Generated/AuthoredValues.h"
        )
        include = (
            "MicroGame/Features/Components/AuthoredValues/AuthoredValuesTypes.h"
        )
    else:
        output_relative = Path(
            "Source/Features/Components/AuthoredValues/AuthoredValuesTypes.h"
        )
        legacy_relative = Path(
            "Source/Features/Components/Generated/AuthoredValues.h"
        )
        include = "Features/Components/AuthoredValues/AuthoredValuesTypes.h"
    prefix = f"FORBOCAI_{identifier(target.label)}_AUTHORED_"
    return TargetPlan(
        label=target.label,
        root=target.root,
        manifest=target.root / MANIFEST_RELATIVE_PATH,
        shard_directory=target.root / SHARD_RELATIVE_DIRECTORY,
        output=target.root / output_relative,
        legacy_outputs=(target.root / legacy_relative,),
        include=include,
        prefix=prefix,
    )


def target_plans() -> tuple[TargetPlan, ...]:
    """Return every discovered UE target's authored-value generation plan."""
    return tuple(target_plan(target) for target in ue_targets())


def value_binding(plan: TargetPlan, kind: str, token: str) -> str:
    """Create a stable macro name from target, kind, and exact C++ token."""
    encoded = json.dumps(
        {"kind": kind, "token": token}, sort_keys=True, separators=(",", ":")
    ).encode("utf-8")
    digest = hashlib.sha256(encoded).hexdigest()[:12].upper()
    return f"{plan.prefix}{kind.upper()}V{digest}"


def read_json(path: Path) -> Any:
    """Read one UTF-8 JSON document."""
    return json.loads(path.read_text(encoding="utf-8"))


def validate_entry(plan: TargetPlan, entry: Any, location: str) -> dict[str, str]:
    """Validate and normalize one canonical authored-value entry."""
    if not isinstance(entry, dict):
        raise ValueError(f"{location} must be an object")
    kind = entry.get("kind")
    token = entry.get("token")
    binding = entry.get("binding")
    if kind not in {"string", "number"}:
        raise ValueError(f"{location}.kind must be string or number")
    if not isinstance(token, str) or not token:
        raise ValueError(f"{location}.token must be a non-empty string")
    if not isinstance(binding, str) or binding != value_binding(plan, kind, token):
        raise ValueError(f"{location}.binding is not canonical")
    return {"binding": binding, "kind": kind, "token": token}


def load_entries(plan: TargetPlan) -> list[dict[str, str]]:
    """Load and validate one target's manifest and composed value shards."""
    manifest = read_json(plan.manifest)
    if not isinstance(manifest, dict):
        raise ValueError(f"{plan.manifest}: expected an object")
    if manifest.get("schemaVersion") != SCHEMA_VERSION:
        raise ValueError(f"{plan.manifest}: invalid schemaVersion")
    if manifest.get("target") != plan.label:
        raise ValueError(f"{plan.manifest}: invalid target")
    shards = manifest.get("shards")
    if not isinstance(shards, list) or not all(
        isinstance(shard, str) and shard.endswith(".json") for shard in shards
    ):
        raise ValueError(f"{plan.manifest}: shards must be JSON paths")

    expected_paths = {plan.root / "Content/Data" / shard for shard in shards}
    actual_paths = set(plan.shard_directory.glob("*.json"))
    if actual_paths != expected_paths:
        raise ValueError(f"{plan.manifest}: shard inventory is not exact")

    entries: list[dict[str, str]] = []
    for shard_index, path in enumerate(sorted(expected_paths)):
        document = read_json(path)
        if not isinstance(document, dict):
            raise ValueError(f"{path}: expected an object")
        if document.get("schemaVersion") != SCHEMA_VERSION:
            raise ValueError(f"{path}: invalid schemaVersion")
        if document.get("target") != plan.label:
            raise ValueError(f"{path}: invalid target")
        values = document.get("values")
        if not isinstance(values, list):
            raise ValueError(f"{path}: values must be an array")
        entries.extend(
            validate_entry(plan, entry, f"shards[{shard_index}].values[{index}]")
            for index, entry in enumerate(values)
        )

    bindings = [entry["binding"] for entry in entries]
    tokens = [(entry["kind"], entry["token"]) for entry in entries]
    if len(bindings) != len(set(bindings)) or len(tokens) != len(set(tokens)):
        raise ValueError(f"{plan.manifest}: duplicate authored values")
    return sorted(entries, key=lambda entry: entry["binding"])


def render_header(plan: TargetPlan, entries: list[dict[str, str]]) -> str:
    """Render deterministic macro adapters from canonical JSON tokens."""
    manifest_name = MANIFEST_RELATIVE_PATH.as_posix()
    lines = [
        "#pragma once",
        "",
        "// Generated by scripts/generate_authored_values.py from "
        f"{manifest_name}; do not edit.",
    ]
    lines.extend(
        f"#define {entry['binding']} {entry['token']}" for entry in entries
    )
    lines.append("")
    return "\n".join(lines)


def shard_text(plan: TargetPlan, entries: list[dict[str, str]]) -> str:
    """Render one compact, line-count-safe canonical value shard."""
    lines = [
        "{",
        f'  "schemaVersion": "{SCHEMA_VERSION}",',
        f'  "target": {json.dumps(plan.label)},',
        '  "values": [',
    ]
    for index, entry in enumerate(entries):
        suffix = "," if index + 1 < len(entries) else ""
        lines.append(
            "    " + json.dumps(entry, ensure_ascii=False, sort_keys=True) + suffix
        )
    lines.extend(["  ]", "}", ""])
    return "\n".join(lines)


def write_contract(plan: TargetPlan, entries: list[dict[str, str]]) -> None:
    """Write one target's manifest, bounded shards, and generated header."""
    plan.shard_directory.mkdir(parents=True, exist_ok=True)
    for stale in plan.shard_directory.glob("*.json"):
        stale.unlink()

    shard_paths: list[str] = []
    for index, start in enumerate(range(0, len(entries), SHARD_SIZE)):
        name = f"values-{index:03d}.json"
        path = plan.shard_directory / name
        path.write_text(
            shard_text(plan, entries[start:start + SHARD_SIZE]), encoding="utf-8"
        )
        shard_paths.append(
            (SHARD_RELATIVE_DIRECTORY.relative_to("Content/Data") / name).as_posix()
        )

    plan.manifest.parent.mkdir(parents=True, exist_ok=True)
    plan.manifest.write_text(
        json.dumps(
            {
                "schemaVersion": SCHEMA_VERSION,
                "target": plan.label,
                "shards": shard_paths,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    for legacy in plan.legacy_outputs:
        if legacy.is_file():
            legacy.unlink()
    plan.output.parent.mkdir(parents=True, exist_ok=True)
    plan.output.write_text(render_header(plan, entries), encoding="utf-8")


def source_paths(plan: TargetPlan) -> list[Path]:
    """Return authored C++ source for one target, excluding generated output."""
    source_root = plan.root / "Source"
    return [
        path
        for path in sorted(source_root.rglob("*"))
        if path.is_file()
        and path.suffix in SOURCE_SUFFIXES
        and path != plan.output
        and path not in plan.legacy_outputs
    ]


def read_source(path: Path) -> str:
    """Read source without normalizing existing line endings."""
    with path.open("r", encoding="utf-8", errors="replace", newline="") as handle:
        return handle.read()


def write_source(path: Path, text: str) -> None:
    """Write rewritten source while retaining its selected newline sequence."""
    with path.open("w", encoding="utf-8", newline="") as handle:
        handle.write(text)


def scannable_source(text: str) -> str:
    """Apply the canonical guard's offset-preserving structural blanking."""
    import check_source_for_data as scanner

    return scanner.blank_unreal_reflection_metadata(
        scanner.blank_json_schema_metadata(
            scanner.blank_comments_and_chars(scanner.blank_preprocessor(text))
        )
    )


def add_generated_include(text: str, include: str) -> str:
    """Add one generated adapter include without disturbing source preambles."""
    directive = f'#include "{include}"'
    newline = "\r\n" if "\r\n" in text else "\n"
    text = re.sub(
        rf"^{re.escape(directive)}\r?\n", "", text, count=1,
        flags=re.MULTILINE,
    )
    pragma = re.search(r"^#pragma once[^\r\n]*(?:\r?\n)?", text, re.MULTILINE)
    if pragma is not None:
        return text[:pragma.end()] + directive + newline + text[pragma.end():]
    first_include = re.search(r"^#include [^\r\n]*(?:\r?\n)?", text, re.MULTILINE)
    if first_include is not None:
        return text[:first_include.end()] + directive + newline + text[first_include.end():]
    return directive + newline + text


def migrate_plan(plan: TargetPlan) -> int:
    """Replace semantic source literals with canonical generated macro bindings."""
    import check_source_for_data as scanner

    existing = load_entries(plan) if plan.manifest.is_file() else []
    entries = {(entry["kind"], entry["token"]): entry for entry in existing}
    changed_files = 0
    for path in source_paths(plan):
        original = read_source(path)
        matches = scanner.literal_matches(
            scannable_source(original), include_metadata=True
        )
        if not matches:
            directive = f'#include "{plan.include}"'
            if directive in original:
                rewritten = add_generated_include(original, plan.include)
                if rewritten != original:
                    write_source(path, rewritten)
                    changed_files += 1
            continue
        rewritten = original
        for start, end, kind, token in reversed(matches):
            key = (kind, token)
            entry = entries.setdefault(
                key,
                {
                    "binding": value_binding(plan, kind, token),
                    "kind": kind,
                    "token": token,
                },
            )
            rewritten = rewritten[:start] + entry["binding"] + rewritten[end:]
        rewritten = add_generated_include(rewritten, plan.include)
        write_source(path, rewritten)
        changed_files += 1

    rewritten_source = "\n".join(read_source(path) for path in source_paths(plan))
    used_entries = [
        entry for entry in entries.values()
        if entry["binding"] in rewritten_source
    ]
    write_contract(plan, sorted(used_entries, key=lambda entry: entry["binding"]))
    return changed_files


def validate_plan(plan: TargetPlan) -> list[str]:
    """Return synchronization and unused-binding failures for one target."""
    try:
        entries = load_entries(plan)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        return [str(error)]
    expected = render_header(plan, entries)
    if not plan.output.is_file() or plan.output.read_text(encoding="utf-8") != expected:
        return [f"{plan.output}: generated authored values are stale"]

    source = "\n".join(read_source(path) for path in source_paths(plan))
    unused = [entry["binding"] for entry in entries if entry["binding"] not in source]
    return [f"{plan.manifest}: unused binding {binding}" for binding in unused]


def validate_contracts() -> list[str]:
    """Return all canonical JSON, generated output, and usage failures."""
    return [failure for plan in target_plans() for failure in validate_plan(plan)]


def generated_output_paths() -> frozenset[Path]:
    """Return generated headers excluded from authored source scans."""
    return frozenset(plan.output for plan in target_plans())


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--migrate", action="store_true")
    parser.add_argument("--write", action="store_true")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if args.migrate:
        for plan in target_plans():
            changed = migrate_plan(plan)
            print(f"Migrated {changed} {plan.label} source file(s) to canonical JSON.")
    elif args.write:
        for plan in target_plans():
            write_contract(plan, load_entries(plan))
            print(f"Generated {plan.output}.")

    failures = validate_contracts()
    if failures:
        print("Authored-value generation failed:")
        for failure in failures:
            print(f"- {failure}")
        return 1
    print("Generated C++ authored values are synchronized with canonical JSON.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
