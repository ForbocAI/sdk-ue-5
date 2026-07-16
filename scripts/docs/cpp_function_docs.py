#!/usr/bin/env python3
"""Discover and inspect first-party C++ function documentation."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re

from cpp_tokens import Token, matching_pairs, tokenize


CONTROL_NAMES = {
    "alignof",
    "catch",
    "check",
    "decltype",
    "ensure",
    "for",
    "if",
    "return",
    "sizeof",
    "static_assert",
    "switch",
    "TEXT",
    "UE_LOG",
    "while",
}
MACRO_PREFIXES = ("DECLARE_", "DEFINE_", "GENERATED_", "IMPLEMENT_")
PREFIX_FORBIDDEN = {
    ".",
    "->",
    "?",
    ":",
    "return",
    "co_return",
    "new",
    "typedef",
    "using",
}
SOURCE_SUFFIXES = {".h", ".hpp", ".cpp"}
EXCLUDED_PARTS = {"Binaries", "Intermediate", "Saved", "ThirdParty", ".forbocai-plugin-host"}


@dataclass(frozen=True)
class FunctionDocTarget:
    path: Path
    name: str
    start: int
    signature: str
    body_start: int | None
    body_end: int | None


@dataclass(frozen=True)
class FunctionDocFinding:
    path: Path
    line: int
    name: str
    missing: tuple[str, ...]


def read_cpp_source(path: Path) -> str:
    return path.read_bytes().decode("utf-8", errors="replace")


def first_party_cpp_files(source_roots: tuple[Path, ...]) -> list[Path]:
    return sorted(
        path
        for root in source_roots
        for path in root.rglob("*")
        if path.is_file()
        and path.suffix in SOURCE_SUFFIXES
        and not any(part in EXCLUDED_PARTS for part in path.parts)
    )


def _function_name(
    tokens: list[Token],
    open_index: int,
    paren_close_to_open: dict[int, int],
    bracket_close_to_open: dict[int, int],
) -> tuple[str, int] | None:
    previous = open_index - 1
    if previous < 0:
        return None
    token = tokens[previous]
    if token.kind == "identifier":
        if token.text == "operator":
            return None
        if previous > 0 and tokens[previous - 1].text == "operator":
            return f"operator {token.text}", previous - 1
        start = previous - 1 if previous > 0 and tokens[previous - 1].text == "~" else previous
        name = ("~" if start != previous else "") + token.text
        return name, start
    if previous > 0 and tokens[previous - 1].text == "operator":
        return f"operator{token.text}", previous - 1
    paired_open = (
        paren_close_to_open.get(previous)
        if token.text == ")"
        else bracket_close_to_open.get(previous)
        if token.text == "]"
        else None
    )
    if paired_open is not None and paired_open > 0 and tokens[paired_open - 1].text == "operator":
        return f"operator{tokens[paired_open].text}{token.text}", paired_open - 1
    return None


def _terminator(tokens: list[Token], close_index: int) -> tuple[int, int | None] | None:
    if close_index + 1 < len(tokens) and tokens[close_index + 1].text in {">", ">>"}:
        return None
    nested = 0
    constructor_colon: int | None = None
    for index in range(close_index + 1, min(len(tokens), close_index + 160)):
        text = tokens[index].text
        if text in {"(", "[", "<"}:
            nested += 1
        elif text in {")",
            "]",
            ">",
        } and nested > 0:
            nested -= 1
        elif nested == 0:
            if text == ":":
                constructor_colon = index
            elif text == "=" and constructor_colon is None:
                following = tokens[index + 1].text if index + 1 < len(tokens) else ""
                if following not in {"0", "default", "delete"}:
                    return None
            elif text == "," and constructor_colon is None:
                return None
            elif text in {".", "?"}:
                return None
            elif text in {"{", ";"}:
                return index, constructor_colon
            elif text == ")":
                return None
    return None


def _declaration_start(
    tokens: list[Token],
    name_index: int,
    paren_close_to_open: dict[int, int],
) -> int:
    index = name_index - 1
    while index >= 0:
        text = tokens[index].text
        if text in {";", "{", "}"}:
            return index + 1
        if text == ")" and index in paren_close_to_open:
            open_index = paren_close_to_open[index]
            macro_index = open_index - 1
            if macro_index >= 0 and tokens[macro_index].text.isupper():
                return index + 1
        if text == ":" and index > 0 and tokens[index - 1].text in {"public", "private", "protected"}:
            return index + 1
        index -= 1
    return 0


def _looks_declared(tokens: list[Token], start: int, name_index: int, name: str) -> bool:
    prefix = tokens[start:name_index]
    angle_depth = 0
    for token in prefix:
        if token.text == "<":
            angle_depth += 1
        elif token.text == ">" and angle_depth > 0:
            angle_depth -= 1
        elif token.text == ">>" and angle_depth > 0:
            angle_depth = max(0, angle_depth - 2)
        elif angle_depth == 0 and (
            token.text in PREFIX_FORBIDDEN or token.text in {"=", ","}
        ):
            return False
    meaningful = [token for token in prefix if token.text not in {"template", "<", ">", ","}]
    scoped_constructor = name_index >= 2 and tokens[name_index - 1].text == "::"
    return bool(meaningful) or scoped_constructor or name.startswith("~") or name[:1].isupper()


def _strip_comments(value: str) -> str:
    return re.sub(r"/\*.*?\*/|//[^\r\n]*", " ", value, flags=re.DOTALL)


def _normalized_signature(source: str, start: int, end: int) -> str:
    return re.sub(r"\s+", " ", _strip_comments(source[start:end])).strip()


def collect_function_targets(path: Path) -> list[FunctionDocTarget]:
    source = read_cpp_source(path)
    tokens = tokenize(source)
    paren_open_to_close, paren_close_to_open = matching_pairs(tokens, "(", ")")
    brace_open_to_close, _brace_close_to_open = matching_pairs(tokens, "{", "}")
    _bracket_open_to_close, bracket_close_to_open = matching_pairs(tokens, "[", "]")
    candidates: list[FunctionDocTarget] = []

    for open_index, close_index in paren_open_to_close.items():
        named = _function_name(tokens, open_index, paren_close_to_open, bracket_close_to_open)
        if named is None:
            continue
        name, name_index = named
        if (
            name in CONTROL_NAMES
            or name.isupper()
            or name.startswith(MACRO_PREFIXES)
        ):
            continue
        terminated = _terminator(tokens, close_index)
        if terminated is None:
            continue
        terminator_index, constructor_colon = terminated
        start_index = _declaration_start(tokens, name_index, paren_close_to_open)
        if not _looks_declared(tokens, start_index, name_index, name):
            continue
        signature_end_index = constructor_colon or terminator_index
        signature = _normalized_signature(
            source,
            tokens[start_index].start,
            tokens[signature_end_index].start,
        )
        if not signature or signature.startswith("#"):
            continue
        body_start = tokens[terminator_index].start if tokens[terminator_index].text == "{" else None
        body_end = (
            tokens[brace_open_to_close[terminator_index]].end
            if body_start is not None and terminator_index in brace_open_to_close
            else None
        )
        candidates.append(
            FunctionDocTarget(
                path=path,
                name=name,
                start=tokens[start_index].start,
                signature=signature,
                body_start=body_start,
                body_end=body_end,
            )
        )

    definitions = [item for item in candidates if item.body_start is not None and item.body_end is not None]
    filtered = [
        item
        for item in candidates
        if not any(
            definition.body_start < item.start < definition.body_end
            for definition in definitions
            if definition.start != item.start
        )
    ]
    by_body: dict[tuple[int, int], FunctionDocTarget] = {}
    without_shared_body: list[FunctionDocTarget] = []
    for item in sorted(filtered, key=lambda candidate: candidate.start):
        if item.body_start is None or item.body_end is None:
            without_shared_body.append(item)
            continue
        body = (item.body_start, item.body_end)
        if body not in by_body:
            by_body[body] = item
            without_shared_body.append(item)

    return sorted(
        {item.start: item for item in without_shared_body}.values(),
        key=lambda item: item.start,
    )


def _is_documentation_gap(value: str) -> bool:
    if not value.strip():
        return True
    tokens = tokenize(value)
    paren_open_to_close, _ = matching_pairs(tokens, "(", ")")
    index = 0
    while index < len(tokens):
        decorator = tokens[index].text
        if not (decorator.isupper() or decorator.startswith(MACRO_PREFIXES)):
            return False
        index += 1
        if index < len(tokens) and tokens[index].text == "(":
            close = paren_open_to_close.get(index)
            if close is None:
                return False
            index = close + 1
    return True


def attached_doxygen(source: str, start: int) -> tuple[int, int] | None:
    close = source.rfind("*/", 0, start)
    open_index = source.rfind("/**", 0, close + 1) if close >= 0 else -1
    return (
        (open_index, close + 2)
        if open_index >= 0 and _is_documentation_gap(source[close + 2 : start])
        else None
    )


def documented_signature(documentation: str) -> str | None:
    match = re.search(r"@fn[ \t]+([^\r\n]+)", documentation)
    if match is None:
        return None
    return re.sub(r"[ \t]*\*/[ \t]*$", "", match.group(1)).strip()


def documentation_domain(path: Path) -> str:
    parts = path.parts[:-1]
    anchors = ("Features", "Core", "Views", "Tests", "CLI")
    anchor_index = next(
        (index for index, part in enumerate(parts) if part in anchors),
        len(parts) - 1,
    )
    candidates = [
        part
        for part in parts[anchor_index:]
        if part not in {"Public", "Private", "Source", "TestGame"}
    ][-3:]
    value = " ".join(candidates) if candidates else path.stem
    return re.sub(
        r"\s+",
        " ",
        re.sub(r"(?<=[a-z0-9])(?=[A-Z])|[_-]+", " ", value),
    ).strip().lower()


def function_operation(name: str) -> str:
    operators = {
        "operator==": "compare values for equality",
        "operator!=": "compare values for inequality",
        "operator()": "invoke the callable value",
        "operator[]": "access an indexed value",
    }
    if name in operators:
        return operators[name]
    words = re.sub(r"(?<=[a-z0-9])(?=[A-Z])|[_-]+", " ", name).lower()
    return f"invoke {words}"


def generated_user_story(path: Path, name: str) -> str:
    domain = documentation_domain(path)
    operation = function_operation(name)
    return (
        f"User Story: As a {domain} consumer, I need to {operation} through a "
        f"stable signature so the {domain} workflow remains explicit and composable."
    )


def inspect_function_docs(path: Path) -> list[FunctionDocFinding]:
    source = read_cpp_source(path)
    findings: list[FunctionDocFinding] = []
    for target in collect_function_targets(path):
        attached = attached_doxygen(source, target.start)
        documentation = source[attached[0] : attached[1]] if attached else ""
        fn_signature = documented_signature(documentation)
        missing = tuple(
            key
            for key, absent in (
                ("signature", fn_signature != target.signature),
                ("user-story", "User Story:" not in documentation),
            )
            if absent
        )
        if missing:
            findings.append(
                FunctionDocFinding(
                    path,
                    source.count("\n", 0, target.start) + 1,
                    target.name,
                    missing,
                )
            )
    return findings
