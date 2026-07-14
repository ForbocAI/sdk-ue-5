#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTPUT=""

echo "Checking runtime readiness through the UE CLI and SDK default API..."

if ! OUTPUT="$(FORBOCAI_UE_SKIP_BUILD=1 "$SCRIPT_DIR/forbocai-ue" status 2>&1)"; then
  printf '%s\n' "$OUTPUT" >&2
  echo "[fail] UE CLI runtime-readiness command failed." >&2
  exit 1
fi

if ! grep -Fq "API: online" <<<"$OUTPUT"; then
  printf '%s\n' "$OUTPUT" >&2
  echo "[fail] UE CLI completed without reporting API: online." >&2
  exit 1
fi

echo "[ok] UE CLI -> SDK -> production API runtime path is healthy."
