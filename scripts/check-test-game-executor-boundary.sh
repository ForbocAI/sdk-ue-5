#!/usr/bin/env bash
# check-test-game-executor-boundary.sh
#
# Coverage-script guard for the UE test-game executor boundary.
#
# This check fails if any code re-introduces an in-process command executor
# under the test-game surface. The retired `TestGameLib.h` header used to
# host `ExecuteForbocAICommand`, a shadow CLI that bypassed CLIOps; that
# entire header is retired (see ForbocAI/sdk-ue-5#5). All UE test-game
# command execution must flow through the Harness `CommandRunnerThunks` role,
# which delegates to the canonical `CLIOps::DispatchCommand`.
#
# Rules enforced:
#   1. No file may include the retired `TestGame/TestGameLib.h` header.
#   2. No file may reintroduce `TestGameLib.h` (the file itself).
#   3. Integration tests under the SDK and UE test-game modules must
#      not name a function `ExecuteForbocAICommand` or shadow the
#      canonical command surface with an alternate executor symbol.
#
# Usage:
#   bash scripts/check-test-game-executor-boundary.sh
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
TEST_GAME_SRC="$ROOT/test-game-cli/Source/ForbocAI_TestGame_CLI"
STATUS=0

normalize_crlf() {
  tr -d '\r'
}

echo "[check] Test-game executor boundary guard"

# 1) No #include of the retired TestGameLib.h header.
LIB_INCLUDES="$(rg -n '#include[[:space:]]+"[^"]*TestGame/TestGameLib\.h"' \
  "$SRC/Public" "$SRC/Private" "$TEST_GAME_SRC/Public" "$TEST_GAME_SRC/Private" \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$LIB_INCLUDES" ]; then
  echo "[fail] Files still include the retired TestGame/TestGameLib.h:"
  echo "$LIB_INCLUDES"
  echo "       Include TestGame/Features/Systems/Harness/HarnessThunks.h for runtime-URL helpers" >&2
  echo "       or TestGame/Views/Terminal/TerminalView.h for ASCII rendering." >&2
  echo "       All command execution must use TestGame::CommandRunner." >&2
  STATUS=1
else
  echo "[ok] No TestGame/TestGameLib.h includes"
fi

# 2) The retired header itself must not be re-added.
if [ -f "$SRC/Public/TestGame/TestGameLib.h" ] || [ -f "$TEST_GAME_SRC/Public/TestGame/TestGameLib.h" ]; then
  echo "[fail] TestGame/TestGameLib.h has been re-added. The retired" >&2
  echo "       in-process executor surface is retired — split helpers" >&2
  echo "       into Features/Systems/Harness/HarnessThunks.h / Views/Terminal/TerminalView.h instead." >&2
  STATUS=1
else
  echo "[ok] Retired TestGameLib.h is absent"
fi

# 3) Integration tests must not reintroduce an executor symbol.
TEST_DIRS=()
[ -d "$SRC/Private/Tests" ] && TEST_DIRS+=("$SRC/Private/Tests")
[ -d "$TEST_GAME_SRC/Private/Tests" ] && TEST_DIRS+=("$TEST_GAME_SRC/Private/Tests")
if [ "${#TEST_DIRS[@]}" -gt 0 ]; then
  EXEC_DECLS="$(rg -n '\b(ExecuteForbocAICommand|FCommandExecutor)\b' \
    "${TEST_DIRS[@]}" \
    2>/dev/null | normalize_crlf || true)"
  if [ -n "$EXEC_DECLS" ]; then
    echo "[fail] Integration tests reference a retired test-game executor entrypoint:"
    echo "$EXEC_DECLS"
    echo "       Drive commands through TestGame::CommandRunner::Execute." >&2
    STATUS=1
  else
    echo "[ok] Integration tests do not reintroduce test-game executor hooks"
  fi
fi

# 4) The canonical SDK CLI dispatch may be called only from the one
# CommandRunner thunk boundary in the test-game program.
CLI_DISPATCHES="$(rg -l '\bCLIOps::DispatchCommand\b' \
  "$TEST_GAME_SRC/Public" "$TEST_GAME_SRC/Private" 2>/dev/null | \
  normalize_crlf || true)"
EXPECTED_DISPATCH="$TEST_GAME_SRC/Public/TestGame/Features/Systems/Harness/CommandRunner/CommandRunnerThunks.h"
if [ "$CLI_DISPATCHES" != "$EXPECTED_DISPATCH" ]; then
  echo "[fail] Test-game CLI dispatch is not isolated to CommandRunnerThunks.h:" >&2
  printf '%s\n' "$CLI_DISPATCHES" >&2
  STATUS=1
else
  echo "[ok] Canonical CLI dispatch is isolated to CommandRunnerThunks.h"
fi

# 5) Test-game code must not own transport or an API endpoint. Contract
# bootstrap is an SDK CLI command like every other scenario operation.
DIRECT_TRANSPORT="$(rg -n '\b(fetchBaseQuery|FHttpModule|IHttpRequest|getTestGameContractThunk|ContractApi)\b' \
  "$TEST_GAME_SRC/Public" "$TEST_GAME_SRC/Private" 2>/dev/null | \
  normalize_crlf || true)"
if [ -n "$DIRECT_TRANSPORT" ]; then
  echo "[fail] Test-game code owns SDK/API transport:" >&2
  echo "$DIRECT_TRANSPORT" >&2
  STATUS=1
else
  echo "[ok] Test-game code owns no API transport"
fi

CONTRACT_COMMAND="$ROOT/test-game-cli/Content/Data/harness/game.json"
if ! rg -q '"command"[[:space:]]*:[[:space:]]*"forbocai contract"' \
  "$CONTRACT_COMMAND"; then
  echo "[fail] Contract bootstrap does not use the SDK CLI command." >&2
  STATUS=1
else
  echo "[ok] Contract bootstrap uses the SDK CLI"
fi

echo "[done] Test-game executor boundary check complete (exit $STATUS)"
exit "$STATUS"
