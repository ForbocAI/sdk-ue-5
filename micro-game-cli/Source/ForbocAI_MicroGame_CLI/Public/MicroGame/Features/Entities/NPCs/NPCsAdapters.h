#pragma once
/**
 * Micro-game adapters own normalized entity indexing.
 */

#include "CoreMinimal.h"
#include "Core/rtk.hpp"
#include "Core/fp.hpp"
#include "MicroGame/Features/Data/DataAdapters.h"
#include "MicroGame/Features/Entities/NPCs/NPCsTypes.h"

namespace MicroGame {

/** User Story: As a features entities npcs consumer, I need to invoke is move verdict action through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline bool IsMoveVerdictAction(const FString &ActionType) */
inline bool IsMoveVerdictAction(const FString &ActionType) {
  static const DataAdapters::FSettingsSource Source =
      DataAdapters::SettingsSource(TEXT("entities/npcs.json"));
  const TSharedRef<FJsonObject> Actions =
      DataAdapters::ReadObjectField(Source, TEXT("verdictActions"));
  return ActionType ==
         DataAdapters::ReadStringField(Actions, TEXT("move"));
}

/**
 * Returns the entity adapter used for micro-game NPC state.
 * User Story: As micro-game entity reducers, I need one shared adapter so NPC
 * CRUD actions update and query a consistent normalized state shape.
 * @fn inline rtk::EntityAdapter<FGameNPC> &GetNPCAdapter()
 */
inline rtk::EntityAdapter<FGameNPC> &GetNPCAdapter() {
  static rtk::EntityAdapter<FGameNPC> Adapter =
      rtk::createEntityAdapter<FGameNPC>(
          [](const FGameNPC &N) { return N.Id; });
  return Adapter;
}

/** User Story: As a features entities npcs consumer, I need to invoke patch npc through a stable signature so the features entities npcs workflow remains explicit and composable. @fn inline FGameNPC PatchNpc(const FGameNPC &Existing, const NPCsActions::FPatchNPCChanges &Patch) */
inline FGameNPC PatchNpc(const FGameNPC &Existing,
                         const NPCsActions::FPatchNPCChanges &Patch) {
  FGameNPC Updated = Existing;
  Updated.Name = Patch.bHasName ? Patch.Name : Updated.Name;
  Updated.Faction = Patch.bHasFaction ? Patch.Faction : Updated.Faction;
  Updated.Hp = Patch.bHasHp ? Patch.Hp : Updated.Hp;
  Updated.Suspicion =
      Patch.bHasSuspicion ? Patch.Suspicion : Updated.Suspicion;
  Updated.Inventory = Patch.bHasInventory ? Patch.Inventory : Updated.Inventory;
  Updated.KnownSecrets =
      Patch.bHasKnownSecrets ? Patch.KnownSecrets : Updated.KnownSecrets;
  Updated.Position = Patch.bHasPosition ? Patch.Position : Updated.Position;
  return Updated;
}

} // namespace MicroGame
