#!/usr/bin/env python3
import sys
from pathlib import Path

def main():
    if len(sys.argv) < 2:
        print("Usage: check-codec-parity.py <path-to-ProtocolThunks.h>")
        sys.exit(1)

    protocol_thunks_path = Path(sys.argv[1])
    if not protocol_thunks_path.exists():
        print(f"[FAIL] Could not find {protocol_thunks_path}")
        sys.exit(1)

    content = protocol_thunks_path.read_text(encoding="utf-8")
    failures = []

    # 1. DecisionResult nested tape fields
    if "SerializeDecisionResult" in content:
        if "SetObjectField(TEXT(\"decisionIntent\")" not in content:
            failures.append("SerializeDecisionResult flattened decisionIntent! It must be nested via SetObjectField.")
    else:
        failures.append("SerializeDecisionResult not found.")

    # 2. ReasoningResult nested tape fields
    if "SerializeReasoningResult" in content:
        if "SetObjectField(TEXT(\"reasoningOutput\")" not in content:
            failures.append("SerializeReasoningResult flattened reasoningOutput! It must be nested via SetObjectField.")
    else:
        failures.append("SerializeReasoningResult not found.")

    # 3. IdentifyActorResult nested tape fields
    if "SerializeIdentifyActorResult" in content:
        if "SetObjectField(TEXT(\"actor\")" not in content:
            failures.append("SerializeIdentifyActorResult flattened actor data! It must be nested via SetObjectField.")
    else:
        failures.append("SerializeIdentifyActorResult not found.")

    if failures:
        print("\n".join(f"[FAIL] {f}" for f in failures))
        sys.exit(1)
    
    print("[OK] Protocol codecs preserve nested tape fields (Decision, Reasoning, IdentifyActor).")

if __name__ == "__main__":
    main()
