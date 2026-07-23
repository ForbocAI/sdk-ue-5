#!/usr/bin/env python3
"""FP source conformance rules, on the check_fp engine.

This is the Python successor to the retired UE FP shell checker and the local
home for general SDK FP guardrails. It keeps the shell checker's structural
coverage while adding shared FP rules from the TS checker: no mutation by
accumulation, explicit nullable modeling, visible use of the FP substrate, and a
public functional-core surface contract.
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
    column_number,
    explain,
    format_json,
    format_sarif,
    format_text,
    fp_helpers_for,
    line_number,
    register,
    rel,
    PROJECT_ROOT,
    SOURCE_ROOT,
)
from ue_targets import ue_targets


SDK_SOURCE_CANDIDATES = (
    PROJECT_ROOT / "Source" / "ForbocAI_SDK",
    SOURCE_ROOT / "ForbocAI_SDK",
    SOURCE_ROOT,
)
SDK_SOURCE_ROOT = next((root for root in SDK_SOURCE_CANDIDATES if root.exists()), SDK_SOURCE_CANDIDATES[0])
PACKAGES_ROOT = PROJECT_ROOT / "packages"
PUBLIC_ROOT = SDK_SOURCE_ROOT / "Public"
PRIVATE_ROOT = SDK_SOURCE_ROOT / "Private"
CLI_ROOT = PRIVATE_ROOT / "CLI"
COMMANDLET_FILE = PRIVATE_ROOT / "Commandlet.cpp"
CPP_SUFFIXES = {".cpp", ".h", ".hpp"}
TS_SUFFIXES = {".ts", ".tsx", ".js", ".jsx", ".mts", ".cts", ".mjs", ".cjs"}
SOURCE_SUFFIXES = CPP_SUFFIXES | TS_SUFFIXES

CONTROL_RE = re.compile(r"\b(for|while)\s*\(|\bdo\s*\{")
IF_RE = re.compile(r"\bif\s*\(")
SWITCH_RE = re.compile(r"\bswitch\s*\(")
CLASS_RE = re.compile(
    r"^\s*(?:export\s+)?(?:default\s+)?class\s+(?:(?:[A-Z][A-Z0-9_]*_API)\s+)?"
    r"([A-Z][A-Za-z0-9_]*)\b(?P<tail>[^\n{;]*)(?P<end>[{;])?",
    re.MULTILINE,
)
MUTABLE_RE = re.compile(r"\bmutable\b")
DIRECT_HTTP_RE = re.compile(r"\bFHttpModule::Get\s*\(\s*\)\s*\.\s*CreateRequest\s*\(")
TS_DIRECT_HTTP_RE = re.compile(r"\bfetch\s*\(|\baxios\s*\.|\bhttps?\s*\.\s*request\s*\(")
MOCK_RE = re.compile(r"\bmock\b|[A-Za-z0-9_]*Mock[A-Za-z0-9_]*", re.IGNORECASE)
ACCUMULATION_RE = re.compile(
    r"\.\s*(?:"
    r"Add|AddUnique|Append|Emplace|Insert|Push|Remove|RemoveAt|Reset|Empty|"
    r"push|pop|shift|unshift|splice|sort|reverse"
    r")\s*\("
)
NULLISH_RE = re.compile(
    r"\bnullptr\b\s*(?:==|!=)|(?:==|!=)\s*\bnullptr\b|"
    r"\b(?:null|undefined)\b\s*(?:={2,3}|!={1,2})|(?:={2,3}|!={1,2})\s*\b(?:null|undefined)\b|"
    r"\bIsValid\s*\("
)
FP_SUBSTRATE_RE = re.compile(
    r"\bfunc::|#include\s+[\"<][^\">]*fp\.hpp[\">]|"
    r"\b(?:just|nothing|fmap|mbind|match|orElse|fromNullable|requireJust|compose|curry|"
    r"multiMatch|createDispatcher|left|right|efmap|ebind|ematch)\b|"
    r"from ['\"][^'\"]*(?:/core/fp|/fp|@forbocai/core)[^'\"]*['\"]"
)

UE_REQUIRED_FP_SURFACE = (
    "Maybe", "Either", "fmap", "mbind", "match", "or_else", "from_nullable",
    "compose", "curry", "fold", "filter", "traverse", "multi_match",
    "Dispatcher", "AsyncResult",
)
TS_REQUIRED_FP_EXPORTS = (
    "just", "nothing", "fmap", "mbind", "match", "orElse", "isJust",
    "isNothing", "fromNullable", "requireJust", "left", "right", "efmap",
    "ebind", "ematch", "isLeft", "isRight", "compose", "curry",
    "fold", "filter", "traverse",
    "createDispatcher", "multiMatch", "_",
)


def _helper_hint(kind: str) -> str:
    helpers = fp_helpers_for(kind)
    return f"func::{{{', '.join(helpers)}}}" if helpers else "the func:: FP helpers"


NO_IMPERATIVE_LOOPS = register(Rule(
    "FP-SOURCE-001",
    Severity.HIGH,
    "imperative loop in first-party runtime source",
    f"Replace open-coded loops with recursion or {_helper_hint('loop')}; keep imperative loops only at documented UE/effect boundaries.",
    "fp: first-party runtime source uses folds/traversals instead of for/while",
))
CLASS_DECLARATION = register(Rule(
    "FP-SOURCE-002",
    Severity.MEDIUM,
    "class declaration in first-party source",
    "Prefer structs plus factory/free functions. UE reflection classes are boundary exceptions and should remain thin.",
    "fp: structs and data factories over ownership-style classes",
))
PUBLIC_MUTABLE = register(Rule(
    "FP-SOURCE-003",
    Severity.MEDIUM,
    "mutable keyword in public header",
    "Keep public values copy-on-write/immutable. Only canonical memoization primitives in fp.hpp/rtk.hpp may expose mutable storage.",
    "fp: public state is immutable or confined to canonical memoization helpers",
))
COMMAND_SIZE = register(Rule(
    "FP-SOURCE-004",
    Severity.HIGH,
    "command handler exceeds FP size budget",
    "Split command handlers into thin parsing surfaces plus composed operation functions. SetupCommands.cpp has a larger migration budget; ordinary handlers stay under 300 lines.",
    "fp: command surfaces stay thin and delegate behavior",
))
COMMAND_DIRECT_HTTP = register(Rule(
    "FP-SOURCE-005",
    Severity.HIGH,
    "direct HTTP request creation in command handler",
    "Command handlers must dispatch through runtime operations, store thunks, or RTK Query endpoints instead of creating requests directly.",
    "rtk/side-effects: effects live in operation/thunk/adapters, not command surfaces",
))
IF_STATEMENT = register(Rule(
    "FP-SOURCE-006",
    Severity.MEDIUM,
    "if statement in first-party source",
    f"Prefer expression-style selection: ternary for tiny value choices, {_helper_hint('route')}, match/multi_match, or Dispatcher tables for routing.",
    "fp: branch routing is data/composition driven where practical",
))
SWITCH_STATEMENT = register(Rule(
    "FP-SOURCE-007",
    Severity.MEDIUM,
    "switch statement in first-party source",
    "Use visitor/variant dispatch, func::multi_match, or dictionary-style Dispatcher tables instead of switch routing.",
    "fp: routing uses match/dispatch tables rather than switch",
))
MOCK_REFERENCE = register(Rule(
    "FP-SOURCE-008",
    Severity.HIGH,
    "mock reference in runtime or public source",
    "Tests should exercise real code paths and real infrastructure boundaries. Use fixtures/authored data, not mock/stub abstractions.",
    "fp: tests verify real behavior and avoid mocked runtime paths",
))
MUTABLE_ACCUMULATION = register(Rule(
    "FP-SOURCE-009",
    Severity.MEDIUM,
    "mutable accumulation call",
    "Prefer map/filter/reduce/fold/traverse, immutable builders, or one effect-boundary adapter over in-place accumulation.",
    "fp: derive collections with folds and transforms instead of push/Add mutation",
))
NULLISH_BRANCH = register(Rule(
    "FP-SOURCE-010",
    Severity.MEDIUM,
    "raw nullable/nullptr check",
    "Lift nullable values into Maybe/Either/from_nullable/fromNullable and route with match/or_else/orElse at the domain boundary.",
    "fp: nullable state is modeled as data, not scattered null checks",
))
FP_SUBSTRATE_MISSING = register(Rule(
    "FP-SOURCE-011",
    Severity.LOW,
    "FP-shaped file does not use the FP substrate",
    "Files with branching, nullable handling, or accumulation should visibly compose through the canonical core/fp substrate unless they are pure adapters.",
    "fp: use the canonical FP substrate instead of ad hoc control plumbing",
))
FP_CORE_SURFACE = register(Rule(
    "FP-SOURCE-012",
    Severity.HIGH,
    "functional core surface is missing documented primitives",
    "Keep fp.hpp and core/fp.ts exposing the documented Maybe/Either/composition/matching primitives so SDKs have the same FP vocabulary.",
    "fp: canonical functional-core surface contract",
))

BLOCKING_SEVERITIES = {Severity.CRITICAL, Severity.HIGH}
IGNORED_PARTS = {".git", "Binaries", "Build", "coverage", "dist", "DerivedDataCache", "Intermediate", "node_modules", "Saved", "__pycache__"}
SKIPPED_PARTS = {"Tests", "ThirdParty", "__pycache__"}
SKIPPED_SOURCE_FILENAMES = {"SqliteAmalgamation.c"}
PUBLIC_MEMOIZATION_EXCEPTIONS = {"fp.hpp", "rtk.hpp"}
FP_IMPLEMENTATION_EXCEPTIONS = {"fp.hpp", "rtk.hpp", "fp.ts"}


def has_part(path: Path, parts: set[str]) -> bool:
    return any(part in parts for part in path.parts)


def is_runtime_source(path: Path) -> bool:
    return (
        not has_part(path, IGNORED_PARTS | SKIPPED_PARTS)
        and path.name not in SKIPPED_SOURCE_FILENAMES
        and path.suffix in SOURCE_SUFFIXES
    )


def is_ts_source(path: Path) -> bool:
    return path.suffix in TS_SUFFIXES


def is_cpp_source(path: Path) -> bool:
    return path.suffix in CPP_SUFFIXES


def iter_files(root: Path, suffixes: set[str] = SOURCE_SUFFIXES) -> list[Path]:
    if not root.exists():
        return []
    return sorted(
        path
        for path in root.rglob("*")
        if path.is_file() and path.suffix in suffixes and not has_part(path, IGNORED_PARTS)
    )


def ue_authored_source_roots() -> list[Path]:
    roots: list[Path] = []
    for target in ue_targets():
        if target.kind == "sdk":
            module_root = target.root / "Source" / "ForbocAI_SDK"
            roots.append(module_root if module_root.exists() else target.root / "Source")
        elif target.kind == "sdk-cli":
            module_root = target.root / "Source" / "ForbocAI_TestGame_CLI"
            roots.append(module_root if module_root.exists() else target.root / "Source")
        else:
            roots.append(target.root / "Source")
    return sorted(dict.fromkeys(root for root in roots if root.exists()))


def authored_source_roots() -> list[Path]:
    ue_roots = ue_authored_source_roots()
    if ue_roots:
        return ue_roots
    if PACKAGES_ROOT.exists():
        return sorted(path for path in PACKAGES_ROOT.glob("*/src") if path.is_dir())
    return [SOURCE_ROOT] if SOURCE_ROOT.exists() else []


def authored_source_files() -> list[Path]:
    files = [path for root in authored_source_roots() for path in iter_files(root)]
    return sorted(dict.fromkeys(files))


def public_header_files() -> list[Path]:
    public_roots = tuple(root / "Public" for root in authored_source_roots() if (root / "Public").exists())
    roots = public_roots or tuple(root for root in (SOURCE_ROOT,) if root.exists())
    return [
        path
        for root in roots
        for path in iter_files(root, CPP_SUFFIXES)
        if path.suffix in {".h", ".hpp"} and not has_part(path, {"ThirdParty", "__pycache__"})
    ]


def command_surface_files() -> list[Path]:
    files: list[Path] = []
    if COMMANDLET_FILE.exists():
        files.append(COMMANDLET_FILE)
    if CLI_ROOT.exists():
        files.extend(path for path in iter_files(CLI_ROOT, CPP_SUFFIXES) if path.suffix in {".h", ".hpp", ".cpp"})
    for root in authored_source_roots():
        for candidate in (
            root / "Private" / "Commandlet.cpp",
            root / "Private" / "CLI",
            root / "Public" / "CLI",
        ):
            if candidate.is_file() and candidate.suffix in CPP_SUFFIXES:
                files.append(candidate)
            elif candidate.is_dir():
                files.extend(path for path in iter_files(candidate, CPP_SUFFIXES) if path.suffix in {".h", ".hpp", ".cpp"})
        features = root / "Public" / "TestGame" / "Features"
        if features.is_dir():
            files.extend(
                path
                for path in iter_files(features, CPP_SUFFIXES)
                if path.suffix in {".h", ".hpp", ".cpp"}
                and path.stem.startswith("CommandRunner")
            )
    if PACKAGES_ROOT.exists():
        files.extend(
            path
            for path in authored_source_files()
            if is_ts_source(path)
            and "commands" in path.parts
            and ("cli" in path.parts or "browserCli" in path.parts)
        )
    return sorted(dict.fromkeys(files))


def mock_scan_files() -> list[Path]:
    files: list[Path] = []
    roots = authored_source_roots()
    if roots:
        files.extend(path for root in roots for path in iter_files(root, SOURCE_SUFFIXES) if not has_part(path, {"ThirdParty", "__pycache__"}))
    elif SOURCE_ROOT.exists():
        files.extend(path for path in iter_files(SOURCE_ROOT, SOURCE_SUFFIXES) if not has_part(path, {"ThirdParty", "__pycache__"}))
    elif PACKAGES_ROOT.exists():
        files.extend(authored_source_files())
    return sorted(dict.fromkeys(files))


def _finding(path: Path, index: int, rule: Rule, message: str) -> Finding:
    text = path.read_text(encoding="utf-8", errors="replace")
    return Finding(path, line_number(text, index), rule.id, rule.severity, message, column_number(text, index))


def _scan_pattern(path: Path, pattern: re.Pattern[str], rule: Rule, message) -> list[Finding]:
    raw = path.read_text(encoding="utf-8", errors="replace")
    code = code_only(raw)
    return [
        Finding(path, line_number(code, match.start()), rule.id, rule.severity, message(match), column_number(code, match.start()))
        for match in pattern.finditer(code)
    ]


def scan_control_flow() -> list[Finding]:
    findings: list[Finding] = []
    for path in authored_source_files():
        if not is_runtime_source(path):
            continue
        findings.extend(_scan_pattern(
            path,
            CONTROL_RE,
            NO_IMPERATIVE_LOOPS,
            lambda match: f"imperative `{match.group(1) or 'do'}` loop",
        ))
        findings.extend(_scan_pattern(
            path,
            IF_RE,
            IF_STATEMENT,
            lambda _match: "if statement; prefer expression-style routing where this is not a UE/effect boundary",
        ))
        findings.extend(_scan_pattern(
            path,
            SWITCH_RE,
            SWITCH_STATEMENT,
            lambda _match: "switch statement; prefer match/dispatch table routing",
        ))
    return findings


def scan_class_declarations() -> list[Finding]:
    findings: list[Finding] = []
    for path in authored_source_files():
        if not is_runtime_source(path):
            continue
        raw = path.read_text(encoding="utf-8", errors="replace")
        code = code_only(raw)
        for match in CLASS_RE.finditer(code):
            if (match.group("end") or "") == ";":
                continue
            findings.append(Finding(
                path,
                line_number(code, match.start()),
                CLASS_DECLARATION.id,
                CLASS_DECLARATION.severity,
                f"class `{match.group(1)}` declaration; review as UE boundary or convert to struct/free functions",
                column_number(code, match.start()),
            ))
    return findings


def scan_public_mutable() -> list[Finding]:
    findings: list[Finding] = []
    for path in public_header_files():
        if path.name in PUBLIC_MEMOIZATION_EXCEPTIONS:
            continue
        findings.extend(_scan_pattern(
            path,
            MUTABLE_RE,
            PUBLIC_MUTABLE,
            lambda _match: "mutable keyword in public header",
        ))
    return findings


def line_count(path: Path) -> int:
    return len(path.read_text(encoding="utf-8", errors="replace").splitlines())


def command_limit(path: Path) -> int:
    if path == COMMANDLET_FILE:
        return 700
    if path.name == "SetupCommands.cpp":
        return 1200
    return 300


def scan_command_sizes() -> list[Finding]:
    findings: list[Finding] = []
    for path in command_surface_files():
        limit = command_limit(path)
        count = line_count(path)
        if count > limit:
            findings.append(Finding(
                path,
                1,
                COMMAND_SIZE.id,
                COMMAND_SIZE.severity,
                f"{rel(path)} is {count} lines (limit: {limit})",
            ))
    return findings


def scan_command_http() -> list[Finding]:
    findings: list[Finding] = []
    for path in command_surface_files():
        pattern = TS_DIRECT_HTTP_RE if is_ts_source(path) else DIRECT_HTTP_RE
        findings.extend(_scan_pattern(
            path,
            pattern,
            COMMAND_DIRECT_HTTP,
            lambda match: f"direct HTTP call `{match.group(0).strip()}` in command handler",
        ))
    return findings


def scan_mock_references() -> list[Finding]:
    findings: list[Finding] = []
    for path in mock_scan_files():
        findings.extend(_scan_pattern(
            path,
            MOCK_RE,
            MOCK_REFERENCE,
            lambda match: f"`{match.group(0)}` reference in runtime/public source",
        ))
    return findings


def scan_accumulation_mutation() -> list[Finding]:
    findings: list[Finding] = []
    for path in authored_source_files():
        if not is_runtime_source(path) or path.name in FP_IMPLEMENTATION_EXCEPTIONS:
            continue
        findings.extend(_scan_pattern(
            path,
            ACCUMULATION_RE,
            MUTABLE_ACCUMULATION,
            lambda match: f"in-place accumulation `{match.group(0).strip()}`",
        ))
    return findings


def scan_nullish_checks() -> list[Finding]:
    findings: list[Finding] = []
    for path in authored_source_files():
        if not is_runtime_source(path) or path.name in FP_IMPLEMENTATION_EXCEPTIONS:
            continue
        findings.extend(_scan_pattern(
            path,
            NULLISH_RE,
            NULLISH_BRANCH,
            lambda match: f"raw nullable check `{match.group(0).strip()}`",
        ))
    return findings


def scan_fp_substrate_adoption() -> list[Finding]:
    findings: list[Finding] = []
    signal_patterns = (CONTROL_RE, IF_RE, SWITCH_RE, ACCUMULATION_RE, NULLISH_RE)
    for path in authored_source_files():
        if not is_runtime_source(path) or path.name in FP_IMPLEMENTATION_EXCEPTIONS:
            continue
        raw = path.read_text(encoding="utf-8", errors="replace")
        code = code_only(raw)
        if not any(pattern.search(code) for pattern in signal_patterns):
            continue
        if FP_SUBSTRATE_RE.search(raw):
            continue
        findings.append(Finding(
            path,
            1,
            FP_SUBSTRATE_MISSING.id,
            FP_SUBSTRATE_MISSING.severity,
            "file has FP-shaped control/data flow but no visible use of the canonical core/fp substrate",
        ))
    return findings


def missing_ue_surface_names(text: str) -> list[str]:
    return [name for name in UE_REQUIRED_FP_SURFACE if not re.search(rf"\b{re.escape(name)}\b", text)]


def cpp_include_closure(entry_path: Path, include_root: Path) -> str:
    visited: set[Path] = set()
    exposed_source: list[str] = []

    def visit(path: Path) -> None:
        resolved = path.resolve()
        if resolved in visited or not resolved.is_file():
            return
        try:
            resolved.relative_to(include_root.resolve())
        except ValueError:
            return
        visited.add(resolved)
        text = resolved.read_text(encoding="utf-8", errors="replace")
        exposed_source.append(text)
        for include in re.findall(r'^\s*#include\s+"([^"]+)"', text, flags=re.MULTILINE):
            visit(include_root / include)

    visit(entry_path)
    return "\n".join(exposed_source)


def ts_export_names(text: str) -> set[str]:
    names = set(re.findall(r"\bexport\s+(?:const|function|interface|type)\s+([A-Za-z_$][A-Za-z0-9_$]*)", text))
    names.update(
        raw.strip().split(" as ")[-1].strip()
        for block in re.findall(r"\bexport\s*\{([^}]+)\}", text, flags=re.DOTALL)
        for raw in block.split(",")
        if raw.strip()
    )
    return names


def missing_ts_surface_names(text: str) -> list[str]:
    exports = ts_export_names(text)
    return [name for name in TS_REQUIRED_FP_EXPORTS if name not in exports]


def scan_fp_core_surface() -> list[Finding]:
    findings: list[Finding] = []
    if SDK_SOURCE_ROOT.exists():
        core_path = PUBLIC_ROOT / "Core" / "fp.hpp"
        if core_path.exists():
            exposed_source = cpp_include_closure(core_path, PUBLIC_ROOT)
            missing = missing_ue_surface_names(exposed_source)
            if missing:
                findings.append(Finding(
                    core_path,
                    1,
                    FP_CORE_SURFACE.id,
                    FP_CORE_SURFACE.severity,
                    f"fp.hpp include graph missing documented FP primitives: {', '.join(missing)}",
                ))
    if PACKAGES_ROOT.exists():
        core_path = PROJECT_ROOT / "packages" / "core" / "src" / "core" / "fp.ts"
        if core_path.exists():
            missing = missing_ts_surface_names(core_path.read_text(encoding="utf-8", errors="replace"))
            if missing:
                findings.append(Finding(
                    core_path,
                    1,
                    FP_CORE_SURFACE.id,
                    FP_CORE_SURFACE.severity,
                    f"core/fp.ts missing documented FP exports: {', '.join(missing)}",
                ))
    return findings


def find_findings() -> list[Finding]:
    findings: list[Finding] = []
    findings.extend(scan_control_flow())
    findings.extend(scan_class_declarations())
    findings.extend(scan_public_mutable())
    findings.extend(scan_command_sizes())
    findings.extend(scan_command_http())
    findings.extend(scan_mock_references())
    findings.extend(scan_accumulation_mutation())
    findings.extend(scan_nullish_checks())
    findings.extend(scan_fp_substrate_adoption())
    findings.extend(scan_fp_core_surface())
    return findings


def has_blocking_findings(findings: list[Finding]) -> bool:
    return any(finding.severity in BLOCKING_SEVERITIES for finding in findings)


def format_text_with_warning_exit(findings: list[Finding], guard: str) -> str:
    if not findings:
        return f"{guard} passed."
    blocker_count = sum(1 for finding in findings if finding.severity in BLOCKING_SEVERITIES)
    if blocker_count:
        return format_text(findings, PROJECT_ROOT, guard)
    lines = [f"{guard} passed with {len(findings)} warning(s)."]
    for finding in sorted(findings, key=lambda f: (f.path.as_posix(), f.line, f.rule_id)):
        lines.append(
            f"{rel(finding.path)}:{finding.line}:{finding.column}: "
            f"[{finding.severity.value.upper()} {finding.rule_id}] {finding.message}"
        )
        if finding.guidance:
            lines.append(f"    fix: {finding.guidance}")
    return "\n".join(lines)


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
    guard = "FP source conformance guard"
    if args.format == "json":
        print(format_json(findings, PROJECT_ROOT))
    elif args.format == "sarif":
        print(format_sarif(findings, PROJECT_ROOT, guard))
    else:
        print(format_text_with_warning_exit(findings, guard))
    return 1 if has_blocking_findings(findings) else 0


if __name__ == "__main__":
    sys.exit(main())
