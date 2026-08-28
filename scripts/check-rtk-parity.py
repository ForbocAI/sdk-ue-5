#!/usr/bin/env python3
"""Compare the UE RTK public surface with installed Redux Toolkit exports."""

from __future__ import annotations

import argparse
import sys

from parity.rtk.run import run


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ts-sdk-root", help="Path to the TS SDK repository root.")
    args = parser.parse_args()
    return run(args.ts_sdk_root)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"[fail] {error}", file=sys.stderr)
        raise SystemExit(1)
