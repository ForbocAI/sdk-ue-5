#!/usr/bin/env python3
"""Reject non-FP control flow in authored Source.

Built on the check_fp engine: findings carry stable ids/severities and the
guidance for each control-flow keyword is DERIVED from the func:: helper
vocabulary in Core/ue_fp.hpp, so a loop always points at the current fold/
traverse helpers and a switch at the current match/dispatch helpers. Beyond the
keyword ban it also flags the two ways people dodge it: deep ternary chains
(mechanical if-else-if replacement) and goto/do.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from check_fp import (
    Finding,
    Rule,
    Severity,
    code_only,
    explain,
    format_json,
    format_sarif,
    format_text,
    fp_helpers_for,
    iter_source_files,
    line_number,
    register,
    PROJECT_ROOT,
    SOURCE_ROOT,
)


CONTROL_RE = re.compile(r"\b(if|else|for|while|switch|case)\b")
# A genuine if-else-if-else-if ternary chain: three or more ?: with nothing that
# ends an expression (comma/semicolon/brace) between them, so separate ternaries
# in an argument list are not joined into a false "chain".
TERNARY_RE = re.compile(r"(?:\?[^?:;{},]+:[^?:;{},]+){2}\?[^?:;{},]+:")
JUMP_RE = re.compile(r"\b(goto|do)\b")


def _helper_hint(kind: str) -> str:
    helpers = fp_helpers_for(kind)
    return f"use func::{{{', '.join(helpers)}}}" if helpers else "use the func:: composition helpers"


CONTROL = register(
    Rule(
        id="FP-BRANCH-001",
        severity=Severity.HIGH,
        summary="non-FP control-flow keyword in authored Source",
        guidance=f"Loops: {_helper_hint('loop')}. Routing/branches: {_helper_hint('route')} plus func::match/ternary for small value selection. Explicit guards belong only at UE/effect boundaries.",
        skill="ue_fp.hpp: branching policy -- match/multi_match/Dispatcher + fold/filter/find/traverse instead of if/for/switch",
    )
)
TERNARY_ABUSE = register(
    Rule(
        id="FP-BRANCH-002",
        severity=Severity.MEDIUM,
        summary="chained ternary used as a mechanical if-replacement",
        guidance="ue_fp.hpp forbids mechanically replacing if with ternary chains. Use func::match for Maybe/Either, func::multi_match/Dispatcher case tables for routing.",
        skill="ue_fp.hpp: do not replace every if with ternary chains",
    )
)
JUMP = register(
    Rule(
        id="FP-BRANCH-003",
        severity=Severity.HIGH,
        summary="goto/do control flow",
        guidance="Replace with SDK functional-core recursion, folds, AsyncChain composition, or an explicit state-machine data table interpreted by one reusable step function.",
        skill="ue_fp.hpp: functional-core recursion/folds over open-ended loops",
    )
)


def scan_file(path: Path) -> list[Finding]:
    code = code_only(path.read_text(encoding="utf-8", errors="replace"))
    findings: list[Finding] = []
    for match in CONTROL_RE.finditer(code):
        findings.append(
            Finding(path, line_number(code, match.start()), CONTROL.id, CONTROL.severity,
                    f"non-FP `{match.group(1)}` statement")
        )
    for match in TERNARY_RE.finditer(code):
        findings.append(
            Finding(path, line_number(code, match.start()), TERNARY_ABUSE.id, TERNARY_ABUSE.severity,
                    TERNARY_ABUSE.summary)
        )
    for match in JUMP_RE.finditer(code):
        findings.append(
            Finding(path, line_number(code, match.start()), JUMP.id, JUMP.severity,
                    f"`{match.group(1)}` control flow")
        )
    return findings


def find_findings() -> list[Finding]:
    return [finding for path in iter_source_files(SOURCE_ROOT) for finding in scan_file(path)]


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
    guard = "Branchless Source guard"
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
