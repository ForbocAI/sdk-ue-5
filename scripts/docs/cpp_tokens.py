#!/usr/bin/env python3
"""Small first-party C++ lexer used by source documentation checks."""

from __future__ import annotations

from dataclasses import dataclass


MULTI_TOKENS = (
    "<=>",
    "->*",
    "...",
    "::",
    "->",
    "&&",
    "||",
    "==",
    "!=",
    "<=",
    ">=",
    "++",
    "--",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "<<",
    ">>",
    "[[",
    "]]",
)


@dataclass(frozen=True)
class Token:
    text: str
    start: int
    end: int
    kind: str = "symbol"


def _quoted_end(source: str, start: int, quote: str) -> int:
    index = start + 1
    while index < len(source):
        if source[index] == "\\":
            index += 2
            continue
        if source[index] == quote:
            return index + 1
        index += 1
    return len(source)


def _raw_string_end(source: str, start: int) -> int | None:
    delimiter_end = source.find("(", start + 2, start + 20)
    if delimiter_end < 0:
        return None
    delimiter = source[start + 2 : delimiter_end]
    terminator = ")" + delimiter + '"'
    literal_end = source.find(terminator, delimiter_end + 1)
    return None if literal_end < 0 else literal_end + len(terminator)


def _preprocessor_end(source: str, start: int) -> int:
    index = start
    while index < len(source):
        newline = source.find("\n", index)
        if newline < 0:
            return len(source)
        line = source[index:newline].rstrip()
        if not line.endswith("\\"):
            return newline + 1
        index = newline + 1
    return len(source)


def tokenize(source: str) -> list[Token]:
    tokens: list[Token] = []
    index = 0
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        line_start = source.rfind("\n", 0, index) + 1
        if char == "#" and not source[line_start:index].strip():
            index = _preprocessor_end(source, index)
            continue
        if char.isspace():
            index += 1
            continue
        if char == "/" and following == "/":
            newline = source.find("\n", index + 2)
            index = len(source) if newline < 0 else newline + 1
            continue
        if char == "/" and following == "*":
            close = source.find("*/", index + 2)
            index = len(source) if close < 0 else close + 2
            continue
        if char == "R" and following == '"':
            end = _raw_string_end(source, index)
            if end is not None:
                tokens.append(Token(source[index:end], index, end, "literal"))
                index = end
                continue
        if char in {'"', "'"}:
            end = _quoted_end(source, index, char)
            tokens.append(Token(source[index:end], index, end, "literal"))
            index = end
            continue
        if char.isalpha() or char == "_":
            end = index + 1
            while end < len(source) and (source[end].isalnum() or source[end] == "_"):
                end += 1
            tokens.append(Token(source[index:end], index, end, "identifier"))
            index = end
            continue
        if char.isdigit():
            end = index + 1
            while end < len(source) and (source[end].isalnum() or source[end] in "._'"):
                end += 1
            tokens.append(Token(source[index:end], index, end, "number"))
            index = end
            continue
        matched = next((value for value in MULTI_TOKENS if source.startswith(value, index)), None)
        if matched:
            tokens.append(Token(matched, index, index + len(matched)))
            index += len(matched)
            continue
        tokens.append(Token(char, index, index + 1))
        index += 1
    return tokens


def matching_pairs(tokens: list[Token], opening: str, closing: str) -> tuple[dict[int, int], dict[int, int]]:
    open_to_close: dict[int, int] = {}
    close_to_open: dict[int, int] = {}
    stack: list[int] = []
    for index, token in enumerate(tokens):
        if token.text == opening:
            stack.append(index)
        elif token.text == closing and stack:
            start = stack.pop()
            open_to_close[start] = index
            close_to_open[index] = start
    return open_to_close, close_to_open
