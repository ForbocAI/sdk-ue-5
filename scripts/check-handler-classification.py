#!/usr/bin/env python3
"""Enforce UE handler ownership, behavior, and TS classification parity."""

import json
import re
import sys
from pathlib import Path


SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
UE_PROTOCOL_ROOT = (
    ROOT_DIR
    / "Source"
    / "ForbocAI_SDK"
    / "Public"
    / "Features"
    / "Protocol"
)
UE_DATA_ROOT = ROOT_DIR / "Content" / "Data"
TS_DATA_ROOT = ROOT_DIR.parent / "sdk" / "packages" / "core" / "data"


def discover_classifications(root: Path) -> tuple[Path | None, dict[str, str]]:
    if not root.is_dir():
        return None, {}
    discovered: list[tuple[Path, dict[str, str]]] = []
    for path in sorted(root.rglob("*.json")):
        try:
            value = json.loads(path.read_text(encoding="utf-8"))
        except (json.JSONDecodeError, OSError):
            continue
        classifications = value.get("classifications") if isinstance(value, dict) else None
        if isinstance(classifications, dict) and classifications and all(
            isinstance(key, str) and isinstance(item, str)
            for key, item in classifications.items()
        ):
            discovered.append((path, classifications))
    return discovered[0] if len(discovered) == 1 else (None, {})


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


def discover_handler(function_name: str) -> list[tuple[Path, str]]:
    discovered: list[tuple[Path, str]] = []
    for path in sorted(UE_PROTOCOL_ROOT.rglob("*.h")):
        body = extract_function_body(path.read_text(encoding="utf-8"), function_name)
        if body is not None:
            discovered.append((path, body))
    return discovered


def main() -> int:
    ue_contract_path, ue_classifications = discover_classifications(UE_DATA_ROOT)
    if ue_contract_path is None:
        print(
            f"[FAIL] Expected exactly one UE classification contract under "
            f"{UE_DATA_ROOT}."
        )
        return 1

    failures = 0
    required_symbols = {
        "IdentifyActor": "SerializeIdentifyActorResult",
        "QueryVector": "SerializeQueryVectorResult",
        "Decision": "SerializeDecisionResult",
        "Reasoning": "SerializeReasoningResult",
        "Finalize": "BuildAgentResponse",
    }

    for instruction, classification in ue_classifications.items():
        function_name = f"Handle{instruction}"
        handlers = discover_handler(function_name)
        if len(handlers) != 1:
            print(
                f"[FAIL] Expected exactly one {function_name} definition; "
                f"found {len(handlers)}."
            )
            failures += 1
            continue

        path, body = handlers[0]
        if classification not in {"Local", "Pass-through"}:
            print(
                f"[FAIL] {instruction} has unsupported classification "
                f"{classification!r} in {ue_contract_path}."
            )
            failures += 1
            continue

        forbidden_local_inference = ("CompleteInference", "nodeCortexThunk")
        if classification == "Pass-through" and any(
            symbol in body for symbol in forbidden_local_inference
        ):
            print(f"[FAIL] {function_name} uses local inference in {path}.")
            failures += 1
        elif classification == "Pass-through":
            print(f"[OK] {function_name} stays clear of local inference ({path}).")

        required = required_symbols.get(instruction)
        if required is None:
            print(f"[FAIL] No result contract is defined for {instruction}.")
            failures += 1
        elif required not in body:
            print(f"[FAIL] {function_name} does not call {required} in {path}.")
            failures += 1
        else:
            print(f"[OK] {function_name} returns through {required} ({path}).")

    if ue_classifications.get("Decision") == "Pass-through":
        decision_root = UE_PROTOCOL_ROOT / "Instructions" / "Decision"
        decision_content = "\n".join(
            path.read_text(encoding="utf-8")
            for path in sorted(decision_root.rglob("*.h"))
        )
        forbidden_decision_policy = (
            "BuildDecisionIntent",
            "DecisionAdapters",
            "NPCSelectors::selectNPCHistory",
            "sdk_decision_policy",
        )
        discovered_policy = [
            symbol for symbol in forbidden_decision_policy
            if symbol in decision_content
        ]
        if discovered_policy:
            print(
                "[FAIL] Pass-through Decision handlers contain SDK-owned "
                f"decision policy: {', '.join(discovered_policy)}."
            )
            failures += 1
        else:
            print("[OK] Decision policy remains API-owned.")

    ts_contract_path, ts_classifications = discover_classifications(TS_DATA_ROOT)
    if ts_contract_path is None:
        print(
            f"[FAIL] Expected exactly one TS classification contract under "
            f"{TS_DATA_ROOT}."
        )
        failures += 1
    elif ue_classifications != ts_classifications:
        print("[FAIL] UE and TS classification tables diverge.")
        print(f"  UE ({ue_contract_path}): {ue_classifications}")
        print(f"  TS ({ts_contract_path}): {ts_classifications}")
        failures += 1
    else:
        print(f"[OK] UE and TS classification contracts match ({ts_contract_path}).")

    if failures:
        print(f"\nFailed {failures} handler classification checks.")
        return 1
    print("\nAll handler classification checks passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
