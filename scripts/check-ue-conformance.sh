#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$ROOT/Source/ForbocAI_SDK"
SDK_CLI_SRC="$ROOT/micro-game-cli/Source/ForbocAI_MicroGame_CLI"
DEMO_SRC="$ROOT/../demo-ue-5/Source"
STATUS=0

# Hard dependency: without ripgrep every rule below silently produces no
# hits and the script reports a false "PASS". Fail loudly instead.
if ! command -v rg >/dev/null 2>&1; then
  echo "[fail] ripgrep (rg) is required but not found on PATH." >&2
  echo "       Install ripgrep before running the UE conformance check." >&2
  exit 2
fi

normalize_crlf() {
  tr -d '\r'
}

existing_dirs() {
  local dirs=()
  local dir
  for dir in "$@"; do
    [ -d "$dir" ] && dirs+=("$dir")
  done
  printf '%s\n' "${dirs[@]}"
}

mapfile -t FIRST_PARTY_ROOTS < <(existing_dirs \
  "$SRC/Public" "$SRC/Private" \
  "$SDK_CLI_SRC/Public" "$SDK_CLI_SRC/Private" \
  "$DEMO_SRC")
mapfile -t PRIVATE_ROOTS < <(existing_dirs "$SRC/Private" "$SDK_CLI_SRC/Private" "$DEMO_SRC")
mapfile -t PUBLIC_ROOTS < <(existing_dirs "$SRC/Public" "$SDK_CLI_SRC/Public" "$DEMO_SRC")

echo "[check] UE SDK conformance guardrails"

if PYTHONDONTWRITEBYTECODE=1 python3 "$ROOT/scripts/cpp/source_structure.py"; then
  echo "[ok] C++ namespaces, braces, header guards, and companion includes"
else
  echo "[fail] C++ source structure violations found"
  STATUS=1
fi

# 1a) No C++17 features in first-party source (excluding ThirdParty).
C17_HITS="$(rg -n 'if constexpr|std::is_same_v|std::decay_t|std::optional|std::variant|std::any' \
  "${FIRST_PARTY_ROOTS[@]}" \
  --glob '!**/Tests/**' \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$C17_HITS" ]; then
  echo "[fail] C++17 features found in first-party source:"
  echo "$C17_HITS"
  STATUS=1
else
  echo "[ok] No C++17 features in first-party source"
fi

# 1b) No C++14 auto return type deduction in first-party source.
C14_AUTO="$(rg -n 'inline auto [A-Za-z]' \
  "${FIRST_PARTY_ROOTS[@]}" \
  --glob '!**/Tests/**' \
  --glob '!**/ThirdParty/**' \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$C14_AUTO" ]; then
  echo "[fail] C++14 inline auto return deduction found in first-party source:"
  echo "$C14_AUTO"
  STATUS=1
else
  echo "[ok] No C++14 auto return deduction in first-party source"
fi

# 2) No raw new/delete in first-party runtime code (excluding Tests, ThirdParty).
RAW_NEW_CANDIDATES="$(rg -n '\bnew [A-Z]|\bdelete [A-Za-z]' \
  "${PRIVATE_ROOTS[@]}" \
  --glob '!**/Tests/**' \
  --glob '!**/Native/SqliteAmalgamation.c' \
  2>/dev/null | normalize_crlf || true)"
RAW_NEW=""
while IFS= read -r line; do
  [ -z "$line" ] && continue
  code="${line#*:*:}"
  trimmed="$(echo "$code" | sed 's/^[[:space:]]*//')"
  case "$trimmed" in
    \**|//*) continue ;;
  esac
  stripped="$(echo "$code" | sed 's|//.*||' | sed 's|/\*.*\*/||g')"
  if echo "$stripped" | rg -q '\bnew [A-Z]|\bdelete [A-Za-z]' 2>/dev/null; then
    RAW_NEW="${RAW_NEW}
${line}"
  fi
done <<< "$RAW_NEW_CANDIDATES"
RAW_NEW="$(echo "$RAW_NEW" | sed '/^$/d')"
if [ -n "$RAW_NEW" ]; then
  echo "[warn] raw new/delete found in first-party runtime code (tracked for remediation):"
  echo "$RAW_NEW"
else
  echo "[ok] No raw new/delete in first-party runtime code"
fi

# 3) No direct FHttpModule::Get().CreateRequest() outside approved adapter layer.
#    Approved locations (Public + Private):
#      Core/RTK/Query/   — canonical Redux Toolkit / RTK Query fetchBaseQuery implementation
#      System Thunks     — non-Forboc external transport (binary payloads, retries, custom timeouts)
#      BridgeModule.cpp  — lazy HTTP wrapper for bridge rules
#      Memory/Local/**/Adapters.cpp — binary download for native dependencies
DIRECT_HTTP="$(rg -n 'FHttpModule::Get\(\)\.CreateRequest\(\)' \
  "${FIRST_PARTY_ROOTS[@]}" \
  --glob '!**/Core/RTK/Query/**' \
  --glob '!**/Systems/Soul/SoulThunks.h' \
  --glob '!**/Bridge/BridgeModule.cpp' \
  --glob '!**/Systems/Memory/Local/**/*Adapters.cpp' \
  --glob '!**/Tests/**' \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$DIRECT_HTTP" ]; then
  echo "[fail] Direct HTTP request creation outside approved adapter layer:"
  echo "$DIRECT_HTTP"
  STATUS=1
