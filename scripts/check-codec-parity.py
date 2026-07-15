#!/usr/bin/env python3
"""Verify canonical nested protocol codec ownership and behavior."""

import re
import sys
from pathlib import Path


def extract_function_body(text: str, function_name: str) -> str | None:
    for match in re.finditer(rf"\b{re.escape(function_name)}\s*\(", text):
        index = match.end()
        parentheses = 1
        while index < len(text) and parentheses:
            parentheses += (text[index] == "(") - (text[index] == ")")
            index += 1
        if parentheses:
            continue
        while index < len(text) and text[index].isspace():
            index += 1
        if index >= len(text) or text[index] != "{":
            continue
        body_start = index + 1
        braces = 1
        index = body_start
        while index < len(text) and braces:
            braces += (text[index] == "{") - (text[index] == "}")
            if braces == 0:
                return text[body_start:index]
            index += 1
    return None


def discover_definition(root: Path, function_name: str) -> list[tuple[Path, str]]:
    definitions: list[tuple[Path, str]] = []
    for path in sorted(root.rglob("*.h")):
        body = extract_function_body(path.read_text(encoding="utf-8"), function_name)
        if body is not None:
            definitions.append((path, body))
    return definitions


def main() -> int:
    if len(sys.argv) != 2:
        print("Usage: check-codec-parity.py <protocol-source-root>")
        return 1

    protocol_root = Path(sys.argv[1]).resolve()
    if not protocol_root.is_dir():
        print(f"[FAIL] Protocol source root is not a directory: {protocol_root}")
        return 1

    contracts = {
        "SerializeDecisionResult": 'SetObjectField(TEXT("decisionIntent")',
        "SerializeReasoningResult": 'SetObjectField(TEXT("reasoningOutput")',
        "SerializeIdentifyActorResult": 'SetObjectField(TEXT("actor")',
    }
    failures: list[str] = []

    for function_name, required_expression in contracts.items():
        definitions = discover_definition(protocol_root, function_name)
        if len(definitions) != 1:
            failures.append(
                f"Expected exactly one {function_name} definition under "
                f"{protocol_root}; found {len(definitions)}."
            )
            continue
        path, body = definitions[0]
        if required_expression not in body:
            failures.append(
                f"{function_name} does not preserve its nested object field in {path}."
            )
        else:
            print(f"[OK] {function_name} preserves nested tape data ({path}).")

    if failures:
        print("\n".join(f"[FAIL] {failure}" for failure in failures))
        return 1
    print("[OK] Protocol codecs preserve canonical nested tape fields.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
