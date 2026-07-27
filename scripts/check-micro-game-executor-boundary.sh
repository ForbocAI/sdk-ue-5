#!/usr/bin/env bash
# check-micro-game-executor-boundary.sh
#
# Coverage-script guard for the UE micro-game executor boundary.
#
# This check fails if any code re-introduces an in-process command executor
# under the micro-game surface. The retired `MicroGameLib.h` header used to
# host `ExecuteForbocAICommand`, a shadow CLI that bypassed CLIOps; that
# entire header is retired (see ForbocAI/sdk-ue-5#5). All UE micro-game
# command execution must flow through the Harness `CommandRunnerThunks` role,
# which delegates to the canonical `CLIOps::DispatchCommand`.
#
# Rules enforced:
#   1. No file may include the retired `MicroGame/MicroGameLib.h` header.
#   2. No file may reintroduce `MicroGameLib.h` (the file itself).
#   3. Integration tests under the SDK and UE micro-game modules must
#      not name a function `ExecuteForbocAICommand` or shadow the
#      canonical command surface with an alternate executor symbol.
#   4. Micro-game ECS includes retain the nested `MicroGame/Features` root.
#   5. The authored-data adapter resolves through its canonical include path.
#
# Usage:
#   bash scripts/check-micro-game-executor-boundary.sh
#
# Exit codes:
#   0 — all rules satisfied
#   1 — at least one rule failed
#   2 — required tool missing
#
# This script is intentionally shell-portable (POSIX-friendly bash + rg)
# so it runs on Windows (Git Bash / WSL) and macOS without extra setup.

set -euo pipefail

if ! command -v rg >/dev/null 2>&1; then
  echo "[fail] ripgrep (rg) is required but not found on PATH." >&2
  echo "       Install ripgrep before running the executor-boundary guard." >&2
  exit 2
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/Source/ForbocAI_SDK"
MICRO_GAME_SRC="$ROOT/micro-game-cli/Source/ForbocAI_MicroGame_CLI"
STATUS=0

normalize_crlf() {
  tr -d '\r'
}

echo "[check] Micro-game executor boundary guard"

