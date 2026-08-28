#!/usr/bin/env python3
"""Verify canonical protocol result projection ownership and behavior."""

import json
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
    if len(sys.argv) != 3:
        print(
            "Usage: check-codec-parity.py <protocol-source-root> "
            "<codec-contract>"
        )
        return 1

    protocol_root = Path(sys.argv[1]).resolve()
    contract_path = Path(sys.argv[2]).resolve()
    if not protocol_root.is_dir():
        print(f"[FAIL] Protocol source root is not a directory: {protocol_root}")
        return 1
    if not contract_path.is_file():
        print(f"[FAIL] Codec contract is not a file: {contract_path}")
        return 1

    document = json.loads(contract_path.read_text(encoding="utf-8"))
    contracts = document.get("protocolProjectionGuard", {}).get("functions", [])
    if not contracts:
        print(f"[FAIL] Codec contract has no projection functions: {contract_path}")
        return 1
    failures: list[str] = []

    for contract in contracts:
        function_name = contract["name"]
        definitions = discover_definition(protocol_root, function_name)
        if len(definitions) != 1:
            failures.append(
                f"Expected exactly one {function_name} definition under "
                f"{protocol_root}; found {len(definitions)}."
            )
            continue
        path, body = definitions[0]
        missing = [
            expression
            for expression in contract["requiredExpressions"]
            if expression not in body
        ]
        if missing:
            failures.append(
                f"{function_name} omits canonical projections in {path}: "
                + ", ".join(missing)
            )
            continue
        print(f"[OK] {function_name} preserves canonical API results ({path}).")

    if failures:
        print("\n".join(f"[FAIL] {failure}" for failure in failures))
        return 1
    print("[OK] Protocol codecs preserve canonical API result projections.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
