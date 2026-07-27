#!/usr/bin/env python3
"""Enforce data-driven, function-aware thin CLI command surfaces."""

from __future__ import annotations

from dataclasses import dataclass
import argparse
import json
from pathlib import Path
import re
import sys


SCRIPT_DIR = Path(__file__).resolve().parent
SCRIPTS_DIR = SCRIPT_DIR.parent
PROJECT_ROOT = SCRIPTS_DIR.parent
sys.path.insert(0, str(SCRIPTS_DIR))
sys.path.insert(0, str(SCRIPTS_DIR / "docs"))

from check_fp import code_only  # noqa: E402
from cpp_function_docs import collect_function_targets, read_cpp_source  # noqa: E402


DEFAULT_CONFIG = PROJECT_ROOT / "Content" / "Data" / "tests" / "cli" / "thin-wrapper.json"


@dataclass(frozen=True)
class Finding:
    path: Path
    line: int
    rule_id: str
    message: str


def load_config(path: Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def relative_path(path: Path, project_root: Path) -> str:
    try:
        return path.relative_to(project_root).as_posix()
    except ValueError:
        return path.as_posix()


def discover_command_surfaces(project_root: Path, config: dict) -> tuple[list[Path], list[Finding]]:
    files: set[Path] = set()
    findings: list[Finding] = []
    for raw_path in config["requiredFiles"]:
        path = project_root / raw_path
        if path.is_file():
            files.add(path)
        else:
            findings.append(Finding(path, 1, "CLI-THIN-000", "required command surface is missing"))
    for discovery in config["discovery"]:
        root = project_root / discovery["root"]
        matches = sorted(path for path in root.glob(discovery["glob"]) if path.is_file()) if root.is_dir() else []
        files.update(matches)
        if len(matches) < discovery["minimumMatches"]:
            findings.append(
                Finding(
                    root,
                    1,
                    "CLI-THIN-000",
                    f"{discovery['name']} discovery returned {len(matches)} file(s); expected at least {discovery['minimumMatches']}",
                )
            )
    return sorted(files), findings


def line_number(source: str, index: int) -> int:
    return source.count("\n", 0, index) + 1


def source_rule_findings(path: Path, config: dict) -> list[Finding]:
    source = read_cpp_source(path)
    code = code_only(source)
    findings: list[Finding] = []
    for rule in config["sourceRules"]:
        flags = re.MULTILINE | (re.IGNORECASE if rule.get("ignoreCase", False) else 0)
        pattern = re.compile(rule["pattern"], flags)
        findings.extend(
            Finding(path, line_number(code, match.start()), rule["id"], rule["summary"])
            for match in pattern.finditer(code)
        )
    return findings


def function_size_findings(path: Path, config: dict) -> list[Finding]:
    source = read_cpp_source(path)
    rule = config["functionSize"]
    maximum = rule["maxLines"]
    findings: list[Finding] = []
    for target in collect_function_targets(path):
        if target.body_end is None:
            continue
        start_line = line_number(source, target.start)
        end_line = line_number(source, target.body_end)
        count = end_line - start_line + 1
        if count > maximum:
            findings.append(
                Finding(
                    path,
                    start_line,
                    rule["id"],
                    f"{rule['summary']}: {target.name} is {count} lines; maximum is {maximum}",
                )
            )
    return findings


def collect_findings(project_root: Path, config: dict) -> tuple[list[Path], list[Finding]]:
    files, findings = discover_command_surfaces(project_root, config)
    for path in files:
        findings.extend(source_rule_findings(path, config))
        findings.extend(function_size_findings(path, config))
    return files, sorted(findings, key=lambda finding: (finding.path.as_posix(), finding.line, finding.rule_id))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--project-root", type=Path, default=PROJECT_ROOT)
    parser.add_argument("--config", type=Path, default=DEFAULT_CONFIG)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    project_root = args.project_root.resolve()
    config = load_config(args.config.resolve())
    files, findings = collect_findings(project_root, config)
    if not files:
        print("Thin-wrapper guard failed: no command surfaces were discovered.")
        return 1
    if findings:
        print(f"Thin-wrapper guard failed: {len(findings)} finding(s) across {len(files)} command surfaces.")
        for finding in findings:
            print(
                f"{relative_path(finding.path, project_root)}:{finding.line}: "
                f"[{finding.rule_id}] {finding.message}"
            )
        return 1
    maximum = config["functionSize"]["maxLines"]
    print(
        f"Thin-wrapper guard passed for {len(files)} command surfaces; "
        f"every function is at most {maximum} lines."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
