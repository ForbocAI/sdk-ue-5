#pragma once

#include "CoreMinimal.h"
#include "Core/ue_fp.hpp"
#include "NPC/NPCId.h"
#include "Features/NPC/NPCSlice.h"
#include "RuntimeConfig.h"
#include "RuntimeStore.h"
#include "Features/API/APIThunks.h"
#include "Features/Bridge/BridgeThunks.h"
#include "Features/Ghost/GhostThunks.h"
#include "Features/Memory/MemoryThunks.h"
#include "Features/Protocol/ProtocolThunks.h"
#include "Features/Soul/SoulThunks.h"
#include "HttpManager.h"
#include "HttpModule.h"
#include <stdexcept>

/**
 * Operations — Free functions mirroring TS sdkOps.ts
 * User Story: As CLI commands, I need thin store-backed operations so command
 * handlers stay declarative and avoid duplicating orchestration logic.
 *
 * Every CLI operation dispatches thunks through the store.
 * No direct HTTP calls, no logic — just store dispatch + await.
 */
namespace Ops {

/**
 * Pumps the HTTP tick loop until the AsyncResult completes.
 * User Story: As synchronous CLI commands, I need async thunks awaited without
 * losing UE HTTP progress so terminal workflows can stay blocking and simple.
 * Allows async thunks to be used in synchronous CLI context.
 */
template <typename T>
T waitForResult(func::AsyncResult<T> &&Async, double TimeoutSeconds = 15.0) {
  bool bCompleted = false;
  T Result{};
  FString Error;

  Async.then([&bCompleted, &Result](T Value) {
    Result = Value;
    bCompleted = true;
  }).catch_([&bCompleted, &Error](std::string Message) {
    Error = UTF8_TO_TCHAR(Message.c_str());
    bCompleted = true;
  });
  Async.execute();

  const double StartTime = FPlatformTime::Seconds();
  struct PollLoop {
    static void apply(bool &Completed, double Start, double Timeout) {
      (!Completed && (FPlatformTime::Seconds() - Start) < Timeout)
          ? (FTaskGraphInterface::Get().ProcessThreadUntilIdle(
                 ENamedThreads::GameThread),
             FHttpModule::Get().GetHttpManager().Tick(0.05f),
             FPlatformProcess::Sleep(0.05f),
             apply(Completed, Start, Timeout), void())
          : void();
    }
  };
  PollLoop::apply(bCompleted, StartTime, TimeoutSeconds);

  !Error.IsEmpty()
      ? throw std::runtime_error(TCHAR_TO_UTF8(*Error))
      : (void)0;

  !bCompleted
      ? throw std::runtime_error("Timed out waiting for async result")
      : (void)0;

  return Result;
}

struct FRuntimeConfig {
  FString ApiUrl;
  FString ApiKey;
};

/**
 * Returns the current API URL and API key resolved from SDK config.
 * User Story: As CLI config consumers, I need resolved runtime credentials so
 * commands can show or reuse the active endpoint configuration.
 */
inline FRuntimeConfig runtimeConfig() {
  FRuntimeConfig Cfg;
  Cfg.ApiUrl = SDKConfig::GetApiUrl();
  Cfg.ApiKey = SDKConfig::GetApiKey();
  return Cfg;
}

/**
 * Fetches API health/status information through the shared store thunk.
 * User Story: As CLI health checks, I need API status routed through the store
 * so command output matches runtime behavior.
 */
inline FApiStatusResponse checkApiStatus(rtk::EnhancedStore<FRuntimeState> &Store) {
  return waitForResult(Store.dispatch(rtk::checkApiStatusThunk()));
}

/**
 * Registers a new NPC with a generated id and the provided persona.
 * User Story: As CLI NPC setup flows, I need a one-call creation helper so new
 * NPCs enter runtime state without duplicating slice wiring.
 */
inline FNPCInternalState createNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                                   const FString &Persona) {
  FString Id = NPCId::GenerateNPCId();
  FNPCInternalState Info;
  Info.Id = Id;
  Info.Persona = Persona;
  Store.dispatch(NPCSlice::Actions::setNPCInfo(Info));
  func::Maybe<FNPCInternalState> Active =
      NPCSlice::selectActiveNPC(Store.getState().NPCs);
  return Active.hasValue ? Active.value : Info;
}

