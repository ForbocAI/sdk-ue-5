#!/usr/bin/env bash
# check-product-boundary.sh
# Audits the UE SDK for game-specific terminology outside the separate test-game module.
#
# The SDK module should be game-agnostic. Scenario-specific language belongs in
# the separate test-game-cli/Source/ForbocAI_TestGame_CLI module.
#
# This script checks all non-TestGame headers for:
#   1. Game-domain framing (gameplay, game logic, combat system, etc.)
#   2. Scenario-specific references (doomguard, miller, stealth, etc.)
#   3. Test-game types leaking into generic surfaces
#   4. Harness-specific helpers exported as product APIs
#
# Run from the SDK plugin root:
#   bash scripts/check-product-boundary.sh

set -euo pipefail

# Hard dependency: without ripgrep every rule below silently produces no
# hits and the script reports a false "PASS". Fail loudly instead.
if ! command -v rg >/dev/null 2>&1; then
  echo "[FAIL] ripgrep (rg) is required but not found on PATH." >&2
  echo "       Install ripgrep before running the product boundary audit." >&2
  exit 2
fi

PLUGIN_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$PLUGIN_ROOT/Source/ForbocAI_SDK"
PUBLIC="$SRC/Public"
PRIVATE="$SRC/Private"

SRC_DIRS=("$PUBLIC")
[ -d "$PRIVATE" ] && SRC_DIRS+=("$PRIVATE")

VIOLATIONS=0

echo "=== Product Boundary Audit ==="
echo "Checking SDK surfaces for game-specific terminology..."
echo ""

# Define excluded paths. Test-game harness code is separate from the SDK module,
# but keep the glob so a reintroduced embedded TestGame folder is still ignored
# by terminology rules and caught by the dedicated boundary checks.
EXCLUDE_DIRS="--glob=!**/TestGame/**"
EXCLUDE_TESTS="--glob=!**/Tests/**"

# ── Rule 1: No game-domain framing in generic headers ──
echo "[Rule 1] No game-domain framing in the SDK module..."
GAME_TERMS="gameplay|game logic|game rules|game engine|combat system|RPG system|inventory system|quest system|leveling|skill tree|character class"
HITS=$(rg -ci "$GAME_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true)
if [ -n "$HITS" ]; then
  echo "  ✗ Game-domain framing found:"
  rg -ni "$GAME_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true
  VIOLATIONS=$((VIOLATIONS + 1))
else
  echo "  ✓ No game-domain framing in SDK surfaces."
fi
echo ""

# ── Rule 2: No scenario-specific references in generic headers ──
echo "[Rule 2] No scenario-specific references in the SDK module..."
SCENARIO_TERMS="doomguard|miller|stealth-door|social-miller|escape-realtime|persistence-recovery|Scout"
HITS=$(rg -ci "$SCENARIO_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true)
if [ -n "$HITS" ]; then
  echo "  ✗ Scenario-specific references found:"
  rg -ni "$SCENARIO_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true
  VIOLATIONS=$((VIOLATIONS + 1))
else
  echo "  ✓ No scenario-specific references in SDK surfaces."
fi
echo ""

# ── Rule 3: No TestGame types imported in generic headers ──
echo "[Rule 3] No TestGame type imports in generic headers..."
TG_IMPORTS="TestGame/TestGame|FTestGameState|FScenarioStep|FCommandSpec|ECommandGroup|FTranscriptEntry|ETranscriptStatus"
# Check CLI, Protocol, Core, Blueprint directories
GENERIC_DIRS=("$PUBLIC/CLI" "$PUBLIC/Protocol" "$PUBLIC/Core")
for dir in "${GENERIC_DIRS[@]}"; do
  [ -d "$dir" ] || continue
  HITS=$(rg -ci "$TG_IMPORTS" "$dir" 2>/dev/null || true)
  if [ -n "$HITS" ]; then
    echo "  ✗ TestGame types leaked into $(basename "$dir"):"
    rg -ni "$TG_IMPORTS" "$dir" 2>/dev/null || true
    VIOLATIONS=$((VIOLATIONS + 1))
  fi
done
echo ""

# ── Rule 4: Product terms used correctly ──
echo "[Rule 4] Verifying product-boundary terminology..."
echo "  Expected terms in generic SDK headers:"
echo "    - NPC decisioning, thought/context flow, rule validation"
echo "    - memory, soul/ghost, host-local execution"
echo "  Checking for correct usage..."

# Positive check: ensure key product terms exist somewhere in the generic surface
PRODUCT_TERMS=("AgentOps" "BridgeOps" "MemoryOps" "SoulOps" "GhostOps")
for term in "${PRODUCT_TERMS[@]}"; do
  if ! rg -q "$term" "${SRC_DIRS[@]}" $EXCLUDE_DIRS 2>/dev/null; then
    echo "  ⚠ Warning: Product term '$term' not found in generic headers"
  fi
done
echo ""

# ── Rule 5: No ASCII grid rendering outside TestGame ──
echo "[Rule 5] No rendering helpers in the SDK module..."
RENDER_TERMS="RenderGrid|RenderRow|CellAt|RenderLegend|ASCII grid"
HITS=$(rg -ci "$RENDER_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true)
if [ -n "$HITS" ]; then
  echo "  ✗ Rendering helpers found outside TestGame:"
  rg -ni "$RENDER_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true
  VIOLATIONS=$((VIOLATIONS + 1))
else
  echo "  ✓ No rendering helpers in SDK surfaces."
fi
echo ""

# ── Rule 6: No transcript/harness types outside TestGame ──
echo "[Rule 6] No transcript/harness types in the SDK module..."
HARNESS_TERMS="FTranscriptEntry|ETranscriptStatus|FHarnessState|FScenarioSliceState|EEventType"
HITS=$(rg -ci "$HARNESS_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true)
if [ -n "$HITS" ]; then
  echo "  ✗ Harness types found outside TestGame:"
  rg -ni "$HARNESS_TERMS" "${SRC_DIRS[@]}" $EXCLUDE_DIRS $EXCLUDE_TESTS 2>/dev/null || true
  VIOLATIONS=$((VIOLATIONS + 1))
else
  echo "  ✓ No harness types in SDK surfaces."
fi
echo ""

# ── Rule 7: No Simulated Coverage Claims ──
echo "[Rule 7] No simulated coverage claims..."
SIMULATED_TERMS="simulated coverage|simulated mode|mock coverage|simulated test"
HITS=$(rg -ci "$SIMULATED_TERMS" "${SRC_DIRS[@]}" 2>/dev/null || true)
if [ -n "$HITS" ]; then
  echo "  ✗ Simulated coverage claims found:"
  rg -ni "$SIMULATED_TERMS" "${SRC_DIRS[@]}" 2>/dev/null || true
  VIOLATIONS=$((VIOLATIONS + 1))
else
  echo "  ✓ No simulated coverage claims."
fi
echo ""

# ── Summary ──
echo "=== Results ==="
if [ "$VIOLATIONS" -eq 0 ]; then
  echo "✓ Product boundary is clean. No game-specific terminology in the SDK module."
  exit 0
else
  echo "✗ $VIOLATIONS boundary violation(s) found."
  echo "  Generic SDK surfaces should describe: NPC decisioning, thought/context flow,"
  echo "  rule validation, memory, soul/ghost, host-local execution."
  echo "  Scenario-specific language belongs in test-game-cli/Source/ForbocAI_TestGame_CLI."
  exit 1
fi
