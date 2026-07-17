#!/usr/bin/env python3
"""Validate a clean ForbocAI Fab source package before archive creation."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
import re


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parents[1]
CONFIG_PATH = SCRIPT_DIR / "data" / "package.json"
SECRET_PATTERN = re.compile(r"\b(?:sk_|ku)[A-Za-z0-9]{24,}\b")
SCANNED_SUFFIXES = {
    ".c",
    ".cmd",
    ".cpp",
    ".cs",
    ".h",
    ".hpp",
    ".ini",
    ".json",
    ".md",
    ".ps1",
    ".sh",
    ".txt",
    ".uplugin",
}


def load_config() -> dict[str, object]:
    """User Story: As package validation, I need the authored release contract. Signature: load_config() -> dict[str, object]."""
    return json.loads(CONFIG_PATH.read_text(encoding="utf-8"))


def resolve_package_root(config: dict[str, object], requested: str | None) -> Path:
    """User Story: As portable tooling, I need package paths resolved from one explicit root. Signature: resolve_package_root(config: dict[str, object], requested: str | None) -> Path."""
    paths = config["paths"]
    value = requested if requested is not None else str(paths["stageDirectory"])
    path = Path(value)
    return path.resolve() if path.is_absolute() else (PROJECT_ROOT / path).resolve()


def descriptor_findings(root: Path, config: dict[str, object]) -> list[str]:
    """User Story: As Fab review preparation, I need the plugin descriptor contract checked. Signature: descriptor_findings(root: Path, config: dict[str, object]) -> list[str]."""
    descriptor_path = root / str(config["descriptor"])
    if not descriptor_path.is_file():
        return [f"missing plugin descriptor: {descriptor_path}"]
    descriptor = json.loads(descriptor_path.read_text(encoding="utf-8"))
    findings: list[str] = []
    required_text = ("VersionName", "FriendlyName", "CreatedBy", "CreatedByURL", "DocsURL", "SupportURL", "Description")
    findings.extend(f"descriptor field {field} must be non-empty" for field in required_text if not str(descriptor.get(field, "")).strip())
    if descriptor.get("FriendlyName") != config.get("productName"):
        findings.append("descriptor FriendlyName must match the package productName")
    modules = descriptor.get("Modules", [])
    if not isinstance(modules, list) or len(modules) != 1:
        findings.append("descriptor must expose exactly one runtime module")
    elif modules[0].get("Name") != config["pluginName"] or modules[0].get("Type") != "Runtime":
        findings.append("descriptor runtime module must match the package pluginName")
    return findings


def path_findings(root: Path, config: dict[str, object]) -> list[str]:
    """User Story: As customer packaging, I need required files present and development residue absent. Signature: path_findings(root: Path, config: dict[str, object]) -> list[str]."""
    findings = [f"missing required package path: {value}" for value in config["required"] if not (root / str(value)).exists()]
    forbidden_parts = {str(value).casefold() for value in config["forbiddenPathParts"]}
    forbidden_extensions = {str(value).casefold() for value in config["forbiddenExtensions"]}
    for path in sorted(root.rglob("*")):
        relative = path.relative_to(root)
        if any(part.casefold() in forbidden_parts for part in relative.parts):
            findings.append(f"forbidden package path: {relative.as_posix()}")
        if path.is_file() and path.suffix.casefold() in forbidden_extensions:
            findings.append(f"forbidden package extension: {relative.as_posix()}")
    return findings


def secret_findings(root: Path) -> list[str]:
    """User Story: As secure distribution, I need credential-shaped values blocked from customer archives. Signature: secret_findings(root: Path) -> list[str]."""
    findings: list[str] = []
    for path in sorted(value for value in root.rglob("*") if value.is_file() and value.suffix.casefold() in SCANNED_SUFFIXES):
        text = path.read_text(encoding="utf-8", errors="ignore")
        if SECRET_PATTERN.search(text):
            findings.append(f"credential-shaped value found in {path.relative_to(root).as_posix()}")
    return findings


def validate(root: Path, config: dict[str, object]) -> list[str]:
    """User Story: As release orchestration, I need all package checks composed into one result. Signature: validate(root: Path, config: dict[str, object]) -> list[str]."""
    if not root.is_dir():
        return [f"package root does not exist: {root}"]
    return descriptor_findings(root, config) + path_findings(root, config) + secret_findings(root)


def main() -> int:
    """User Story: As a publisher, I need validation failures to stop a Fab release. Signature: main() -> int."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--package-root", help="Prepared plugin package root")
    args = parser.parse_args()
    config = load_config()
    root = resolve_package_root(config, args.package_root)
    findings = validate(root, config)
    if findings:
        print(f"Fab package validation failed with {len(findings)} finding(s):")
        for finding in findings:
            print(f"- {finding}")
        return 1
    file_count = sum(1 for path in root.rglob("*") if path.is_file())
    print(f"Fab package validation passed: {file_count} file(s) in {root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
