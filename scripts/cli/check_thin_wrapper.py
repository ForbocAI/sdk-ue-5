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


def pascal_case(value: str) -> str:
    return value[:1].upper() + value[1:]


def role_dispatch_findings(project_root: Path, config: dict) -> list[Finding]:
    findings: list[Finding] = []
    for contract in config.get("roleDispatchContracts", []):
        roles_path = project_root / contract["rolesFile"]
        commands_path = project_root / contract["commandsFile"]
        source_paths = [
            project_root / path
            for path in contract.get("sourceFiles", [contract.get("sourceFile", "")])
        ]
        if (
            not roles_path.is_file()
            or not commands_path.is_file()
            or any(not path.is_file() for path in source_paths)
        ):
            missing = next(
                path
                for path in (roles_path, commands_path, *source_paths)
                if not path.is_file()
            )
            findings.append(
                Finding(missing, 1, contract["id"], contract["missingSummary"])
            )
            continue
        roles = load_config(roles_path)
        commands = load_config(commands_path)
        group_keys = {
            key
            for key, command in commands.items()
            if command.get("group") == contract["group"]
        }
        role_keys = {
            command_key: role_key
            for role_key, command_key in roles.items()
            if command_key in group_keys
        }
        for command_key in sorted(group_keys - role_keys.keys()):
            findings.append(
                Finding(
                    roles_path,
                    1,
                    contract["id"],
                    f"{contract['unmappedSummary']}: {command_key}",
                )
            )
        code = "\n".join(code_only(read_cpp_source(path)) for path in source_paths)
        for command_key, role_key in sorted(role_keys.items()):
            pattern = re.compile(rf"\bRoles\.{re.escape(pascal_case(role_key))}\b")
            if not pattern.search(code):
                findings.append(
                    Finding(
                        source_paths[0],
                        1,
                        contract["id"],
                        f"{contract['undispatchedSummary']}: {command_key}",
                    )
                )
    return findings


def function_binding_findings(project_root: Path, config: dict) -> list[Finding]:
    findings: list[Finding] = []
    for binding in config.get("functionBindings", []):
        path = project_root / binding["sourceFile"]
        if not path.is_file():
            findings.append(
                Finding(path, 1, binding["id"], binding["missingSummary"])
            )
            continue
        source = read_cpp_source(path)
        targets = [
            target
            for target in collect_function_targets(path)
            if target.name == binding["function"]
            and target.body_start is not None
            and target.body_end is not None
        ]
        if len(targets) != 1:
            findings.append(
                Finding(path, 1, binding["id"], binding["missingSummary"])
            )
            continue
        target = targets[0]
        body = code_only(source[target.body_start : target.body_end + 1])
        for required in binding.get("requiredPatterns", []):
            if not re.search(required, body, re.MULTILINE):
                findings.append(
                    Finding(
                        path,
                        line_number(source, target.start),
                        binding["id"],
                        binding["requiredSummary"],
                    )
                )
        for forbidden in binding.get("forbiddenPatterns", []):
            match = re.search(forbidden, body, re.MULTILINE)
            if match:
                findings.append(
                    Finding(
                        path,
                        line_number(source, target.body_start + match.start()),
                        binding["id"],
                        binding["forbiddenSummary"],
                    )
                )
    return findings


def collect_findings(project_root: Path, config: dict) -> tuple[list[Path], list[Finding]]:
    files, findings = discover_command_surfaces(project_root, config)
    for path in files:
        findings.extend(source_rule_findings(path, config))
        findings.extend(function_size_findings(path, config))
    findings.extend(role_dispatch_findings(project_root, config))
    findings.extend(function_binding_findings(project_root, config))
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
