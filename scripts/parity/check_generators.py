#!/usr/bin/env python3
"""Run source-derived parity generator regression tests."""

from __future__ import annotations

from pathlib import Path
import sys
import unittest


SCRIPTS_ROOT = Path(__file__).resolve().parents[1]
if str(SCRIPTS_ROOT) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_ROOT))


def main() -> int:
    """User Story: Verify every parity generator through one portable command.

    Signature: () -> int.
    """
    suite = unittest.defaultTestLoader.discover(
        str(Path(__file__).resolve().parent),
        pattern="test_*.py",
    )
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
