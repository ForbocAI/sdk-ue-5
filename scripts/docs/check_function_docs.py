#!/usr/bin/env python3
"""Require Doxygen user stories and exact @fn signatures for every UE function."""

from __future__ import annotations

from pathlib import Path
import sys


SCRIPT_DIR = Path(__file__).resolve().parent
SCRIPTS_DIR = SCRIPT_DIR.parent
sys.path.insert(0, str(SCRIPT_DIR))
sys.path.insert(0, str(SCRIPTS_DIR))

from cpp_function_docs import (  # noqa: E402
    collect_function_targets,
    first_party_cpp_files,
    inspect_function_docs,
)
from ue_targets import ue_source_roots  # noqa: E402


def main() -> int:
    files = first_party_cpp_files(ue_source_roots())
    targets = sum(len(collect_function_targets(path)) for path in files)
    findings = [
        finding
        for path in files
        for finding in inspect_function_docs(path)
    ]
    if findings:
        print(
            "Function documentation check failed: "
            f"{len(findings)} of {targets} function declaration(s)/definition(s) "
            "lack an attached User Story or exact @fn signature."
        )
        for finding in findings:
            missing = ", ".join(finding.missing)
            print(f"{finding.path}:{finding.line}: {finding.name}: {missing}")
        print("Run scripts/docs/sync_function_docs.py and review the generated stories.")
        return 1
    print(
        "Function documentation check passed: "
        f"{targets} declaration(s)/definition(s) across {len(files)} C++ files."
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