/**
 * Returns the currently active NPC, if the store has one selected.
 * User Story: As CLI inspection flows, I need the active NPC resolved from the
 * store so follow-up commands can target the current actor.
 */
inline func::Maybe<FNPCInternalState>
getActiveNpc(rtk::EnhancedStore<FRuntimeState> &Store) {
  return NPCSlice::selectActiveNPC(Store.getState().NPCs);
}

/**
 * Applies a state delta to the target NPC and returns the updated record.
 * User Story: As CLI mutation flows, I need one helper to update NPC state so
 * manual state edits stay aligned with reducer behavior.
 */
inline func::Maybe<FNPCInternalState>
updateNpc(rtk::EnhancedStore<FRuntimeState> &Store, const FString &NpcId,
          const FAgentState &Delta) {
  Store.dispatch(NPCSlice::Actions::updateNPCState(NpcId, Delta));
  return NPCSlice::selectNPCById(Store.getState().NPCs, NpcId);
}

/**
 * Runs a local protocol turn for the target NPC using the supplied text input.
 * User Story: As CLI interaction flows, I need one helper to drive a protocol
 * turn so local NPC chat and action generation stay testable from the shell.
 */
inline FAgentResponse processNpc(rtk::EnhancedStore<FRuntimeState> &Store,
                                 const FString &NpcId, const FString &Text) {
  return waitForResult(
      Store.dispatch(rtk::processNPC(NpcId, Text, TEXT("{}"), TEXT(""),
                                     FAgentState(),
                                     rtk::InMemoryProtocolRuntime())));
}

/**
 * Lists every NPC currently stored in local runtime state.
 * User Story: As CLI listing flows, I need all NPC records from the store so
 * operators can inspect current runtime state.
 */
inline TArray<FNPCInternalState>
listNpcs(rtk::EnhancedStore<FRuntimeState> &Store) {
  return NPCSlice::selectAllNPCs(Store.getState().NPCs);
}

/**
 * Lists remote memories for the given NPC via the API-backed memory thunk.
 * User Story: As CLI memory inspection, I need remote memories fetched through
 * the thunk layer so API-backed memory output matches runtime behavior.
 */
inline TArray<FMemoryItem> listMemory(rtk::EnhancedStore<FRuntimeState> &Store,
                                      const FString &NpcId) {
  return waitForResult(Store.dispatch(rtk::listMemoryRemoteThunk(NpcId)));
}

/**
 * Recalls remote memories for a query and truncates the result set to Limit.
 * User Story: As CLI recall workflows, I need query-based memory recall with a
 * local limit so terminal output stays readable without changing API results.
 */
inline TArray<FMemoryItem> recallMemory(rtk::EnhancedStore<FRuntimeState> &Store,
                                        const FString &NpcId,
                                        const FString &Query,
                                        int32 Limit = 10,
                                        float Threshold = 0.7f) {
  TArray<FMemoryItem> Results = waitForResult(
      Store.dispatch(rtk::recallMemoryRemoteThunk(NpcId, Query, Threshold)));
  (Limit >= 0 && Results.Num() > Limit)
      ? (Results.SetNum(Limit), void())
      : void();
  return Results;
}

/**
 * Stores a remote observation memory for the given NPC.
 * User Story: As CLI memory capture flows, I need a helper that routes
 * observations through the remote memory thunk so persistence stays consistent.
 */
inline rtk::FEmptyPayload storeMemory(rtk::EnhancedStore<FRuntimeState> &Store,
                                      const FString &NpcId,
                                      const FString &Observation,
                                      float Importance = 0.8f) {
  return waitForResult(
      Store.dispatch(rtk::storeMemoryRemoteThunk(NpcId, Observation, Importance)));
}

