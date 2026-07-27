#include "Systems/Integration/Unreal/Subsystem/SubsystemThunks.h"
#include "Components/AuthoredValues/AuthoredValuesTypes.h"
#include "Entities/Bridge/BridgeSelectors.h"
#include "Entities/Memory/MemorySelectors.h"
#include "Entities/NPC/NPCSelectors.h"
#include "Systems/CLI/Config/ConfigThunks.h"
#include "Systems/Store/StoreAdapters.h"
#include "Systems/Protocol/ProtocolThunks.h"
#include "Systems/Soul/SoulThunks.h"

/**
 * Initializes the subsystem against the package-owned root store.
 * User Story: As game runtime startup, I need every SDK surface to share the
 * same state authority while completed operations project to Blueprint
 * delegates.
 * @fn void UForbocAISubsystem::Initialize(FSubsystemCollectionBase &Collection)
 */
void UForbocAISubsystem::Initialize(FSubsystemCollectionBase &Collection) {
  Super::Initialize(Collection);

  Store = &StoreAdapters::RootStore();
}
/**
 * Releases the runtime store during subsystem shutdown.
 * User Story: As game runtime shutdown, I need the subsystem to release store
 * resources so teardown does not leak runtime state.
 * @fn void UForbocAISubsystem::Deinitialize()
 */
void UForbocAISubsystem::Deinitialize() {
  Store = nullptr;
  Super::Deinitialize();
}

/**
 * Applies API credentials and URL overrides to the runtime config.
 * User Story: As subsystem setup flows, I need one init entry point for API
 * config so host code can point the runtime at the right backend.
 * @fn void UForbocAISubsystem::Init(FString ApiKey, FString ApiUrl)
 */
void UForbocAISubsystem::Init(FString ApiKey, FString ApiUrl) {
  Store == nullptr ? void()
                   : Ops::commitApiConfiguration(*Store, ApiUrl, ApiKey);
}

/**
 * Runs a protocol turn for an NPC and broadcasts dialogue or actions.
 * User Story: As host interaction flows, I need NPC turns processed from
 * the subsystem so dialogue, typing, and action events are broadcast to game code.
 * @fn void UForbocAISubsystem::ProcessNPC(FString NpcId, FString Input)
 */
void UForbocAISubsystem::ProcessNPC(FString NpcId, FString Input) {
  Store == nullptr
      ? void()
      : (OnTypingStart.Broadcast(),
         Store
             ->dispatch(rtk::processNPC(NpcId, Input, TEXT(FORBOCAI_SDK_AUTHORED_STRINGVF54CAD9838EB), TEXT(""),
                                        FAgentState(),
                                        rtk::LocalProtocolHandlerContext(
                                            NpcId)))
             .then([this](const FAgentResponse &Result) {
               !Result.Dialogue.IsEmpty()
                   ? (OnMessageReceived.Broadcast(Result.Dialogue),
                      OnTTSRequested.Broadcast(Result.Dialogue), void())
                   : void();
               !Result.Action.Type.IsEmpty()
                   ? (OnNPCActionReceived.Broadcast(Result.Action), void())
                   : void();
               OnTypingEnd.Broadcast();
             })
             .catch_([this](std::string Error) {
               static_cast<void>(Error);
               OnTypingEnd.Broadcast();
             })
             .execute());
}

/**
 * Exports an NPC soul and broadcasts the completed transaction id.
 * User Story: As host soul-export flows, I need subsystem-triggered export
 * events so game code can react when a soul has been published.
 * @fn void UForbocAISubsystem::exportSoul(FString AgentId)
 */
void UForbocAISubsystem::exportSoul(FString AgentId) {
  Store == nullptr
      ? void()
      : (void)Store->dispatch(rtk::exportSoulThunk()(AgentId))
            .then([this](const FSoulExportResult &Result) {
              OnSoulExportComplete.Broadcast(Result.TxId);
            })
            .execute();
}

/**
 * Returns the latest state snapshot for the requested NPC.
 * User Story: As host state queries, I need the latest NPC state from the
 * subsystem so UI and logic can inspect current agent data.
 * @fn FAgentState UForbocAISubsystem::GetNPCState(FString NpcId) const
 */
FAgentState UForbocAISubsystem::GetNPCState(FString NpcId) const {
  return Store == nullptr
             ? FAgentState()
             : func::or_else(
                   func::fmap(
                       NPCSelectors::selectNPCById(Store->getState().NPCs, NpcId),
                       [](const FNPCInternalState &Npc) { return Npc.State; }),
                   FAgentState());
}

/**
 * Returns the id of the active NPC, if any.
 * User Story: As host targeting flows, I need the active NPC id so other
 * systems can address the current actor consistently.
 * @fn FString UForbocAISubsystem::GetActiveNPCId() const
 */
FString UForbocAISubsystem::GetActiveNPCId() const {
  return Store == nullptr
             ? FString()
             : NPCSelectors::selectActiveNpcId(Store->getState().NPCs);
}

/**
 * Writes the active NPC into OutNPC when one is present.
 * User Story: As host state queries, I need the active NPC materialized so
 * consumers can read the full runtime record without manual store access.
 * @fn bool UForbocAISubsystem::GetActiveNPC(FNPCInternalState &OutNPC) const
 */
bool UForbocAISubsystem::GetActiveNPC(FNPCInternalState &OutNPC) const {
  return Store == nullptr
             ? false
             : func::match(
                   NPCSelectors::selectActiveNPC(Store->getState().NPCs),
                   [&OutNPC](const FNPCInternalState &Active) {
                     OutNPC = Active;
                     return true;
                   },
                   []() { return false; });
}

/**
 * Returns the last memory recall result emitted by the store.
 * User Story: As host memory UIs, I need the last recall batch so recent
 * memory results can be rendered without reissuing the query.
 * @fn TArray<FMemoryItem> UForbocAISubsystem::GetRecalledMemories() const
 */
TArray<FMemoryItem> UForbocAISubsystem::GetRecalledMemories() const {
  return Store == nullptr
             ? TArray<FMemoryItem>()
             : MemorySelectors::selectRecalledMemories(
                   Store->getState().Memory);
}

/**
 * Writes the most recent bridge validation result into OutResult.
 * User Story: As host validation feedback, I need the latest bridge result
 * so designers can inspect whether an action was valid.
 * @fn bool UForbocAISubsystem::GetBridgeValidationResult( FValidationResult &OutResult) const
 */
bool UForbocAISubsystem::GetBridgeValidationResult(
    FValidationResult &OutResult) const {
  return Store == nullptr
             ? false
             : func::match(
                   BridgeSelectors::selectBridgeValidationResult(
                       Store->getState().Bridge),
                   [&OutResult](const FValidationResult &Result) {
                     OutResult = Result;
                     return true;
                   },
                   []() { return false; });
}

/**
 * Writes the last imported soul into OutSoul when one exists.
 * User Story: As host soul-import flows, I need the latest imported soul
 * exposed so game tools can inspect the restored payload.
 * @fn bool UForbocAISubsystem::GetImportedSouledSoul(FSoul &OutSoul) const
 */
bool UForbocAISubsystem::GetImportedSouledSoul(FSoul &OutSoul) const {
  return Store == nullptr ? false
                          : [this, &OutSoul]() -> bool {
    const FRuntimeState State = Store->getState();
    return State.Soul.bHasImportedSoul
               ? (OutSoul = State.Soul.ImportedSoul, true)
               : false;
  }();
}
