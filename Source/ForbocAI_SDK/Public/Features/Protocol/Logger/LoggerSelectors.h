#pragma once

#include "Core/rtk.hpp"
#include "Features/Bridge/BridgeSelectors.h"
#include "Features/Dependencies/DependenciesTypes.h"
#include "Features/Directive/DirectiveSlice.h"
#include "Features/Ghost/GhostSlice.h"
#include "Features/Memory/MemorySelectors.h"
#include "Features/NPC/NPCSlice.h"
#include "Features/Protocol/Logger/Configuration/LoggerConfigurationAdapters.h"
#include "Features/Soul/SoulSlice.h"
#include "Features/Vector/VectorTypes.h"

namespace LoggerSelectors {
namespace Internal {

using FSummaryField = TPair<FString, FString>;

/** User Story: As protocol logging, I need booleans rendered through the authored logger vocabulary so platform output remains identical. @fn inline FString booleanText(bool Value) */
inline FString booleanText(bool Value) {
  const auto &Data = LoggerConfiguration::loggerData();
  return Value ? Data.Boolean.True : Data.Boolean.False;
}

/** User Story: As protocol logging, I need state fields composed by one pure formatter so slice selectors only derive values. @fn inline FString summarizeFields(const TArray<FSummaryField> &Fields) */
inline FString summarizeFields(const TArray<FSummaryField> &Fields) {
  const auto &Data = LoggerConfiguration::loggerData();
  return FString::Join(
      func::map_array<FSummaryField, FString>(
          Fields, [&Data](const FSummaryField &Field) {
            return Field.Key + Data.Text.KeyValueSeparator + Field.Value;
          }),
      *Data.Text.FieldSeparator);
}

/** User Story: As protocol logging, I need NPC state summarized through authored field names so deltas remain stable across SDKs. @fn inline FString summarizeNPCState(const NPCSlice::FNPCSliceState &State) */
inline FString summarizeNPCState(const NPCSlice::FNPCSliceState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields({{Fields.NpcIds, LexToString(State.Entities.ids.Num())},
                          {Fields.NpcActive, State.ActiveNpcId}});
}

/**
 * User Story: As protocol logging, I need memory state summarized through authored field names so deltas expose persistence transitions.
 * @fn inline FString summarizeMemoryState( const MemorySlice::FMemorySliceState &State)
 */
inline FString summarizeMemoryState(
    const MemorySlice::FMemorySliceState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields(
      {{Fields.MemoryIds, LexToString(State.Entities.ids.Num())},
       {Fields.MemoryStore, State.StorageStatus},
       {Fields.MemoryRecall, State.RecallStatus},
       {Fields.MemoryRecalled, LexToString(State.RecalledIds.Num())},
       {Fields.MemoryError, State.Error}});
}

/**
 * User Story: As protocol logging, I need directive state summarized through authored field names so run lifecycle transitions are inspectable.
 * @fn inline FString summarizeDirectiveState( const DirectiveSlice::FDirectiveSliceState &State)
 */
inline FString summarizeDirectiveState(
    const DirectiveSlice::FDirectiveSliceState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields(
      {{Fields.DirectiveIds, LexToString(State.Entities.ids.Num())},
       {Fields.DirectiveActive, State.ActiveDirectiveId}});
}

/**
 * User Story: As protocol logging, I need bridge state summarized through authored field names so validation state remains auditable.
 * @fn inline FString summarizeBridgeState( const BridgeSlice::FBridgeSliceState &State)
 */
inline FString summarizeBridgeState(
    const BridgeSlice::FBridgeSliceState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields(
      {{Fields.BridgeStatus, BridgeSelectors::selectBridgeStatus(State)},
       {Fields.BridgePresets,
        LexToString(BridgeSelectors::selectActivePresets(State).Num())},
       {Fields.BridgeRulesets,
        LexToString(BridgeSelectors::selectAvailableRulesets(State).Num())},
       {Fields.BridgePresetIds, LexToString(State.AvailablePresetIds.Num())},
       {Fields.BridgeValidated, booleanText(State.bHasValidationResult)},
       {Fields.BridgeError, State.Error}});
}

/** User Story: As protocol logging, I need Soul state summarized through authored field names so import and export transitions remain visible. @fn inline FString summarizeSoulState(const SoulSlice::FSoulSliceState &State) */
inline FString summarizeSoulState(const SoulSlice::FSoulSliceState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields(
      {{Fields.SoulExport, State.ExportStatus},
       {Fields.SoulImport, State.ImportStatus},
       {Fields.SoulHasExport, booleanText(State.bHasExportResult)},
       {Fields.SoulHasImport, booleanText(State.bHasImportedSoul)},
       {Fields.SoulAvailable, LexToString(State.AvailableSouls.Num())},
       {Fields.SoulError, State.Error}});
}

/** User Story: As protocol logging, I need ghost state summarized through authored field names so session progress remains inspectable. @fn inline FString summarizeGhostState(const GhostSlice::FGhostSliceState &State) */
inline FString summarizeGhostState(const GhostSlice::FGhostSliceState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields(
      {{Fields.GhostSession, State.ActiveSessionId},
       {Fields.GhostStatus, State.Status},
       {Fields.GhostProgress, LexToString(State.Progress)},
       {Fields.GhostHasResults, booleanText(State.bHasResults)},
       {Fields.GhostHistory, LexToString(State.History.Num())},
       {Fields.GhostLoading, booleanText(State.bLoading)},
       {Fields.GhostError, State.Error}});
}

/** User Story: As protocol logging, I need vector state summarized through authored field names so readiness transitions remain visible. @fn inline FString summarizeVectorState(const FVectorState &State) */
inline FString summarizeVectorState(const FVectorState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields({{Fields.VectorStatus, State.Status},
                          {Fields.VectorReady, booleanText(State.bIsReady)},
                          {Fields.VectorError, State.Error}});
}

/** User Story: As protocol logging, I need dependency state summarized through authored field names so setup outcomes remain auditable. @fn inline FString summarizeDependenciesState(const FDependenciesState &State) */
inline FString summarizeDependenciesState(const FDependenciesState &State) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields(
      {{Fields.DependenciesStatus, State.Status},
       {Fields.DependenciesReport, booleanText(State.bHasReport)},
       {Fields.DependenciesResult, booleanText(State.bHasResult)},
       {Fields.DependenciesError, State.Error}});
}

/** User Story: As protocol logging, I need extension state summarized through authored field names so host reducers participate in deltas. @fn inline FString summarizeExtraState(const TMap<FString, FString> &Extra) */
inline FString summarizeExtraState(const TMap<FString, FString> &Extra) {
  const auto &Fields = LoggerConfiguration::loggerData().Fields;
  return summarizeFields({{Fields.ExtraEntries, LexToString(Extra.Num())}});
}

/** User Story: As protocol logging, I need changed slice summaries appended with one authored delta grammar so every state domain renders consistently. @fn inline void appendDeltaIfChanged(TArray<FString> &Changes, const FString &Label, const FString &Before, const FString &After) */
inline void appendDeltaIfChanged(TArray<FString> &Changes,
                                 const FString &Label,
                                 const FString &Before,
                                 const FString &After) {
  const auto &Delta = LoggerConfiguration::loggerData().Delta;
  Before == After
      ? void()
      : (Changes.Add(Label + Delta.Open + Before + Delta.Transition + After +
                     Delta.Close),
         void());
}

} // namespace Internal

