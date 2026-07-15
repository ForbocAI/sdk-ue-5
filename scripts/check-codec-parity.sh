#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROTOCOL_ROOT="$SCRIPT_DIR/../Source/ForbocAI_SDK/Public/Features/Protocol"

echo "Checking protocol codec parity..."
python3 "$SCRIPT_DIR/check-codec-parity.py" "$PROTOCOL_ROOT"
