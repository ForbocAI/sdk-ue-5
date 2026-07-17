#pragma once

#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Entities/Player/PlayerActions.h"
#include "TestGame/Features/Systems/Harness/Game/GameAdapters.h"
#include "TestGame/Features/Systems/Harness/Game/Command/CommandThunks.h"
#include "TestGame/Features/Systems/Harness/Game/Progress/ProgressThunks.h"
#include "TestGame/Features/Systems/Harness/Game/GameTypes.h"
#include "TestGame/Features/Systems/Memory/MemoryActions.h"
#include "TestGame/Features/Systems/Social/SocialActions.h"
#include "TestGame/Features/Systems/Soul/SoulActions.h"
#include "TestGame/Features/Systems/Stealth/StealthActions.h"

namespace TestGame::GameThunksDetail {

/**
 * User Story: As a scenario runner, I need to apply authored initial state through a stable signature so every CLI coverage scenario starts deterministically.
 * @fn inline void ApplyScenarioInitialState(const FScenarioStep &Step, FTestGameStore &Store)
 */
inline void ApplyScenarioInitialState(const FScenarioStep &Step,
                                      FTestGameStore &Store) {
  const FGameRuntimeData &Data = GameAdapters::GameRuntimeData();
  (Step.EventType == Data.scenarioEventTypes.stealth)
      ? [&]() {
          Store.dispatch(StealthActions::setDoorOpen(
              Data.initialState.stealth.doorOpen));
          Store.dispatch(StealthActions::bumpAlert(
              Data.initialState.stealth.alertDelta));
          Store.dispatch(
              NPCsActions::UpsertNPC(Data.initialState.stealth.npc));
          Store.dispatch(GameMemoryActions::storeMemory(
              Data.initialState.stealth.memory));
        }()
      : void();

  (Step.EventType == Data.scenarioEventTypes.social)
      ? [&]() {
          Store.dispatch(
              NPCsActions::UpsertNPC(Data.initialState.social.npc));
          Store.dispatch(SocialActions::setDialogue(
              Data.initialState.social.dialogue));
          Store.dispatch(SocialActions::setTradeOffer(
              Data.initialState.social.tradeOffer));

          NPCsActions::FPatchNPCPayload Patch;
          Patch.Id = Data.initialState.social.npc.Id;
          Patch.Patch.Suspicion = Data.initialState.social.suspicion;
          Patch.Patch.bHasSuspicion = true;
          Store.dispatch(NPCsActions::PatchNPC(Patch));
        }()
      : void();

  Step.EventType == Data.scenarioEventTypes.escape
      ? (Store.dispatch(PlayerActions::setHidden(
             Data.initialState.escape.hidden)),
         void())
      : void();

  (Step.EventType == Data.scenarioEventTypes.persistence)
      ? [&]() {
          FMarkSoulExportedPayload Export;
          Export.NpcId = Data.initialState.persistence.npcId;
          Export.TxId = Data.initialState.persistence.txId;
          Store.dispatch(GameSoulActions::markSoulExported(Export));
          Store.dispatch(GameSoulActions::markSoulImported(
              Data.initialState.persistence.txId));
          Store.dispatch(GameMemoryActions::clearMemoryForNpc(
              Data.initialState.persistence.npcId));
        }()
      : void();
}

/** User Story: As a contract runner, I need scenarios sequenced recursively through the same root store and CLI boundary. @fn inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index, FTestGameStore &Store, const FString &ApiUrl, const FGameProgressSink &Sink) */
inline void ProcessSteps(const TArray<FScenarioStep> &Steps, int32 Index,
                         FTestGameStore &Store,
                         const FString &ApiUrl,
                         const FGameProgressSink &Sink) {
  Index >= Steps.Num()
      ? void()
      : [&]() {
          FGameProgress Progress;
          Progress.Type =
              GameAdapters::GameRuntimeData().lifecycleEvents.stepStarted;
          Progress.Step = Steps[Index];
          Emit(Sink, MoveTemp(Progress));
          ApplyScenarioInitialState(Steps[Index], Store);
          ProcessCommands(
              Steps[Index],
              GameAdapters::GameRuntimeData().numbers.emptyCount, Store,
              ApiUrl, Sink);
          ProcessSteps(
              Steps,
              Index + GameAdapters::GameRuntimeData().numbers.nextIndex,
              Store, ApiUrl, Sink);
        }();
}

} // namespace TestGame::GameThunksDetail
