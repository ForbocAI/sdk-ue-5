#pragma once

#include "CoreMinimal.h"
#include "Features/Bridge/BridgeSlice.h"
#include "Features/Dependencies/DependenciesTypes.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Ghost/GhostSlice.h"
#include "Features/Memory/MemorySlice.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Soul/SoulSlice.h"
#include "Features/Vector/VectorTypes.h"

namespace LoggingSelectors {

namespace Internal {

inline FString summarizeNPCState(const NPCSlice::FNPCSliceState &State) {
  return FString::Printf(TEXT("ids=%d active=%s"), State.Entities.ids.Num(),
                         *State.ActiveNpcId);
}

inline FString summarizeMemoryState(
    const MemorySlice::FMemorySliceState &State) {
  return FString::Printf(TEXT("ids=%d store=%s recall=%s recalled=%d error=%s"),
                         State.Entities.ids.Num(), *State.StorageStatus,
                         *State.RecallStatus, State.LastRecalledIds.Num(),
                         *State.Error);
}

inline FString summarizeDirectiveState(
    const DirectiveSlice::FDirectiveSliceState &State) {
  return FString::Printf(TEXT("ids=%d active=%s"), State.Entities.ids.Num(),
                         *State.ActiveDirectiveId);
}

inline FString summarizeBridgeState(
    const BridgeSlice::FBridgeSliceState &State) {
  return FString::Printf(
      TEXT("status=%s presets=%d rulesets=%d presetIds=%d validated=%s error=%s"),
      *State.Status, State.ActivePresets.Num(), State.AvailableRulesets.Num(),
      State.AvailablePresetIds.Num(),
      State.bHasLastValidation ? TEXT("true") : TEXT("false"), *State.Error);
}

inline FString summarizeSoulState(const SoulSlice::FSoulSliceState &State) {
  return FString::Printf(
      TEXT("export=%s import=%s hasExport=%s hasImport=%s available=%d error=%s"),
      *State.ExportStatus, *State.ImportStatus,
      State.bHasLastExport ? TEXT("true") : TEXT("false"),
      State.bHasLastImport ? TEXT("true") : TEXT("false"),
      State.AvailableSouls.Num(), *State.Error);
}

inline FString summarizeGhostState(const GhostSlice::FGhostSliceState &State) {
  return FString::Printf(
      TEXT("session=%s status=%s progress=%.2f hasResults=%s history=%d loading=%s error=%s"),
      *State.ActiveSessionId, *State.Status, State.Progress,
      State.bHasResults ? TEXT("true") : TEXT("false"), State.History.Num(),
      State.bLoading ? TEXT("true") : TEXT("false"), *State.Error);
}

inline FString summarizeVectorState(const FVectorState &State) {
  return FString::Printf(TEXT("status=%s ready=%s error=%s"), *State.Status,
                         State.bIsReady ? TEXT("true") : TEXT("false"),
                         *State.Error);
}

inline FString summarizeDependenciesState(const FDependenciesState &State) {
  return FString::Printf(TEXT("status=%s report=%s result=%s error=%s"),
                         *State.Status,
                         State.bHasReport ? TEXT("true") : TEXT("false"),
                         State.bHasResult ? TEXT("true") : TEXT("false"),
                         *State.Error);
}

inline FString summarizeExtraState(const TMap<FString, FString> &Extra) {
  return FString::Printf(TEXT("entries=%d"), Extra.Num());
}

inline void appendDeltaIfChanged(TArray<FString> &Changes,
                                 const FString &Label,
                                 const FString &Before,
                                 const FString &After) {
  Before == After
      ? void()
      : (Changes.Add(FString::Printf(TEXT("%s{%s -> %s}"), *Label, *Before,
                                     *After)),
         void());
}

} // namespace Internal

template <typename State>
inline FString describeStateDelta(const State &Before, const State &After) {
  TArray<FString> Changes;
  Internal::appendDeltaIfChanged(
      Changes, TEXT("NPCs"), Internal::summarizeNPCState(Before.NPCs),
      Internal::summarizeNPCState(After.NPCs));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Memory"), Internal::summarizeMemoryState(Before.Memory),
      Internal::summarizeMemoryState(After.Memory));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Directives"),
      Internal::summarizeDirectiveState(Before.Directives),
      Internal::summarizeDirectiveState(After.Directives));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Bridge"), Internal::summarizeBridgeState(Before.Bridge),
      Internal::summarizeBridgeState(After.Bridge));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Soul"), Internal::summarizeSoulState(Before.Soul),
      Internal::summarizeSoulState(After.Soul));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Ghost"), Internal::summarizeGhostState(Before.Ghost),
      Internal::summarizeGhostState(After.Ghost));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Vector"), Internal::summarizeVectorState(Before.Vector),
      Internal::summarizeVectorState(After.Vector));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Dependencies"),
      Internal::summarizeDependenciesState(Before.Dependencies),
      Internal::summarizeDependenciesState(After.Dependencies));
  Internal::appendDeltaIfChanged(
      Changes, TEXT("Extra"), Internal::summarizeExtraState(Before.Extra),
      Internal::summarizeExtraState(After.Extra));
  return Changes.Num() == 0 ? TEXT("<none>") : FString::Join(Changes, TEXT("; "));
}

} // namespace LoggingSelectors
