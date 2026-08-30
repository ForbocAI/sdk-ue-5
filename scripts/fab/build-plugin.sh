#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
STAGE_DIR="${FORBOCAI_FAB_STAGE_DIR:-$PROJECT_ROOT/dist/fab/stage/ForbocAI_SDK}"
OUTPUT_DIR="${FORBOCAI_FAB_BUILD_DIR:-$PROJECT_ROOT/dist/fab/build/ForbocAI_SDK}"
PLUGIN_PATH="$STAGE_DIR/ForbocAI_SDK.uplugin"
USE_CMD=0

if command -v python3 >/dev/null 2>&1; then
  PYTHON="$(command -v python3)"
elif command -v python >/dev/null 2>&1; then
  PYTHON="$(command -v python)"
else
  echo "Python 3 is required to prepare the Fab package." >&2
  exit 1
fi

"$PYTHON" "$SCRIPT_DIR/prepare_package.py"

if grep -qi microsoft /proc/version 2>/dev/null; then
  UE_ROOT="${UE_ROOT:-/mnt/c/Program Files/Epic Games/UE_5.8}"
  UAT_PATH="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.bat"
  USE_CMD=1
elif [[ "$(uname -s)" == "Darwin" ]]; then
  UE_ROOT="${UE_ROOT:-/Users/Shared/Epic Games/UE_5.8}"
  UAT_PATH="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
else
  UE_ROOT="${UE_ROOT:-/opt/UnrealEngine}"
  UAT_PATH="$UE_ROOT/Engine/Build/BatchFiles/RunUAT.sh"
fi

[[ -f "$PLUGIN_PATH" ]] || { echo "Prepared plugin descriptor not found: $PLUGIN_PATH" >&2; exit 1; }
[[ -f "$UAT_PATH" ]] || { echo "Unreal Automation Tool not found: $UAT_PATH" >&2; exit 1; }

rm -rf "$OUTPUT_DIR"
mkdir -p "$(dirname "$OUTPUT_DIR")"

if [[ "$USE_CMD" -eq 1 ]]; then
  UAT_WIN="$(wslpath -w "$UAT_PATH")"
  PLUGIN_WIN="$(wslpath -w "$PLUGIN_PATH")"
  OUTPUT_WIN="$(wslpath -w "$OUTPUT_DIR")"
  cmd.exe /c call "$UAT_WIN" BuildPlugin -Plugin="$PLUGIN_WIN" -Package="$OUTPUT_WIN" -Rocket
else
  "$UAT_PATH" BuildPlugin -Plugin="$PLUGIN_PATH" -Package="$OUTPUT_DIR" -Rocket
fi

"$PYTHON" "$SCRIPT_DIR/finalize_package.py" --package-root "$OUTPUT_DIR"
"$PYTHON" "$SCRIPT_DIR/validate_package.py" --package-root "$OUTPUT_DIR"
echo "Compiled Fab plugin package: $OUTPUT_DIR"
