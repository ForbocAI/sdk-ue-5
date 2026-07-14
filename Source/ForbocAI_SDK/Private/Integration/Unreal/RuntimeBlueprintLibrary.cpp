#include "Integration/Unreal/RuntimeBlueprintLibrary.h"
#include "CLI/CliOperations.h"
#include "Store.h"

namespace {
/**
 * Returns the singleton store used by blueprint convenience wrappers.
 * User Story: As blueprint utility calls, I need one shared store so
 * convenience functions reflect the same runtime state across blueprint nodes.
 */
rtk::EnhancedStore<FRuntimeState> &GetBPStore() {
  static rtk::EnhancedStore<FRuntimeState> Store = store();
  return Store;
}
} // namespace

/**
 * Returns the remote API status string exposed to blueprints.
 * User Story: As blueprint status panels, I need the API health string so
 * designers can inspect connectivity without writing C++.
 */
FString UForbocAIBlueprintLibrary::checkApiStatus() {
  FApiStatusResponse Resp = Ops::checkApiStatus(GetBPStore());
  return Resp.Status;
}

/**
 * Creates an NPC and returns the generated NPC id.
 * User Story: As blueprint setup flows, I need a node that creates NPCs and
 * returns ids so designers can spawn runtime agents without C++.
 */
FString UForbocAIBlueprintLibrary::createNpc(const FString &Persona) {
  FNPCInternalState Npc = Ops::createNpc(GetBPStore(), Persona);
  return Npc.Id;
}

/**
 * Processes NPC input and returns the dialogue string only.
 * User Story: As blueprint interaction flows, I need dialogue-only processing
 * so designers can wire NPC text responses into UI quickly.
 */
FString UForbocAIBlueprintLibrary::processNpc(const FString &NpcId,
                                               const FString &Text) {
  FAgentResponse Resp = Ops::processNpc(GetBPStore(), NpcId, Text);
  return Resp.Dialogue;
}

/**
 * Processes a chat-style NPC input and returns the dialogue string.
 * User Story: As blueprint chat flows, I need a chat-friendly wrapper so
 * dialogue requests can be triggered with minimal blueprint setup.
 */
FString UForbocAIBlueprintLibrary::ChatNpc(const FString &NpcId,
                                            const FString &Message) {
  FAgentResponse Resp = Ops::processNpc(GetBPStore(), NpcId, Message);
  return Resp.Dialogue;
}

/**
 * Reports whether the shared blueprint store currently has an active NPC.
 * User Story: As blueprint control flow, I need to know whether an active NPC
 * exists so graphs can branch safely before issuing commands.
 */
bool UForbocAIBlueprintLibrary::HasActiveNpc() {
  func::Maybe<FNPCInternalState> Active = Ops::getActiveNpc(GetBPStore());
  return Active.hasValue;
}

/**
 * Stores a new memory observation for the given NPC.
 * User Story: As blueprint memory flows, I need a node that stores
 * observations so designers can persist events without custom C++ glue.
 */
void UForbocAIBlueprintLibrary::storeMemory(const FString &NpcId,
                                             const FString &Observation) {
  Ops::storeMemory(GetBPStore(), NpcId, Observation);
}

/**
 * Clears all stored memories for the given NPC.
 * User Story: As blueprint reset flows, I need a node that clears memories so
 * tests and scripted resets can start from a clean slate.
 */
void UForbocAIBlueprintLibrary::clearMemory(const FString &NpcId) {
  Ops::clearMemory(GetBPStore(), NpcId);
}

/**
 * Starts a ghost run and returns the remote session id.
 * User Story: As blueprint automation flows, I need a node that launches ghost
 * runs so integrators can trigger runtime tests from host tools.
 */
FString UForbocAIBlueprintLibrary::startGhost(const FString &TestSuite,
                                             int32 Duration) {
  FGhostRunResponse Resp = Ops::startGhost(GetBPStore(), TestSuite, Duration);
  return Resp.SessionId;
}

/**
 * Stops a ghost run and returns the stop status string.
 * User Story: As blueprint automation flows, I need a node that stops ghost
 * runs so long-running tests can be cancelled from host tools.
 */
FString UForbocAIBlueprintLibrary::stopGhost(const FString &SessionId) {
  FGhostStopResponse Resp = Ops::stopGhost(GetBPStore(), SessionId);
  return Resp.StopStatus;
}

/**
 * Exports an NPC soul and returns the resulting transaction id.
 * User Story: As blueprint soul-export flows, I need a node that exports souls
 * so designers can persist NPC snapshots without custom code.
 */
FString UForbocAIBlueprintLibrary::exportSoul(const FString &NpcId) {
  FSoulExportResult Result = Ops::exportSoul(GetBPStore(), NpcId);
  return Result.TxId;
}

/**
 * Imports a soul and returns the imported soul id.
 * User Story: As blueprint soul-import flows, I need a node that imports souls
 * so designers can restore NPC data from transactions directly.
 */
FString UForbocAIBlueprintLibrary::importSoul(const FString &TxId) {
  FSoul Soul = Ops::importSoul(GetBPStore(), TxId);
  return Soul.Id;
}

/**
 * Verifies a soul transaction and returns whether it is valid.
 * User Story: As blueprint verification flows, I need a node that validates
 * soul transactions so trust checks can be built without C++.
 */
bool UForbocAIBlueprintLibrary::verifySoul(const FString &TxId) {
  FSoulVerifyResult Result = Ops::verifySoul(GetBPStore(), TxId);
  return Result.bValid;
}

/**
 * Validates a bridge action represented as raw JSON.
 * User Story: As blueprint bridge-validation flows, I need a node that checks
 * raw actions so design tools can test bridge rules without native code.
 */
bool UForbocAIBlueprintLibrary::ValidateBridgeAction(const FString &ActionJson) {
  FAgentAction Action;
  Action.PayloadJson = ActionJson;
  FBridgeValidationContext Context;
  FValidationResult Result = Ops::validateBridgePayload(GetBPStore(), Action, Context);
  return Result.bValid;
}

/**
 * Persists a runtime config value from blueprint code.
 * User Story: As blueprint config tools, I need a node that persists settings
 * so designers can update runtime config without editing files manually.
 */
void UForbocAIBlueprintLibrary::setConfigValue(const FString &Key,
                                           const FString &Value) {
  Ops::setConfigValue(Key, Value);
}

/**
 * Reads a runtime config value from blueprint code.
 * User Story: As blueprint config tools, I need a node that reads settings so
 * designers can inspect runtime config without C++ helpers.
 */
FString UForbocAIBlueprintLibrary::getConfigValue(const FString &Key) {
  return Ops::getConfigValue(Key);
}