# 1) No #include of the retired MicroGameLib.h header.
LIB_INCLUDES="$(rg -n '#include[[:space:]]+"[^"]*MicroGame/MicroGameLib\.h"' \
  "$SRC/Public" "$SRC/Private" "$MICRO_GAME_SRC/Public" "$MICRO_GAME_SRC/Private" \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$LIB_INCLUDES" ]; then
  echo "[fail] Files still include the retired MicroGame/MicroGameLib.h:"
  echo "$LIB_INCLUDES"
  echo "       Include MicroGame/Features/Systems/Harness/HarnessThunks.h for runtime-URL helpers" >&2
  echo "       or MicroGame/Views/Terminal/TerminalView.h for ASCII rendering." >&2
  echo "       All command execution must use MicroGame::CommandRunner." >&2
  STATUS=1
else
  echo "[ok] No MicroGame/MicroGameLib.h includes"
fi

# 2) The retired header itself must not be re-added.
if [ -f "$SRC/Public/MicroGame/MicroGameLib.h" ] || [ -f "$MICRO_GAME_SRC/Public/MicroGame/MicroGameLib.h" ]; then
  echo "[fail] MicroGame/MicroGameLib.h has been re-added. The retired" >&2
  echo "       in-process executor surface is retired — split helpers" >&2
  echo "       into Features/Systems/Harness/HarnessThunks.h / Views/Terminal/TerminalView.h instead." >&2
  STATUS=1
else
  echo "[ok] Retired MicroGameLib.h is absent"
fi

# 3) Integration tests must not reintroduce an executor symbol.
TEST_DIRS=()
[ -d "$SRC/Private/Tests" ] && TEST_DIRS+=("$SRC/Private/Tests")
[ -d "$MICRO_GAME_SRC/Private/Tests" ] && TEST_DIRS+=("$MICRO_GAME_SRC/Private/Tests")
if [ "${#TEST_DIRS[@]}" -gt 0 ]; then
  EXEC_DECLS="$(rg -n '\b(ExecuteForbocAICommand|FCommandExecutor)\b' \
    "${TEST_DIRS[@]}" \
    2>/dev/null | normalize_crlf || true)"
  if [ -n "$EXEC_DECLS" ]; then
    echo "[fail] Integration tests reference a retired micro-game executor entrypoint:"
    echo "$EXEC_DECLS"
    echo "       Drive commands through MicroGame::CommandRunner::Execute." >&2
    STATUS=1
  else
    echo "[ok] Integration tests do not reintroduce micro-game executor hooks"
  fi
fi

# 4) The canonical SDK CLI dispatch may be called only from the one
# CommandRunner thunk boundary in the micro-game program.
CLI_DISPATCHES="$(rg -l '\bCLIOps::DispatchCommand\b' \
  "$MICRO_GAME_SRC/Public" "$MICRO_GAME_SRC/Private" 2>/dev/null | \
  normalize_crlf || true)"
EXPECTED_DISPATCH="$MICRO_GAME_SRC/Public/MicroGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
if [ "$CLI_DISPATCHES" != "$EXPECTED_DISPATCH" ]; then
  echo "[fail] Micro-game CLI dispatch is not isolated to CommandRunnerThunks.h:" >&2
  printf '%s\n' "$CLI_DISPATCHES" >&2
  STATUS=1
else
  echo "[ok] Canonical CLI dispatch is isolated to CommandRunnerThunks.h"
fi

# 5) Micro-game code must not own transport or an API endpoint. Contract
# bootstrap is an SDK CLI command like every other scenario operation.
DIRECT_TRANSPORT="$(rg -n '\b(fetchBaseQuery|FHttpModule|IHttpRequest|getMicroGameContractThunk|ContractApi)\b' \
  "$MICRO_GAME_SRC/Public" "$MICRO_GAME_SRC/Private" 2>/dev/null | \
  normalize_crlf || true)"
if [ -n "$DIRECT_TRANSPORT" ]; then
  echo "[fail] Micro-game code owns SDK/API transport:" >&2
  echo "$DIRECT_TRANSPORT" >&2
  STATUS=1
else
  echo "[ok] Micro-game code owns no API transport"
fi

# 6) The SDK uses direct ECS roots, but the independently runnable micro-game
# owns one nested Features graph. A broad SDK path rewrite must not flatten it.
FLATTENED_ECS_INCLUDES="$(rg -n '#include[[:space:]]+"MicroGame/(Components|Entities|Systems)/' \
  "$MICRO_GAME_SRC/Public" "$MICRO_GAME_SRC/Private" 2>/dev/null | \
  normalize_crlf || true)"
if [ -n "$FLATTENED_ECS_INCLUDES" ]; then
  echo "[fail] Micro-game includes bypass the nested MicroGame/Features root:" >&2
  echo "$FLATTENED_ECS_INCLUDES" >&2
  STATUS=1
else
  echo "[ok] Micro-game ECS includes retain the nested Features root"
fi

# 7) The micro-game authored-data adapter is a sibling of the ECS roots. Keep
# callers on the real path so a broad SDK migration cannot invent a directory
# that still passes the namespace-shape check above.
CANONICAL_DATA_ADAPTER="$MICRO_GAME_SRC/Public/MicroGame/Features/Data/DataAdapters.h"
INVALID_DATA_INCLUDES="$(rg -n '#include[[:space:]]+"MicroGame/Features/Systems/Data/DataAdapters\.h"' \
  "$MICRO_GAME_SRC/Public" "$MICRO_GAME_SRC/Private" 2>/dev/null | \
  normalize_crlf || true)"
if [ ! -f "$CANONICAL_DATA_ADAPTER" ] || [ -n "$INVALID_DATA_INCLUDES" ]; then
  echo "[fail] Micro-game authored-data adapter does not resolve through Features/Data:" >&2
  echo "$INVALID_DATA_INCLUDES" >&2
  STATUS=1
else
  echo "[ok] Micro-game authored-data includes resolve through Features/Data"
fi

CONTRACT_COMMAND="$ROOT/micro-game-cli/Content/Data/harness/game.json"
if ! rg -q '"command"[[:space:]]*:[[:space:]]*"forbocai contract"' \
  "$CONTRACT_COMMAND"; then
  echo "[fail] Contract bootstrap does not use the SDK CLI command." >&2
  STATUS=1
else
  echo "[ok] Contract bootstrap uses the SDK CLI"
fi

echo "[done] Micro-game executor boundary check complete (exit $STATUS)"
exit "$STATUS"