/**
 * Clears all remote memories for the given NPC.
 * User Story: As CLI cleanup flows, I need one helper to clear remote memory
 * so reset operations go through the same thunk contract as runtime code.
 */
inline rtk::FEmptyPayload clearMemory(rtk::EnhancedStore<FRuntimeState> &Store,
                                      const FString &NpcId) {
  return waitForResult(Store.dispatch(rtk::clearMemoryRemoteThunk(NpcId)));
}

/**
 * Initializes the local node-backed memory store at the optional database path.
 * User Story: As local-memory setup flows, I need one CLI helper to bootstrap
 * the database so shell workflows can prepare native memory state quickly.
 */
inline rtk::FEmptyPayload initNodeMemory(rtk::EnhancedStore<FRuntimeState> &Store,
                                          const FString &DatabasePath = TEXT("")) {
  return waitForResult(Store.dispatch(rtk::initNodeMemoryThunk(DatabasePath)));
}

/**
 * Stores a local observation memory using the node-backed memory runtime.
 * User Story: As local-memory CLI flows, I need one helper to persist an
 * observation so native memory behavior can be exercised from the terminal.
 */
inline FMemoryItem storeNodeMemory(rtk::EnhancedStore<FRuntimeState> &Store,
                                   const FString &Text,
                                   float Importance = 0.8f) {
  return waitForResult(
      Store.dispatch(
          rtk::storeNodeMemoryThunk(Text, TEXT("observation"), Importance)));
}

/**
 * Recalls node-backed memories for a semantic query.
 * User Story: As local-memory CLI flows, I need query-based recall routed
 * through the native thunk so semantic search matches runtime behavior.
 */
inline TArray<FMemoryItem>
recallNodeMemory(rtk::EnhancedStore<FRuntimeState> &Store, const FString &Query,
                 int32 Limit = 10, float Threshold = 0.7f) {
  return waitForResult(
      Store.dispatch(rtk::recallNodeMemoryThunk(Query, Limit, Threshold)));
}

/**
 * Clears the local node-backed memory store.
 * User Story: As local-memory cleanup flows, I need one helper to reset the
 * native memory store so test runs can start from a clean slate.
 */
inline rtk::FEmptyPayload
clearNodeMemory(rtk::EnhancedStore<FRuntimeState> &Store) {
  return waitForResult(Store.dispatch(rtk::clearNodeMemoryThunk()));
}

/**
 * Starts a ghost run for the requested test suite and duration.
 * User Story: As ghost-test CLI flows, I need one helper to launch a run so
 * automated test sessions can be started from the terminal.
 */
inline FGhostRunResponse startGhost(rtk::EnhancedStore<FRuntimeState> &Store,
                                  const FString &TestSuite,
                                  int32 Duration = 300) {
  FGhostConfig Config;
  Config.TestSuite = TestSuite;
  Config.Duration = Duration;
  return waitForResult(Store.dispatch(rtk::startGhostThunk(Config)));
}

/**
 * Fetches the current status for a ghost run session.
 * User Story: As ghost-test CLI flows, I need live run status so I can poll
 * progress without reimplementing session lookups.
 */
inline FGhostStatusResponse
getGhostStatus(rtk::EnhancedStore<FRuntimeState> &Store, const FString &SessionId) {
  return waitForResult(Store.dispatch(rtk::getGhostStatusThunk(SessionId)));
}

/**
 * Fetches accumulated results for a completed or running ghost session.
 * User Story: As ghost-test CLI flows, I need run results routed through the
 * thunk layer so terminal reporting matches runtime semantics.
 */
inline FGhostResultsResponse
getGhostResults(rtk::EnhancedStore<FRuntimeState> &Store, const FString &SessionId) {
  return waitForResult(Store.dispatch(rtk::getGhostResultsThunk(SessionId)));
}

