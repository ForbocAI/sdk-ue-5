#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SDK_ROOT=$(git -C "$SCRIPT_DIR/../.." rev-parse --show-toplevel)
DEMO_ROOT=${1:-$(git -C "$SDK_ROOT" config --get forboc.demoPath || true)}

if [ -z "$DEMO_ROOT" ]; then
  exit 0
fi

if [ ! -d "$DEMO_ROOT" ]; then
  printf '%s\n' "[forboc] demo path not found: $DEMO_ROOT"
  exit 0
fi

SUBMODULE_PATH=Plugins/ForbocAI_SDK
SUBMODULE_ROOT=$DEMO_ROOT/$SUBMODULE_PATH
if [ ! -d "$SUBMODULE_ROOT" ]; then
  printf '%s\n' "[forboc] demo SDK submodule not found: $SUBMODULE_ROOT"
  exit 1
fi

if ! git -C "$SUBMODULE_ROOT" diff --quiet ||
   ! git -C "$SUBMODULE_ROOT" diff --cached --quiet; then
  printf '%s\n' "[forboc] refusing to replace a dirty demo SDK submodule"
  exit 1
fi

SDK_HEAD=$(git -C "$SDK_ROOT" rev-parse HEAD)
git -C "$SUBMODULE_ROOT" fetch --quiet "$SDK_ROOT" "$SDK_HEAD"
git -C "$SUBMODULE_ROOT" checkout --quiet --detach "$SDK_HEAD"

if git -C "$DEMO_ROOT" diff --quiet HEAD -- "$SUBMODULE_PATH"; then
  exit 0
fi

# Commit only the gitlink so unrelated demo staging remains untouched.
git -C "$DEMO_ROOT" commit --only -m "chore: update SDK submodule to latest" -- \
  "$SUBMODULE_PATH"
