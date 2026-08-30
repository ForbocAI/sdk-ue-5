#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if command -v python3 >/dev/null 2>&1; then
  PYTHON="$(command -v python3)"
elif command -v python >/dev/null 2>&1; then
  PYTHON="$(command -v python)"
else
  echo "Python 3 is required to release the Fab package." >&2
  exit 1
fi
"$SCRIPT_DIR/build-plugin.sh"
"$PYTHON" "$SCRIPT_DIR/archive_package.py"
"$PYTHON" "$SCRIPT_DIR/render_listing.py"
"$PYTHON" "$SCRIPT_DIR/generate_media.py"
"$PYTHON" "$SCRIPT_DIR/validate_listing.py"
