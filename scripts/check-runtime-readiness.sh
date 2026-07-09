#!/usr/bin/env bash
set -euo pipefail

if [ -z "${FORBOCAI_API_URL:-}" ]; then
  echo "[skip] FORBOCAI_API_URL is not set. Skipping runtime-readiness check."
  exit 0
fi

URL="${FORBOCAI_API_URL%/}/status"
echo "Checking API status at $URL..."

STATUS_CODE=$(curl -s -o /dev/null -w "%{http_code}" "$URL" || true)

if [ "$STATUS_CODE" -eq 200 ]; then
  echo "[ok] Runtime API is reachable and healthy."
  exit 0
else
  echo "[fail] Runtime API check failed (HTTP $STATUS_CODE)."
  exit 1
fi
