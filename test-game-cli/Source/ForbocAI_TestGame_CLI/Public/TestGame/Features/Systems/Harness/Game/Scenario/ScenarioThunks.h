#pragma once

#include "TestGame/Features/Entities/NPCs/NPCsActions.h"
#include "TestGame/Features/Entities/Player/PlayerActions.h"
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
  (Step.EventType == EEventType::Stealth)
      ? [&]() {
          Store.dispatch(StealthActions::setDoorOpen(true));
          Store.dispatch(StealthActions::bumpAlert(25));

          FGameNPC Npc;
          Npc.Id = TEXT("doomguard");
          Npc.Name = TEXT("Doomguard Patrol");
          Npc.Faction = TEXT("Doomguards");
          Npc.Hp = 100;
          Npc.Suspicion = 40;
          Npc.Position = FPosition(5, 10);
          Store.dispatch(NPCsActions::UpsertNPC(Npc));

          FMemoryRecord Memory;
          Memory.Id = TEXT("mem-door-001");
          Memory.NpcId = TEXT("doomguard");
          Memory.Text = TEXT("Armory door found open at x:5, y:12");
          Memory.Importance = 0.9f;
          Store.dispatch(GameMemoryActions::storeMemory(Memory));
        }()
      : void();

  (Step.EventType == EEventType::Social)
      ? [&]() {
          FGameNPC Npc;
          Npc.Id = TEXT("miller");
          Npc.Name = TEXT("Miller");
          Npc.Faction = TEXT("Neutral");
          Npc.Hp = 100;
          Npc.Suspicion = 50;
          Npc.Inventory.Add(TEXT("medkit"));
          Npc.KnownSecrets.Add(TEXT("player_stole_rations"));
          Npc.Position = FPosition(5, 12);
          Store.dispatch(NPCsActions::UpsertNPC(Npc));
          Store.dispatch(SocialActions::setDialogue(
              TEXT("I know you took those rations...")));

          FTradeOffer Offer;
          Offer.NpcId = TEXT("miller");
          Offer.Item = TEXT("medkit");
          Offer.Price = 100;
          Store.dispatch(SocialActions::setTradeOffer(Offer));

          NPCsActions::FPatchNPCPayload Patch;
          Patch.Id = TEXT("miller");
          Patch.Patch.Suspicion = 75;
          Patch.Patch.bHasSuspicion = true;
          Store.dispatch(NPCsActions::PatchNPC(Patch));
        }()
      : void();

  Step.EventType == EEventType::Escape
      ? (Store.dispatch(PlayerActions::setHidden(false)), void())
      : void();

  (Step.EventType == EEventType::Persistence)
      ? [&]() {
          FMarkSoulExportedPayload Export;
          Export.NpcId = TEXT("doomguard");
          Export.TxId = TEXT("tx-runtime-001");
          Store.dispatch(GameSoulActions::markSoulExported(Export));
          Store.dispatch(
              GameSoulActions::markSoulImported(TEXT("tx-runtime-001")));
          Store.dispatch(
              GameMemoryActions::clearMemoryForNpc(TEXT("doomguard")));
        }()
      : void();
}

} // namespace TestGame::GameThunksDetail