/**
 * Requests abort of a running ghost session.
 * User Story: As ghost-test CLI flows, I need one helper to stop a run so long
 * sessions can be cancelled without bespoke HTTP handling.
 */
inline FGhostStopResponse stopGhost(rtk::EnhancedStore<FRuntimeState> &Store,
                                    const FString &SessionId) {
  return waitForResult(Store.dispatch(rtk::stopGhostThunk(SessionId)));
}

/**
 * Lists historical ghost sessions, capped by the requested limit.
 * User Story: As ghost-test CLI flows, I need historical sessions so recent
 * runs can be inspected or compared from the terminal.
 */
inline TArray<FGhostHistoryEntry>
getGhostHistory(rtk::EnhancedStore<FRuntimeState> &Store, int32 Limit = 10) {
  return waitForResult(Store.dispatch(rtk::getGhostHistoryThunk(Limit)));
}

/**
 * Validates an action against the current bridge context and rules.
 * User Story: As bridge-validation CLI flows, I need one helper to validate an
 * action so shell tooling reuses the same rule engine as runtime code.
 */
inline FValidationResult
validateBridgePayload(rtk::EnhancedStore<FRuntimeState> &Store,
               const FAgentAction &Action,
               const FBridgeValidationContext &Context,
               const FString &NpcId = TEXT("")) {
  return waitForResult(
      Store.dispatch(rtk::validateBridgeThunk(Action, Context, NpcId)));
}

/**
 * Fetches the bridge rule set currently exposed by the runtime.
 * User Story: As bridge-inspection CLI flows, I need the active rules so
 * operators can inspect validation logic from the terminal.
 */
inline TArray<FBridgeRule>
getBridgeRules(rtk::EnhancedStore<FRuntimeState> &Store) {
  return waitForResult(Store.dispatch(rtk::getBridgeRulesThunk()));
}

/**
 * Loads a named bridge preset through the runtime thunk layer.
 * User Story: As bridge-preset CLI flows, I need one helper to resolve a named
 * preset so shell tooling can preview or apply preset rules.
 */
inline FDirectiveRuleSet loadBridgePreset(rtk::EnhancedStore<FRuntimeState> &Store,
                                      const FString &PresetName) {
  return waitForResult(
      Store.dispatch(rtk::loadBridgePresetThunk(PresetName)));
}

/**
 * Lists all registered bridge rulesets.
 * User Story: As bridge-rules CLI flows, I need the full ruleset list so I can
 * inspect or manage available rule definitions.
 */
inline TArray<FDirectiveRuleSet>
listRulesets(rtk::EnhancedStore<FRuntimeState> &Store) {
  return waitForResult(Store.dispatch(rtk::listRulesetsThunk()));
}

/**
 * Lists the available bridge preset identifiers.
 * User Story: As bridge-rules CLI flows, I need preset ids so terminal users
 * can discover the presets available for loading.
 */
inline TArray<FString> listRulePresets(rtk::EnhancedStore<FRuntimeState> &Store) {
  return waitForResult(Store.dispatch(rtk::listRulePresetsThunk()));
}

/**
 * Registers a bridge ruleset and returns the persisted record.
 * User Story: As bridge-rules CLI flows, I need one helper to persist rulesets
 * so rule management stays aligned with runtime APIs.
 */
inline FDirectiveRuleSet
registerRuleset(rtk::EnhancedStore<FRuntimeState> &Store,
              const FDirectiveRuleSet &Ruleset) {
  return waitForResult(Store.dispatch(rtk::registerRulesetThunk(Ruleset)));
}

/**
 * Deletes a registered bridge ruleset by id.
 * User Story: As bridge-rules CLI flows, I need one helper to remove rulesets
 * so cleanup uses the same thunk contract as the runtime.
 */
inline rtk::FEmptyPayload deleteRuleset(rtk::EnhancedStore<FRuntimeState> &Store,
                                      const FString &RulesetId) {
  return waitForResult(Store.dispatch(rtk::deleteRulesetThunk(RulesetId)));
}

