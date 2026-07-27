#!/usr/bin/env bash
# Runs the regression-tested, data-driven thin CLI boundary guard.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

python3 "$SCRIPT_DIR/cli/test_check_thin_wrapper.py"
python3 "$SCRIPT_DIR/cli/check_thin_wrapper.py"
