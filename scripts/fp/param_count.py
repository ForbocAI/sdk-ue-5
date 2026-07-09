#!/usr/bin/env python3
"""UE runtime parameter-count discipline, on the check_fp engine.

Mirrors Therapy 12's parameter discipline for this C++ runtime: keep authored
function DEFINITIONS at two data parameters or fewer and drive over-arity
functions toward functional composition. Each over-limit definition is bucketed
by WHY it carries the extra parameters; only the genuinely reducible
("actionable") bucket is a finding (FP-ARITY-001). The other buckets are
reported as an informational summary and never fail the guard:

    fn-arg       - a parameter is already a function/callable ("supply functions
                   as arguments"); leave at two.
    mut-sink     - the first parameter is a non-const &/* output/engine target.
                   A const &/* parameter is DATA, not a sink.
    reducer      - a Reduce*(state, action) function whose first parameter is the
                   by-value state accumulator; leave at two.
    ue-callback  - a UE reflection/lifecycle callback owned by the engine.

A payload/request struct is NOT a solution: bundling the arguments into one
struct only hides the arity behind a wrapper -- the function still does too much.
The real fix is to split the function into multiple smaller functions that
compose (currying to capture the stable inputs, functions-as-arguments, and
func::compose/pipe3/pipe4/converge2/folds); see Core/ue_fp.hpp.
"""

from __future__ import annotations

import argparse
from collections import Counter
from pathlib import Path
import re
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from check_fp import (
    Finding,
    Rule,
    Severity,
    explain,
    format_json,
    format_sarif,
    format_text,
    iter_source_files,
    register,
    PROJECT_ROOT,
    SOURCE_ROOT,
)


MAX_DATA_PARAMS = 2

FN_RE = re.compile(
    r"(?:^|[;\n{}])\s*(?:[\w:<>&*\s]+\s+)?([A-Za-z_][A-Za-z0-9_:~]*)\s*\(",
    re.MULTILINE,
)
CONTROL = {"if", "for", "while", "switch", "return", "sizeof", "TEXT"}
UE_CALLBACKS = {
    "BeginPlay", "Tick", "TickComponent", "SetupInputComponent", "InitGame",
    "NativeConstruct", "HandleInteractionBegin", "HandleInteractionEnd",
}
FN_TYPE_RE = re.compile(r"std::function\s*<|\bT(?:Unique)?Function(?:Ref)?\s*<|\(\s*\*")

ACTIONABLE = register(
    Rule(
        id="FP-ARITY-001",
        severity=Severity.HIGH,
        summary="function carries more than two reducible data parameters",
        guidance="A payload/request struct is NOT the fix -- it only hides the arity behind a wrapper. Split the function into multiple smaller functions that compose: (1) curry -- capture the stable inputs in a factory that returns a unary function over the varying input; (2) supply behavior as a function argument and compose the steps with func::compose/pipe3/pipe4/converge2, folds, catalogs, or a Dispatcher; (3) factor independent responsibilities into their own functions and wire them together. A leading const &/* parameter is DATA -- do not widen a signature by threading more const inputs.",
        skill="ue_fp.hpp cookbook: split into composed functions (currying, functions-as-args), not a payload-struct wrapper",
    )
)


def _split_top_level(text: str):
    depth = 0
    start = 0
    pairs = {"(": ")", "<": ">", "[": "]", "{": "}"}
    closers = set(pairs.values())
    for index, char in enumerate(text):
        if char in pairs:
            depth += 1
        elif char in closers:
            depth -= 1
        elif char == "," and depth == 0:
            yield text[start:index]
            start = index + 1
    yield text[start:]


def _matched_paren_body(text: str, open_index: int) -> str | None:
    depth = 1
    index = open_index + 1
    while index < len(text) and depth > 0:
        char = text[index]
        depth += 1 if char == "(" else (-1 if char == ")" else 0)
        index += 1
    return None if depth != 0 else text[open_index + 1: index - 1]


def _strip_receiver(name: str) -> str:
    return name.rsplit("::", 1)[-1].lstrip("~")


def _data_segments(param_text: str) -> list[str]:
    segments = [segment.strip() for segment in _split_top_level(param_text)]
    return [segment for segment in segments if segment and segment != "void"]


def _is_mut_sink(segment: str) -> bool:
    return ("&" in segment or "*" in segment) and "const" not in segment


def classify(name: str, segments: list[str], tail: str) -> str:
    simple = _strip_receiver(name)
    if simple in UE_CALLBACKS or re.search(r"\boverride\b", tail):
        return "ue-callback"
    if any(FN_TYPE_RE.search(segment) for segment in segments):
        return "fn-arg"
    if segments and _is_mut_sink(segments[0]):
        return "mut-sink"
    if simple.startswith("Reduce") and segments and "&" not in segments[0] and "*" not in segments[0]:
        return "reducer"
    return "actionable"


def scan_file(path: Path):
    text = path.read_text(encoding="utf-8", errors="replace")
    for match in FN_RE.finditer(text):
        name = match.group(1)
        if _strip_receiver(name) in CONTROL:
            continue
        open_index = match.end() - 1
        body = _matched_paren_body(text, open_index)
        if body is None:
            continue
        close_index = open_index + len(body) + 2
        tail = text[close_index: close_index + 80]
        if not re.match(
            r"\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?(?:noexcept\s*)?(?:->[^{;]+)?\{",
            tail,
        ):
            continue
        segments = _data_segments(body)
        yield name, len(segments), text[: match.start()].count("\n") + 1, classify(name, segments, tail)


def gather() -> tuple[list[Finding], Counter]:
    findings: list[Finding] = []
    counts: Counter = Counter()
    for path in iter_source_files(SOURCE_ROOT):
        for name, params, line, bucket in scan_file(path):
            if params <= MAX_DATA_PARAMS:
                continue
            counts[bucket] += 1
            if bucket == "actionable":
                findings.append(
                    Finding(path, line, ACTIONABLE.id, ACTIONABLE.severity, f"{params} data params: {name}")
                )
    return findings, counts


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

    findings, counts = gather()
    guard = "Parameter discipline guard"
    if args.format == "json":
        print(format_json(findings, PROJECT_ROOT))
        return 1 if findings else 0
    if args.format == "sarif":
        print(format_sarif(findings, PROJECT_ROOT, guard))
        return 1 if findings else 0

    total = sum(counts.values())
    print(
        f"Over {MAX_DATA_PARAMS} data params: {total} "
        f"(fn-arg {counts['fn-arg']}, mut-sink {counts['mut-sink']}, "
        f"reducer {counts['reducer']}, ue-callback {counts['ue-callback']} -- informational)."
    )
    if findings:
        print(format_text(findings, PROJECT_ROOT, guard))
    else:
        print(f"{guard} passed.")
    return 1 if findings else 0


if __name__ == "__main__":
    sys.exit(main())
