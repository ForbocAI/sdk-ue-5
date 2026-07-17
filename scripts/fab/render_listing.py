#!/usr/bin/env python3
"""Render the canonical Fab listing JSON into reviewer-ready Markdown."""

from __future__ import annotations

import json
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parents[1]
SOURCE = PROJECT_ROOT / "fab" / "listing.json"
DESTINATION = PROJECT_ROOT / "fab" / "listing.md"


def bullets(values: list[str]) -> list[str]:
    """User Story: As listing reviewers, I need structured values rendered consistently. Signature: bullets(values: list[str]) -> list[str]."""
    return [f"- {value}" for value in values]


def render_listing(data: dict[str, object]) -> str:
    """User Story: As a publisher, I need one canonical listing rendered without copy drift. Signature: render_listing(data: dict[str, object]) -> str."""
    technical = data["technicalDetails"]
    pricing = data["pricing"]
    support = data["support"]
    lines = [
        f"# {data['title']}",
        "",
        str(data["subtitle"]),
        "",
        "## Description",
        "",
        *[f"{paragraph}\n" for paragraph in data["description"]],
        "## Features",
        "",
        *bullets(data["features"]),
        "",
        "## Technical Details",
        "",
        f"- Engine version: Unreal Engine {technical['engineVersion']}",
        f"- Code modules: {technical['codeModules']} ({technical['moduleType']})",
        f"- Tested platforms: {', '.join(technical['testedPlatforms'])}",
        f"- Network: {technical['network']}",
        f"- Account: {technical['account']}",
        f"- Local storage: {technical['localStorage']}",
        f"- Dependencies: {technical['dependencies']}",
        "",
        "## Pricing",
        "",
        f"- License: {pricing['license']}",
        f"- Personal: ${pricing['personalUsd']:.2f}",
        f"- Professional: ${pricing['professionalUsd']:.2f}",
        "",
        "## Support",
        "",
        f"- Documentation: {support['documentation']}",
        f"- Support and issues: {support['issues']}",
        f"- Account: {support['account']}",
        f"- Website: {support['website']}",
        "",
        "## FAQ",
        "",
    ]
    for item in data["faq"]:
        lines.extend((f"### {item['question']}", "", str(item["answer"]), ""))
    return "\n".join(lines).rstrip() + "\n"


def main() -> int:
    """User Story: As release automation, I need listing Markdown refreshed from canonical data. Signature: main() -> int."""
    data = json.loads(SOURCE.read_text(encoding="utf-8"))
    DESTINATION.write_text(render_listing(data), encoding="utf-8")
    print(f"Rendered Fab listing: {DESTINATION}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