else
  echo "[ok] No unapproved direct HTTP request creation"
fi

# 4) No class declarations in core FP zone (fp.hpp, rtk.hpp).
CORE_CLASSES="$(rg -n '^\s*class [A-Z]' \
  "$SRC/Public/Core/fp.hpp" \
  "$SRC/Public/Core/rtk.hpp" \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$CORE_CLASSES" ]; then
  echo "[warn] class declarations in core FP zone (documented exceptions expected):"
  echo "$CORE_CLASSES"
else
  echo "[ok] No class declarations in core FP zone"
fi

# 4b) The retired AsyncHttp adapter must not reappear. SDK-to-API transport
#     belongs to rtk.hpp's RTK Query fetchBaseQuery equivalent.
ASYNC_HTTP_HITS="$(rg -n 'AsyncHttp|Core/AsyncHttp' \
  "${FIRST_PARTY_ROOTS[@]}" \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$ASYNC_HTTP_HITS" ]; then
  echo "[fail] Retired AsyncHttp adapter reference found:"
  echo "$ASYNC_HTTP_HITS"
  STATUS=1
else
  echo "[ok] Retired AsyncHttp adapter is absent"
fi

# 5) No FPlatformProcess::CreateProc outside approved CLI/setup code.
#    Approved locations (Public + Private):
#      CLI/                         — CLI command handlers (build, host tools)
#      Systems/Dependencies/**/Adapters   — portable native dependency processes
#    The retired MicroGame/MicroGameLib.h scenario command runner was removed
#    in favor of MicroGame::CommandSurface — no micro-game exemption remains.
DIRECT_PROC="$(rg -n 'FPlatformProcess::CreateProc' \
  "${FIRST_PARTY_ROOTS[@]}" \
  --glob '!**/CLI/**' \
  --glob '!**/Systems/Dependencies/**/*Adapters.cpp' \
  --glob '!**/Tests/**' \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$DIRECT_PROC" ]; then
  echo "[fail] Direct process creation outside CLI/setup layer:"
  echo "$DIRECT_PROC"
  STATUS=1
else
  echo "[ok] No unapproved process creation"
fi

# 6) ThirdParty isolation — no direct ThirdParty includes in public headers.
THIRDPARTY_LEAK="$(rg -n '#include.*ThirdParty' \
  "${PUBLIC_ROOTS[@]}" \
  2>/dev/null | normalize_crlf || true)"
if [ -n "$THIRDPARTY_LEAK" ]; then
  echo "[fail] ThirdParty headers included directly in public headers:"
  echo "$THIRDPARTY_LEAK"
  STATUS=1
else
  echo "[ok] ThirdParty headers quarantined from public surface"
fi

# 7) No imperative branching (if/for/while/switch) in first-party non-test code.
#    Excludes: Tests/, comments, ThirdParty/, SqliteAmalgamation.c
IMPERATIVE_HITS="$(rg -n '\b(if|for|while|switch)\s*\(' \
  "${FIRST_PARTY_ROOTS[@]}" \
  --glob '!**/Tests/**' \
  --glob '!**/ThirdParty/**' \
  --glob '!**/Native/SqliteAmalgamation.c' \
  2>/dev/null | normalize_crlf | grep -v '^\s*//' | grep -v '^\s*\*' | grep -v '^\s*//.*\b(if|for|while|switch)\s*(' || true)"
# Filter out comment-only matches (lines starting with // or * after filename:line:)
IMPERATIVE_REAL=""
while IFS= read -r line; do
  [ -z "$line" ] && continue
  # Extract the code after filename:linenum:
  code="${line#*:*:}"
  # Skip block-comment continuation lines ( * ...) and line comments (// ...)
  trimmed="$(echo "$code" | sed 's/^[[:space:]]*//')"
  case "$trimmed" in
    \**|//*) continue ;;
  esac
  # Skip lines where the remaining match is inside an inline comment
  stripped="$(echo "$code" | sed 's|//.*||' | sed 's|/\*.*\*/||g')"
  if echo "$stripped" | rg -q '\b(if|for|while|switch)\s*\(' 2>/dev/null; then
    IMPERATIVE_REAL="$IMPERATIVE_REAL
$line"
  fi
done <<< "$IMPERATIVE_HITS"
IMPERATIVE_REAL="$(echo "$IMPERATIVE_REAL" | sed '/^$/d')"
if [ -n "$IMPERATIVE_REAL" ]; then
  echo "[fail] Imperative branching/loops in first-party non-test code:"
  echo "$IMPERATIVE_REAL"
  STATUS=1
else
  echo "[ok] No imperative branching in first-party non-test code"
fi

echo ""
echo "[done] UE conformance check complete (exit $STATUS)"
exit "$STATUS"