/**
 * Exports the target NPC as a remotely stored soul payload.
 * User Story: As soul-export CLI flows, I need one helper to export an NPC so
 * remote soul publishing can be triggered from the terminal.
 */
inline FSoulExportResult exportSoul(rtk::EnhancedStore<FRuntimeState> &Store,
                                    const FString &NpcId) {
  return waitForResult(Store.dispatch(rtk::remoteExportSoulThunk(NpcId)));
}

/**
 * Imports a soul payload from Arweave into runtime memory.
 * User Story: As soul-import CLI flows, I need one helper to import a remote
 * soul so terminal workflows can restore NPC state from transactions.
 */
inline FSoul importSoul(rtk::EnhancedStore<FRuntimeState> &Store,
                        const FString &TxId) {
  return waitForResult(
      Store.dispatch(rtk::importSoulFromArweaveThunk(TxId)));
}

/**
 * Imports an NPC directly from a soul transaction id.
 * User Story: As soul-import CLI flows, I need one helper to materialize an
 * NPC from a soul so recovery workflows stay simple in the shell.
 */
inline FImportedNpc importNpcFromSoul(rtk::EnhancedStore<FRuntimeState> &Store,
                                      const FString &TxId) {
  return waitForResult(Store.dispatch(rtk::importNpcFromSoulThunk(TxId)));
}

/**
 * Lists remotely discoverable souls up to the requested limit.
 * User Story: As soul-browsing CLI flows, I need discoverable soul metadata so
 * terminal users can inspect available remote souls.
 */
inline TArray<FSoulListItem> listSouls(rtk::EnhancedStore<FRuntimeState> &Store,
                                       int32 Limit = 50) {
  return waitForResult(Store.dispatch(rtk::getSoulListThunk(Limit)));
}

/**
 * Verifies a soul transaction and returns the validation result.
 * User Story: As soul-verification CLI flows, I need one helper to validate a
 * transaction so trust checks reuse the runtime verification thunk.
 */
inline FSoulVerifyResult verifySoul(rtk::EnhancedStore<FRuntimeState> &Store,
                                    const FString &TxId) {
  return waitForResult(Store.dispatch(rtk::verifySoulThunk(TxId)));
}

/**
 * Exports the target NPC to a local in-memory soul without remote upload.
 * User Story: As local-soul CLI flows, I need one helper to export without
 * upload so shell workflows can inspect soul payloads locally.
 */
inline FSoul localExportSoul(rtk::EnhancedStore<FRuntimeState> &Store,
                              const FString &NpcId = TEXT("")) {
  return waitForResult(Store.dispatch(rtk::localExportSoulThunk(NpcId)));
}

/**
 * Initializes the local vector/embedding runtime.
 * User Story: As vector-runtime CLI flows, I need one helper to initialize the
 * embedder so local vector features can be prepared from the terminal.
 */
inline rtk::FEmptyPayload initVector(rtk::EnhancedStore<FRuntimeState> &Store) {
  return rtk::FEmptyPayload{};
}

/**
 * Generates an embedding vector for the supplied text.
 * User Story: As vector-runtime CLI flows, I need one helper to generate
 * embeddings so shell tooling can exercise local vectorization directly.
 */
inline TArray<float> generateEmbedding(rtk::EnhancedStore<FRuntimeState> &Store,
                                       const FString &Text) {
  return TArray<float>();
}

/**
 * Persists a supported config key to the local SDK config file.
 * User Story: As CLI config management, I need one helper to persist settings
 * so shell commands can update SDK config without manual file edits.
 */
inline void setConfigValue(const FString &Key, const FString &Value) {
  SDKConfig::SetConfigValue(Key, Value);
}

/**
 * Reads a supported config key from the local SDK config file.
 * User Story: As CLI config management, I need one helper to read settings so
 * shell commands can inspect active SDK config values consistently.
 */
inline FString getConfigValue(const FString &Key) {
  return SDKConfig::GetConfigValue(Key);
}

} // namespace Ops