/** User Story: As protocol middleware, I need action selection driven by authored prefixes so TS and UE observe the same RTK events. @fn inline bool selectIsProtocolAction(const rtk::AnyAction &Action) */
inline bool selectIsProtocolAction(const rtk::AnyAction &Action) {
  return func::any_array<FString>(
      LoggerConfiguration::loggerData().ActionPrefixes,
      [&Action](const FString &Prefix) { return Action.Type.StartsWith(Prefix); });
}

/** User Story: As protocol middleware, I need one pure state-delta selector so logging remains outside reducers and thunks. @fn template <typename State> inline FString describeStateDelta(const State &Before, const State &After) */
template <typename State>
inline FString describeStateDelta(const State &Before, const State &After) {
  const auto &Data = LoggerConfiguration::loggerData();
  TArray<FString> Changes;
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Npcs, Internal::summarizeNPCState(Before.NPCs),
      Internal::summarizeNPCState(After.NPCs));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Memory,
      Internal::summarizeMemoryState(Before.Memory),
      Internal::summarizeMemoryState(After.Memory));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Directives,
      Internal::summarizeDirectiveState(Before.Directives),
      Internal::summarizeDirectiveState(After.Directives));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Bridge,
      Internal::summarizeBridgeState(Before.Bridge),
      Internal::summarizeBridgeState(After.Bridge));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Soul, Internal::summarizeSoulState(Before.Soul),
      Internal::summarizeSoulState(After.Soul));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Ghost, Internal::summarizeGhostState(Before.Ghost),
      Internal::summarizeGhostState(After.Ghost));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Vector,
      Internal::summarizeVectorState(Before.Vector),
      Internal::summarizeVectorState(After.Vector));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Dependencies,
      Internal::summarizeDependenciesState(Before.Dependencies),
      Internal::summarizeDependenciesState(After.Dependencies));
  Internal::appendDeltaIfChanged(
      Changes, Data.Labels.Extra, Internal::summarizeExtraState(Before.Extra),
      Internal::summarizeExtraState(After.Extra));
  return Changes.IsEmpty() ? Data.Delta.None
                           : FString::Join(Changes, *Data.Delta.Separator);
}

} // namespace LoggerSelectors
